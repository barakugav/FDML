#include "trapezoid.h"
#include "CGAL/Boolean_set_operations_2.h"
#include "CGAL/Boolean_set_operations_2/Gps_polygon_validation.h"
#include "CGAL/Polygon_with_holes_2.h"
#include "utils.hpp"
#include <CGAL/enum.h>

const Direction Trapezoid::ANGLE_NONE(0, 0);

Trapezoid::Trapezoid(Trapezoid::ID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex)
	: id(id), top_edge(top_edge), bottom_edge(bottom_edge), left_vertex(left_vertex), right_vertex(right_vertex),
	  angle_begin(Trapezoid::ANGLE_NONE), angle_end(Trapezoid::ANGLE_NONE) {}

Trapezoid::ID Trapezoid::get_id() const { return id; }

/* rotate a direction by a given angle (radians) */
template <typename _Direction> static _Direction rotate(const _Direction &d, double r) {
	return d.transform(Kernel::Aff_transformation_2(CGAL::Rotation(), std::sin(r), std::cos(r)));
}

static Direction get_mid_angle(Direction angle_begin, Direction angle_end) {
	auto v_begin = normalize(angle_begin.vector()), v_end = normalize(angle_end.vector());
	double angle_between = std::acos(CGAL::to_double(v_begin * v_end));
	assert(angle_between != 0);
	double a_mid = angle_between / 2;
	return rotate(v_begin, a_mid).direction();
}

/* Calculate which of an edge endpoint is "left" and "right" relative to some direction */
static void calc_edge_left_right_vertices(const Halfedge &edge, const Direction &dir, Point &left, Point &right) {
	Point p1 = edge->source()->point(), p2 = edge->target()->point();
	Point mid_top((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2);
	if (calc_half_plane_side(dir, Direction(p1.x() - mid_top.x(), p1.y() - mid_top.y())) == HalfPlaneSide::Left) {
		left = p1;
		right = p2;
	} else {
		left = p2;
		right = p1;
	}
}

Polygon Trapezoid::get_bounds_2d() const {
	auto v_mid = get_mid_angle(angle_begin, angle_end);

	/* Calculate left and right vertices of the top edge relative to the trapezoid's direction */
	Point t1 = top_edge->source()->point(), t2 = top_edge->target()->point();
	Point top_left, top_right;
	calc_edge_left_right_vertices(top_edge, v_mid, top_left, top_right);

	/* Calculate left and right vertices of the bottom edge relative to the trapezoid's direction */
	Point bottom_left, bottom_right;
	calc_edge_left_right_vertices(bottom_edge, v_mid, bottom_left, bottom_right);

	/* construct the bounds polygon. Might used only 3 vertices if top and bottom edge share a vertex */
	std::vector<Point> points;
	if (top_left == bottom_left)
		points = {top_right, top_left, bottom_right};
	else if (top_right == bottom_right)
		points = {top_right, top_left, bottom_left};
	else
		points = {top_right, top_left, bottom_left, bottom_right};
	Polygon bounds(points.begin(), points.end());

	CGAL::Gps_default_traits<Polygon>::Traits traits;
	assert(CGAL::has_valid_orientation_polygon(bounds, traits));
	return bounds;
}

/* calculate the intersection point of two lines */
static Point intersection(const Line &l1, const Line &l2) {
	auto res = CGAL::intersection(l1, l2);
	assert(!res->empty());
	return boost::get<Point>(res.get());
}

/* We use a polygon approximation to repsent the complex curves of the result. These defines determine the percision of
 * the approximation. Both approximation of the arc and conchoid is done by discretizing the angle. The defines below
 * define how many discrete step will be used in a 2*PI angle, relative approximation will be used for other angles. */
#define ARC_APPX_POINTS_NUM 360
#define CONCHOID_APPX_POINTS_NUM 360

void Trapezoid::calc_result_m1(const Kernel::FT &d, std::vector<Polygon> &res) const {
	debugln("[Trapezoid] calculating single measurement result...");
	/* oriante angles relative to the top edge */
	Direction a_begin = -angle_begin, a_end = -angle_end;
	assert(calc_half_plane_side(a_begin, a_end) == HalfPlaneSide::Left);
	auto s = top_edge->source()->point(), t = top_edge->target()->point();
	Direction top_edge_direction(t.x() - s.x(), t.y() - s.y());
	assert(is_free(top_edge->face()));

	/* Calculate the trapezoid bounds. Will be used to intersect each result entry. */
	const Polygon trapezoid_bounds = get_bounds_2d();
	debugln("\ttrapezoid bounds (" << trapezoid_bounds << ')');

	/* calculate the mid angle, which is perpendicular to the top edge, and use it to split the trapezoid angle
	 * interval into 2 to ensure simple polygon output for each result entry. */
	Direction mid_angle = top_edge_direction.perpendicular(CGAL::LEFT_TURN);
	bool begin_before_mid = calc_half_plane_side(mid_angle, a_begin) == HalfPlaneSide::Right;
	bool end_after_mid = calc_half_plane_side(mid_angle, a_end) == HalfPlaneSide::Left;
	Direction angle_intervals[2][2] = {{begin_before_mid ? a_begin : mid_angle, end_after_mid ? mid_angle : a_end},
									   {begin_before_mid ? mid_angle : a_begin, end_after_mid ? a_end : mid_angle}};

	debugln("\ttop edge (" << top_edge->curve() << ") bottom edge (" << bottom_edge->curve() << ')');

	/* calculate result for each angle interval */
	for (unsigned int internal_idx = 0; internal_idx < 2; internal_idx++) {
		auto &angle_interval = angle_intervals[internal_idx];
		bool before_mid = internal_idx == 0;
		auto i_begin = angle_interval[0], i_end = angle_interval[1];
		if (i_begin == i_end)
			continue; /* ignore if the angle interval is empty */

		debugln("\tangle interval [" << i_begin << ", " << i_end << ']');
		auto top_edge_line = top_edge->curve().line();
		auto v_begin = normalize(i_begin.vector()), v_end = normalize(i_end.vector());
		double angle_between = std::acos(CGAL::to_double(v_begin * v_end));
		assert(angle_between != 0);
		debugln("\tv_begin(" << v_begin << ") v_end(" << v_end << ')');

		/* calculate the points representing the curves in both sides of the trapezoid */
		std::vector<Point> left_points, right_points;
		const auto LEFT = 0, RIGHT = 1;
		for (auto side : {LEFT, RIGHT}) {
			auto vertex = (side == LEFT ? left_vertex : right_vertex)->point();
			auto &points = side == LEFT ? left_points : right_points;

			if (top_edge_line.has_on(vertex)) {
				/* Arc curve */
				debugln("\tcurve " << (side == LEFT ? "left" : "right") << " is arc");
				Point begin = vertex + v_begin * d;
				Point end = vertex + v_end * d;
				unsigned int appx_num = (unsigned int)((std::abs(angle_between) / (M_PI * 2)) * ARC_APPX_POINTS_NUM);

				/* approximate all points of the curve by used angle steps */
				points.push_back(begin);
				for (unsigned int i = 1; i < appx_num; i++) {
					double a = i * angle_between / appx_num;
					Direction dir = rotate(i_begin, a);
					points.push_back(Point(vertex + normalize(dir.vector()) * d));
				}
				points.push_back(end);
				debugln("\t\tO(" << vertex << ") r(" << d << ") B(" << begin << ") E(" << end << ')');

			} else {
				/* Conchoid curve */
				debugln("\tcurve " << (side == LEFT ? "left" : "right") << " is conchoid");
				Point begin = intersection(top_edge_line, Line(vertex, i_begin)) + v_begin * d;
				Point end = intersection(top_edge_line, Line(vertex, i_end)) + v_end * d;
				unsigned int appx_num =
					(unsigned int)((std::abs(angle_between) / (M_PI * 2)) * CONCHOID_APPX_POINTS_NUM);

				/* approximate all points of the curve by used angle steps */
				points.push_back(begin);
				for (unsigned int i = 1; i < appx_num; i++) {
					double a = i * angle_between / appx_num;
					Direction dir = rotate(i_begin, a);
					points.push_back(intersection(top_edge_line, Line(vertex, dir)) + normalize(dir.vector()) * d);
				}
				points.push_back(end);

				debugln("\t\tO(" << vertex << ") r(" << d << ") B(" << begin << ") E(" << end << ')');
			}

			debug("\t\tcurve points:");
			for (auto &p : left_points)
				debug(" (" << p << ')');
			debugln("");
		}

		/* construct a simple polygon from the two approximated curves */
		Polygon res_unbounded;
		if (before_mid) {
			/* avoid points duplication if the two curves share start/end vertices */
			auto left_begin = left_points.begin();
			if (*left_begin == right_points.front())
				++left_begin;
			auto right_begin = right_points.rbegin();
			if (*right_begin == left_points.back())
				right_begin++;
			res_unbounded.insert(res_unbounded.vertices_end(), left_begin, left_points.end());
			res_unbounded.insert(res_unbounded.vertices_end(), right_begin, right_points.rend());
		} else {
			/* avoid points duplication if the two curves share start/end vertices */
			auto left_begin = left_points.rbegin();
			if (*left_begin == right_points.back())
				++left_begin;
			auto right_begin = right_points.begin();
			if (*right_begin == left_points.front())
				right_begin++;
			res_unbounded.insert(res_unbounded.vertices_end(), left_begin, left_points.rend());
			res_unbounded.insert(res_unbounded.vertices_end(), right_begin, right_points.end());
		}

		/* intersect the result polygon with the trapezoids bound and add the result to the output */
		std::vector<CGAL::Polygon_with_holes_2<Kernel>> res_bounded;
		CGAL::intersection(trapezoid_bounds, res_unbounded, std::back_inserter(res_bounded));
		debugln("\ttrapezoid result:");
		for (const auto &res_cell : res_bounded) {
			assert(res_cell.number_of_holes() == 0);
			res.push_back(res_cell.outer_boundary());
			debugln("\t\t" << res_cell.outer_boundary());
		}
	}
}

void Trapezoid::calc_min_max_openings(Kernel::FT &opening_min, Kernel::FT &opening_max) const {
	/* for any fixed angle, the opening function is a affine function, and therefore monotonically increasing or
	 * decreasing as a function x. Therefore, to calculate the minimum or the maximum of the opening function we only
	 * need to consider the values at the end of the x valid interval of the functions, these are the x values defined
	 * by the left and right limiting vertices. */

	auto calc_arc_opening = [this](const Point &vertex, const Direction &angle) {
		Line bottom_line = bottom_edge->curve().line();
		if (bottom_line.has_on(vertex))
			return (Kernel::FT)0;
		Line opening_line = Line(vertex, angle);
		Point inter = intersection(bottom_line, opening_line);
		Kernel::FT xd = vertex.x() - inter.x(), xy = vertex.y() - inter.y();
		return CGAL::approximate_sqrt(xd * xd + xy * xy);
	};
	auto calc_conchoid_opening = [this](const Point &vertex, const Direction &angle) {
		Line top_line = top_edge->curve().line();
		Line bottom_line = bottom_edge->curve().line();
		Line opening_line = Line(vertex, angle);
		Point inter1 = top_line.has_on(vertex) ? vertex : intersection(top_line, opening_line);
		Point inter2 = bottom_line.has_on(vertex) ? vertex : intersection(bottom_line, opening_line);
		Kernel::FT xd = inter1.x() - inter2.x(), xy = inter1.y() - inter2.y();
		return CGAL::approximate_sqrt(xd * xd + xy * xy);
	};
	auto angle_between = [](double low, double high) {
		double r = high - low;
		return low < high ? r : r + 2 * M_PI;
	};
	auto dir_to_angle = [](const Direction &dir) {
		return std::atan2(CGAL::to_double(dir.dy()), CGAL::to_double(dir.dx()));
	};
	for (unsigned int side = 0; side < 2; side++) {
		Point limit_vertex = (side == 0 ? left_vertex : right_vertex)->point();
		Kernel::FT max, min;

		if (top_edge->curve().line().has_on(limit_vertex)) {
			/* Arc */
			/* for an arc curve of a limiting vertex, the minimum is always achieved at the angle perpendicular to the
			 * bottom edge, but it may not be included in the trapezoid angle interval, and we consider the interval
			 * limits as well. The maximum will always be one of the angle interval limits. */
			Kernel::FT m1 = calc_arc_opening(limit_vertex, -angle_begin);
			Kernel::FT m2 = calc_arc_opening(limit_vertex, -angle_end);
			max = CGAL::max(m1, m2);

			Point bottom_left, bottom_right;
			calc_edge_left_right_vertices(bottom_edge, get_mid_angle(angle_begin, angle_end), bottom_left,
										  bottom_right);
			Direction perp = Direction(bottom_right.x() - bottom_left.x(), bottom_right.y() - bottom_left.y())
								 .perpendicular(CGAL::LEFT_TURN);
			if (perp.counterclockwise_in_between(angle_begin, angle_end))
				min = calc_arc_opening(limit_vertex, -perp);
			else
				min = CGAL::min(m1, m2);

		} else {
			/* Conchoid */
			/* for a conchoid curve of a limiting vertex, i failed to calculate analytically the minimum point, and
			 * therefore forced to search numerically on the convex function. The maximum will always be one of the
			 * angle interval limits. */
			Kernel::FT m1 = calc_conchoid_opening(limit_vertex, -angle_begin);
			Kernel::FT m2 = calc_conchoid_opening(limit_vertex, -angle_end);
			min = CGAL::min(m1, m2);
			max = CGAL::max(m1, m2);

			double a_low = dir_to_angle(angle_begin), a_high = dir_to_angle(angle_end);
			const double PRECISION = 0.01;
			while (std::abs(a_high - a_low) > PRECISION) {
				double a = angle_between(a_low, a_high);
				double mid1 = a * 1 / 3, mid2 = a * 2 / 3;
				Kernel::FT mid1_min_opening = calc_conchoid_opening(limit_vertex, rotate(angle_begin, mid1));
				Kernel::FT mid2_min_opening = calc_conchoid_opening(limit_vertex, rotate(angle_begin, mid2));
				if (mid1_min_opening <= mid2_min_opening)
					a_high = a_low + mid2;
				else
					a_low = a_low + mid1;
			}
			double a = angle_between(a_low, a_high) / 2;
			min = CGAL::min(min, calc_conchoid_opening(limit_vertex, rotate(angle_begin, a)));
		}

		if (side == 0) {
			opening_min = min;
			opening_max = max;
		} else {
			opening_min = CGAL::min(opening_min, min);
			opening_max = CGAL::max(opening_max, max);
		}
	}
}
