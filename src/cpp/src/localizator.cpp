#include "localizator.h"
#include "CGAL/Boolean_set_operations_2.h"
#include "CGAL/Polygon_with_holes_2.h"
#include "utils.hpp"

#define ARC_APPX_POINTS_NUM 100
#define CONCHOID_APPX_POINTS_NUM 100

void Localizator::init(const std::vector<Point> &points) {
	trapezoids.clear();
	sorted_by_max.clear();
	rtree.clear();

	std::vector<Trapezoid> trapezoids_v;
	trapezoider.calc_trapezoids(points, trapezoids_v);
	for (const auto &trapezoid : trapezoids_v)
		trapezoids[trapezoid.get_id()] = trapezoid;

	debug_println("Trapezoids openings:");
	for (auto &t : trapezoids) {
		auto &trapezoid = t.second;
		trapezoid.calc_min_max_openings();
		debug_println("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max
							<< "]");
	}

	for (const auto &trapezoid : trapezoids)
		sorted_by_max.push_back(trapezoid.first);
	sort(sorted_by_max.begin(), sorted_by_max.end(),
		 [this](const auto &t1, const auto &t2) { return trapezoids[t1].opening_max < trapezoids[t2].opening_max; });
	debug_println("sorted_by_max:");
	for (const auto &t_id : sorted_by_max) {
		const Trapezoid &trapezoid = trapezoids[t_id];
		debug_println("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max
							<< "]");
	}

	for (const auto &t : trapezoids) {
		const auto &trapezoid = t.second;
		TrapezoidRTreePoint min(trapezoid.opening_min.exact().convert_to<double>());
		TrapezoidRTreePoint max(trapezoid.opening_max.exact().convert_to<double>());
		rtree.insert(TrapezoidRTreeValue(TrapezoidRTreeSegment(min, max), t.first));
	}
}

static Point intersection(Line l1, Line l2) {
	auto res = CGAL::intersection(l1, l2);
	assert(!res->empty());
	return boost::get<Point>(res.get());
}

// TODO remove
static bool is_degenerate(const Segment &s) {
	Kernel::Kernel kernel;
	Kernel::Comparison_result res = kernel.compare_xy_2_object()(s.source(), s.target());
	return res == CGAL::EQUAL;
}

static void calc_trapezoid_result_m1(const Trapezoid &trapezoid, const Kernel::FT &d, std::vector<Polygon> &res) {
	// oriante angles relative to the top edge
	Direction angle_begin = -trapezoid.angle_begin, angle_end = -trapezoid.angle_end;
	assert(calc_half_plane_side(angle_begin, angle_end) == HalfPlaneSide::Left);
	auto s = trapezoid.top_edge->source()->point(), t = trapezoid.top_edge->target()->point();
	Direction top_edge_direction(t.hx() - s.hx(), t.hy() - s.hy());
	assert(is_free(trapezoid.top_edge->face()));
	Direction mid_angle = top_edge_direction.perpendicular(CGAL::LEFT_TURN);

	const Polygon trapezoid_bounds = trapezoid.get_bounds_2d();

	bool begin_before_mid = calc_half_plane_side(mid_angle, angle_begin) == HalfPlaneSide::Right;
	bool end_after_mid = calc_half_plane_side(mid_angle, angle_end) == HalfPlaneSide::Left;
	Direction angle_intervals[][2] = {
		{begin_before_mid ? angle_begin : mid_angle, end_after_mid ? mid_angle : angle_end},
		{begin_before_mid ? mid_angle : angle_begin, end_after_mid ? angle_end : mid_angle}};

	debug_println("\t\t top edge " << trapezoid.top_edge->curve());
	debug_println("\t\t bottom edge " << trapezoid.bottom_edge->curve());
	debug_println("\t\t mid_angle " << mid_angle << " angle_begin " << angle_begin << " angle_end " << angle_end);
	debug_println("\t\t begin_before_mid " << begin_before_mid << " end_after_mid " << end_after_mid);
	debug_println("\t\t angle_intervals [" << angle_intervals[0][0] << ", " << angle_intervals[0][1] << "] ["
										   << angle_intervals[1][0] << ", " << angle_intervals[1][1] << "]");

	for (auto &angle_interval : angle_intervals) {
		auto a_begin = angle_interval[0], a_end = angle_interval[1];
		if (a_begin == a_end)
			continue;
		auto v_begin = normalize(a_begin.vector()), v_end = normalize(a_end.vector());
		std::vector<Point> left_points, right_points;
		debug_println("\t\t v_begin(" << v_begin << ") v_end(" << v_end << ")");

		const auto LEFT = 0, RIGHT = 1;
		for (auto side : {LEFT, RIGHT}) {
			auto vertex = (side == LEFT ? trapezoid.left_vertex : trapezoid.right_vertex)->point();
			auto &points = side == LEFT ? left_points : right_points;
			auto top_edge_line = trapezoid.top_edge->curve().line();

			if (top_edge_line.has_on(vertex)) {
				debug_println("\t\t" << (side == LEFT ? "left" : "right") << " is arc");
				Point begin = vertex + v_begin * d;
				Point end = vertex + v_end * d;
				double angle_between = std::acos((v_begin * v_end).exact().convert_to<double>());
				assert(angle_between != 0);

				points.push_back(begin);
				for (unsigned int i = 1; i < ARC_APPX_POINTS_NUM; i++) {
					double a = i * angle_between / ARC_APPX_POINTS_NUM;
					Direction dir =
						a_begin.transform(Kernel::Aff_transformation_2(CGAL::Rotation(), std::sin(a), std::cos(a)));
					points.push_back(Point(vertex + normalize(dir.vector()) * d));
				}
				points.push_back(end);
				debug_println("\t\t\t C(" << vertex << ") B(" << begin << ") E(" << end << ")");

			} else {
				debug_println("\t\t" << (side == LEFT ? "left" : "right") << " is conchoid");
				Point begin = intersection(top_edge_line, Line(vertex, a_begin)) + v_begin * d;
				Point end = intersection(top_edge_line, Line(vertex, a_end)) + v_end * d;
				double angle_between = std::acos((v_begin * v_end).exact().convert_to<double>());

				points.push_back(begin);
				for (unsigned int i = 1; i < CONCHOID_APPX_POINTS_NUM; i++) {
					double a = i * angle_between / CONCHOID_APPX_POINTS_NUM;
					Direction dir =
						a_begin.transform(Kernel::Aff_transformation_2(CGAL::Rotation(), std::sin(a), std::cos(a)));
					points.push_back(intersection(top_edge_line, Line(vertex, dir)) + normalize(dir.vector()) * d);
				}
				points.push_back(end);

				// debug_println("\t\t\t C(" << vertex << ") a= " << a << " b= " << d << " m= " << m << " pm= " <<
				// pm
				// 						<< " B(" << begin << ") E(" << end << ")");
			}
		}

		debug_print("left_points:");
		for (auto &p : left_points)
			debug_print(" (" << p << ")");
		debug_println("");
		debug_print("right_points:");
		for (auto &p : right_points)
			debug_print(" (" << p << ")");
		debug_println("");

		Polygon res_unbounded;
		res_unbounded.insert(res_unbounded.vertices_end(), left_points.begin(), left_points.end());
		res_unbounded.insert(res_unbounded.vertices_end(), right_points.rbegin(), right_points.rend());

		debug_println("trapezoid_bounds: " << trapezoid_bounds);
		debug_println("res_unbounded: " << res_unbounded);

		std::vector<CGAL::Polygon_with_holes_2<Kernel>> res_bounded;
		CGAL::intersection(trapezoid_bounds, res_unbounded, std::back_inserter(res_bounded));
		for (const auto &res_cell : res_bounded) {
			assert(res_cell.number_of_holes() == 0);
			debug_println("res_bounded: " << res_cell.outer_boundary());
			res.push_back(res_cell.outer_boundary());
		}
	}
}

void Localizator::query(const Kernel::FT &d, std::vector<Polygon> &res) {
	auto it =
		std::lower_bound(sorted_by_max.begin(), sorted_by_max.end(), d, [this](const auto &trapezoid, const auto &d) {
			return trapezoids[trapezoid].opening_max < d;
		});

	debug_println("Single measurement query (d = " << d << "):");
	for (; it != sorted_by_max.end(); ++it) {
		// Union result for each trapezoid
		const auto &trapezoid = trapezoids[*it];
		debug_println("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max
							<< "]");
		calc_trapezoid_result_m1(trapezoid, d, res);
	}
}

void Localizator::query(const Kernel::FT &d1, const Kernel::FT &d2, Arrangement &res) {
	const Kernel::FT d = d1 + d2;
	TrapezoidRTreePoint a(d), b(d);
	TrapezoidRTreeSegment s(a, b);
	std::vector<TrapezoidRTreeValue> res_vals;
	rtree.query(boost::geometry::index::intersects(s), std::back_inserter(res_vals));

	debug_println("Two measurements query (d1 = " << d1 << ", d2 = " << d2 << "):");
	for (const TrapezoidRTreeValue &rtree_val : res_vals) {
		// Union result for each trapezoid
		const auto &trapezoid = trapezoids[rtree_val.second];
		debug_println("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max
							<< "]");
		// TODO
	}
}
