#include "localizator.h"
#include "utils.hpp"

static Kernel::FT sqrt(Kernel::FT x) {
	// TODO this function losses precision, maybe can use CGAL::Algebraic_structure_traits<Kernel::FT>::Sqrt
	return std::sqrt(x.exact().convert_to<double>());
}

template <typename _Vector> static _Vector normalize(_Vector v) {
	Kernel::FT norm = sqrt(v.squared_length());
	return norm > 1e-30 ? v / norm : v;
}

#define ARC_APPX_POINTS_NUM 100
#define CONCHOID_APPX_POINTS_NUM 100

static Kernel::FT calc_min_opening(const Trapezoid &trapezoid) {
	return trapezoid.get_id() * 2; // TODO
}

static Kernel::FT calc_max_opening(const Trapezoid &trapezoid) {
	return calc_min_opening(trapezoid) + 10; // TODO
}

void Localizator::init(const std::vector<Point> &points) {
	trapezoids.clear();
	sorted_by_max.clear();
	rtree.clear();

	std::vector<Trapezoid> trapezoids_v;
	trapezoider.calc_trapezoids(points, trapezoids_v);
	for (const auto &trapezoid : trapezoids_v)
		trapezoids[trapezoid.get_id()] = trapezoid;

	DEBUG_PRINT("Trapezoids openings:" << std::endl);
	for (auto &t : trapezoids) {
		auto &trapezoid = t.second;
		trapezoid.opening_min = calc_min_opening(trapezoid);
		trapezoid.opening_max = calc_max_opening(trapezoid);
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]"
						  << std::endl);
	}

	for (const auto &trapezoid : trapezoids)
		sorted_by_max.push_back(trapezoid.first);
	sort(sorted_by_max.begin(), sorted_by_max.end(),
		 [this](const auto &t1, const auto &t2) { return trapezoids[t1].opening_max < trapezoids[t2].opening_max; });
	DEBUG_PRINT("sorted_by_max:" << std::endl);
	for (const auto &t_id : sorted_by_max) {
		const auto &trapezoid = trapezoids[t_id];
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]"
						  << std::endl);
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

void Localizator::query(const Kernel::FT &d, std::vector<Polygon> &res) {
	auto it =
		std::lower_bound(sorted_by_max.begin(), sorted_by_max.end(), d, [this](const auto &trapezoid, const auto &d) {
			return trapezoids[trapezoid].opening_max < d;
		});

	DEBUG_PRINT("Single measurement query (d = " << d << "):" << std::endl);
	for (; it != sorted_by_max.end(); ++it) {
		// Union result for each trapezoid
		const auto &trapezoid = trapezoids[*it];
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]"
						  << std::endl);

		// oriante angles relative to the top edge
		Direction angle_begin = -trapezoid.angle_begin, angle_end = -trapezoid.angle_end;
		assert(calc_half_plane_side(angle_begin, angle_end) == HalfPlaneSide::Left);
		auto s = trapezoid.top_edge->source()->point(), t = trapezoid.top_edge->target()->point();
		Direction top_edge_direction(t.hx() - s.hx(), t.hy() - s.hy());
		assert(is_free(trapezoid.top_edge->face()));
		Direction mid_angle = top_edge_direction.perpendicular(CGAL::LEFT_TURN);

		bool begin_before_mid = calc_half_plane_side(mid_angle, angle_begin) == HalfPlaneSide::Right;
		bool end_after_mid = calc_half_plane_side(mid_angle, angle_end) == HalfPlaneSide::Left;
		Direction angle_intervals[][2] = {
			{begin_before_mid ? angle_begin : mid_angle, end_after_mid ? mid_angle : angle_end},
			{begin_before_mid ? mid_angle : angle_begin, end_after_mid ? angle_end : mid_angle}};

		DEBUG_PRINT("\t\t top edge " << trapezoid.top_edge->curve() << std::endl);
		DEBUG_PRINT("\t\t mid_angle " << mid_angle << " angle_begin " << angle_begin << " angle_end " << angle_end
									  << std::endl);
		DEBUG_PRINT("\t\t begin_before_mid " << begin_before_mid << " end_after_mid " << end_after_mid << std::endl);
		DEBUG_PRINT("\t\t angle_intervals [" << angle_intervals[0][0] << ", " << angle_intervals[0][1] << "] ["
											 << angle_intervals[1][0] << ", " << angle_intervals[1][1] << "]"
											 << std::endl);

		for (auto &angle_interval : angle_intervals) {
			auto a_begin = angle_interval[0], a_end = angle_interval[1];
			if (a_begin == a_end)
				continue;
			auto v_begin = normalize(a_begin.vector()), v_end = normalize(a_end.vector());
			std::vector<Point> left_points, right_points;
			DEBUG_PRINT("\t\t v_begin(" << v_begin << ") v_end(" << v_end << ")" << std::endl);

			const auto LEFT = 0, RIGHT = 1;
			for (auto side : {LEFT, RIGHT}) {
				auto vertex = (side == LEFT ? trapezoid.left_vertex : trapezoid.right_vertex)->point();
				auto &points = side == LEFT ? left_points : right_points;
				auto top_edge_line = trapezoid.top_edge->curve().line();

				if (top_edge_line.has_on(vertex)) {
					DEBUG_PRINT("\t\t" << (side == LEFT ? "left" : "right") << " is arc" << std::endl);
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
					DEBUG_PRINT("\t\t\t C(" << vertex << ") B(" << begin << ") E(" << end << ")" << std::endl);

				} else {
					DEBUG_PRINT("\t\t" << (side == LEFT ? "left" : "right") << " is conchoid" << std::endl);
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

					// DEBUG_PRINT("\t\t\t C(" << vertex << ") a= " << a << " b= " << d << " m= " << m << " pm= " <<
					// pm
					// 						<< " B(" << begin << ") E(" << end << ")" << std::endl);
				}
			}

			Polygon cell;
			Point prev = left_points.front();
			for (unsigned int i = 1; i < left_points.size(); i++) {
				Segment s(prev, left_points[i]);
				cell.push_back(s);
				prev = left_points[i];
			}
			if (prev != right_points.back()) {
				cell.push_back(Segment(prev, left_points.front()));
				prev = left_points.front();
			}
			for (int i = right_points.size() - 2; i >= 0; i--) {
				Segment s(prev, right_points[i]);
				cell.push_back(s);
				prev = right_points[i];
			}

			if (prev != left_points.front())
				cell.push_back(Segment(prev, left_points.front()));

			res.push_back(cell);
		}
	}
}

void Localizator::query(const Kernel::FT &d1, const Kernel::FT &d2, Arrangement &res) {
	const Kernel::FT d = d1 + d2;
	TrapezoidRTreePoint a(d), b(d);
	TrapezoidRTreeSegment s(a, b);
	std::vector<TrapezoidRTreeValue> res_vals;
	rtree.query(boost::geometry::index::intersects(s), std::back_inserter(res_vals));

	DEBUG_PRINT("Two measurements query (d1 = " << d1 << ", d2 = " << d2 << "):" << std::endl);
	for (const TrapezoidRTreeValue &rtree_val : res_vals) {
		// Union result for each trapezoid
		const auto &trapezoid = trapezoids[rtree_val.second];
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]"
						  << std::endl);
		// TODO
	}
}
