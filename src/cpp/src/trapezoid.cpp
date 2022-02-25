#include "trapezoid.h"
#include "CGAL/Boolean_set_operations_2.h"
#include "CGAL/Boolean_set_operations_2/Gps_polygon_validation.h"
#include "CGAL/Polygon_with_holes_2.h"
#include "utils.hpp"

const Direction Trapezoid::ANGLE_NONE(0, 0);

Trapezoid::Trapezoid(Trapezoid::ID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex)
	: id(id), top_edge(top_edge), bottom_edge(bottom_edge), left_vertex(left_vertex), right_vertex(right_vertex),
	  angle_begin(Trapezoid::ANGLE_NONE), angle_end(Trapezoid::ANGLE_NONE) {}

Trapezoid::ID Trapezoid::get_id() const { return id; }

template <typename _Direction>
static _Direction rotate(const _Direction& d, double r) {
	return d.transform(Kernel::Aff_transformation_2(CGAL::Rotation(), std::sin(r), std::cos(r)));
}

static void calc_edge_left_right_vertices(const Halfedge& edge, const Direction& dir,  Point& left, Point& right) {
	Point p1 = edge->source()->point(), p2 = edge->target()->point();
	Point mid_top((p1.hx() + p2.hx()) / 2, (p1.hy() + p2.hy()) / 2);
	if (calc_half_plane_side(dir, Direction(p1.hx() - mid_top.hx(), p1.hy() - mid_top.hy())) == HalfPlaneSide::Left) {
		left = p1;
		right = p2;
	} else {
		left = p2;
		right = p1;
	}
}

Polygon Trapezoid::get_bounds_2d() const {
	auto v_begin = normalize(angle_begin.vector()), v_end = normalize(angle_end.vector());
	double angle_between = std::acos((v_begin * v_end).exact().convert_to<double>());
	assert(angle_between != 0);
	double a_mid = angle_between / 2;
	auto v_mid = rotate(v_begin, a_mid).direction();

	Point t1 = top_edge->source()->point(), t2 = top_edge->target()->point();
	Point top_left, top_right;
	calc_edge_left_right_vertices(top_edge, v_mid, top_left, top_right);

	Point b1 = bottom_edge->source()->point(), b2 = bottom_edge->target()->point();
	Point bottom_left, bottom_right;
	calc_edge_left_right_vertices(bottom_edge, v_mid, bottom_left, bottom_right);

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

static Point intersection(Line l1, Line l2) {
	auto res = CGAL::intersection(l1, l2);
	assert(!res->empty());
	return boost::get<Point>(res.get());
}

#define ARC_APPX_POINTS_NUM 360
#define CONCHOID_APPX_POINTS_NUM 360

void Trapezoid::calc_result_m1(const Kernel::FT &d, std::vector<Polygon> &res) const {
	// oriante angles relative to the top edge
	Direction a_begin = -angle_begin, a_end = -angle_end;
	assert(calc_half_plane_side(a_begin, a_end) == HalfPlaneSide::Left);
	auto s = top_edge->source()->point(), t = top_edge->target()->point();
	Direction top_edge_direction(t.hx() - s.hx(), t.hy() - s.hy());
	assert(is_free(top_edge->face()));
	Direction mid_angle = top_edge_direction.perpendicular(CGAL::LEFT_TURN);

	const Polygon trapezoid_bounds = get_bounds_2d();
	debugln("\t\t trapezoid_bounds (" << trapezoid_bounds << ")");

	bool begin_before_mid = calc_half_plane_side(mid_angle, a_begin) == HalfPlaneSide::Right;
	bool end_after_mid = calc_half_plane_side(mid_angle, a_end) == HalfPlaneSide::Left;
	Direction angle_intervals[][2] = {{begin_before_mid ? a_begin : mid_angle, end_after_mid ? mid_angle : a_end},
									  {begin_before_mid ? mid_angle : a_begin, end_after_mid ? a_end : mid_angle}};

	debugln("\t\t top edge (" << top_edge->curve() << ") bottom edge (" << bottom_edge->curve() << ")");
	debugln("\t\t mid_angle " << mid_angle << " angle_begin " << a_begin << " angle_end " << a_end);
	debugln("\t\t begin_before_mid " << begin_before_mid << " end_after_mid " << end_after_mid);
	debugln("\t\t angle_intervals [" << angle_intervals[0][0] << ", " << angle_intervals[0][1] << "] ["
									 << angle_intervals[1][0] << ", " << angle_intervals[1][1] << "]");

	for (unsigned int i = 0; i < 2; i++) {
		auto &angle_interval = angle_intervals[i];
		bool before_mid = i == 0;
		auto i_begin = angle_interval[0], i_end = angle_interval[1];
		if (i_begin == i_end)
			continue;
		auto top_edge_line = top_edge->curve().line();
		auto v_begin = normalize(i_begin.vector()), v_end = normalize(i_end.vector());
		double angle_between = std::acos((v_begin * v_end).exact().convert_to<double>());
		assert(angle_between != 0);
		debugln("\t\t v_begin(" << v_begin << ") v_end(" << v_end << ")");

		std::vector<Point> left_points, right_points;
		const auto LEFT = 0, RIGHT = 1;
		for (auto side : {LEFT, RIGHT}) {
			auto vertex = (side == LEFT ? left_vertex : right_vertex)->point();
			auto &points = side == LEFT ? left_points : right_points;

			if (top_edge_line.has_on(vertex)) {
				debugln("\t\t" << (side == LEFT ? "left" : "right") << " is arc");
				Point begin = vertex + v_begin * d;
				Point end = vertex + v_end * d;
				unsigned int appx_num = (unsigned int)((std::abs(angle_between) / (M_PI * 2)) * ARC_APPX_POINTS_NUM);

				points.push_back(begin);
				debugln(points.back());
				for (unsigned int i = 1; i < appx_num; i++) {
					double a = i * angle_between / appx_num;
					Direction dir = rotate(i_begin, a);
					points.push_back(Point(vertex + normalize(dir.vector()) * d));
					debugln(points.back());
				}
				points.push_back(end);
				debugln(points.back());
				debugln("\t\t\t C(" << vertex << ") B(" << begin << ") E(" << end << ")");

			} else {
				debugln("\t\t" << (side == LEFT ? "left" : "right") << " is conchoid");
				Point begin = intersection(top_edge_line, Line(vertex, i_begin)) + v_begin * d;
				Point end = intersection(top_edge_line, Line(vertex, i_end)) + v_end * d;
				unsigned int appx_num =
					(unsigned int)((std::abs(angle_between) / (M_PI * 2)) * CONCHOID_APPX_POINTS_NUM);

				points.push_back(begin);
				debugln(points.back());
				for (unsigned int i = 1; i < appx_num; i++) {
					double a = i * angle_between / appx_num;
					Direction dir = rotate(i_begin, a);
					points.push_back(intersection(top_edge_line, Line(vertex, dir)) + normalize(dir.vector()) * d);
					debugln(points.back());
				}
				points.push_back(end);
				debugln(points.back());

				// debugln("\t\t\t C(" << vertex << ") a= " << a << " b= " << d << " m= " << m << " pm= " <<
				// pm
				// 						<< " B(" << begin << ") E(" << end << ")");
			}
		}

		// debug("left_points:");
		// for (auto &p : left_points)
		// 	debug(" (" << p << ")");
		// debugln("");
		// debug("right_points:");
		// for (auto &p : right_points)
		// 	debug(" (" << p << ")");
		// debugln("");

		Polygon res_unbounded;
		if (before_mid) {
			auto left_begin = left_points.begin();
			if (*left_begin == right_points.front())
				++left_begin;
			auto right_begin = right_points.rbegin();
			if (*right_begin == left_points.back())
				right_begin++;
			res_unbounded.insert(res_unbounded.vertices_end(), left_begin, left_points.end());
			res_unbounded.insert(res_unbounded.vertices_end(), right_begin, right_points.rend());
		} else {
			auto left_begin = left_points.rbegin();
			if (*left_begin == right_points.back())
				++left_begin;
			auto right_begin = right_points.begin();
			if (*right_begin == left_points.front())
				right_begin++;
			res_unbounded.insert(res_unbounded.vertices_end(), left_begin, left_points.rend());
			res_unbounded.insert(res_unbounded.vertices_end(), right_begin, right_points.end());
		}

		// debugln("trapezoid_bounds: " << trapezoid_bounds);
		// debugln("res_unbounded: " << res_unbounded);

		std::vector<CGAL::Polygon_with_holes_2<Kernel>> res_bounded;
		CGAL::intersection(trapezoid_bounds, res_unbounded, std::back_inserter(res_bounded));
		for (const auto &res_cell : res_bounded) {
			assert(res_cell.number_of_holes() == 0);
			// debugln("res_bounded: " << res_cell.outer_boundary());
			res.push_back(res_cell.outer_boundary());
		}
	}
}

void Trapezoid::calc_min_max_openings() {
	std::vector<Polygon> res;
	auto has_bigger_opening = [this, &res](const Kernel::FT &d) {
		calc_result_m1(d, res);
		bool has_bigger = res.size() > 0;
		res.clear();
		return has_bigger;
	};

	Kernel::FT lower, upper;
	if (has_bigger_opening(1)) {
		lower = 1;
		for (;;) {
			upper = lower * 2;
			if (!has_bigger_opening(upper))
				break;
			lower = upper;
		}
	} else {
		upper = 1;
		for (;;) {
			lower = upper / 2;
			if (has_bigger_opening(lower))
				break;
			upper = lower;
		}
	}
	assert(has_bigger_opening(lower));
	assert(!has_bigger_opening(upper));

	const Kernel::FT binary_search_precision = 0.01;
	while (upper - lower > binary_search_precision) {
		Kernel::FT mid = (upper + lower) / 2;
		if (has_bigger_opening(mid))
			lower = mid;
		else
			upper = mid;
	}
	opening_max = upper;

	opening_min = 0; // TODO
}
