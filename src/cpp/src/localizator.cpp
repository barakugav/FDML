#include "localizator.h"
#include "utils.hpp"

void Localizator::init(const std::vector<Point> &points) {
	trapezoids.clear();
	sorted_by_max.clear();
	rtree.clear();

	std::vector<Trapezoid> trapezoids_v;
	trapezoider.calc_trapezoids(points, trapezoids_v);
	for (const auto &trapezoid : trapezoids_v)
		trapezoids[trapezoid.get_id()] = trapezoid;

	for (auto &t : trapezoids) {
		auto &trapezoid = t.second;
		trapezoid.calc_min_max_openings();
	}

	debugln("Trapezoids openings:");
	for (auto &t : trapezoids)
		debugln("\tT" << t.second.get_id() << " [" << t.second.opening_min << ", " << t.second.opening_max << "]");

	for (const auto &trapezoid : trapezoids)
		sorted_by_max.push_back(trapezoid.first);
	sort(sorted_by_max.begin(), sorted_by_max.end(), [this](const auto &t1, const auto &t2) {
		return trapezoids.at(t1).opening_max < trapezoids.at(t2).opening_max;
	});
	debugln("sorted_by_max:");
	for (const auto &t_id : sorted_by_max) {
		const Trapezoid &trapezoid = trapezoids.at(t_id);
		debugln("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]");
	}

	for (const auto &t : trapezoids) {
		const auto &trapezoid = t.second;
		TrapezoidRTreePoint min(trapezoid.opening_min.exact().convert_to<double>());
		TrapezoidRTreePoint max(trapezoid.opening_max.exact().convert_to<double>());
		rtree.insert(TrapezoidRTreeValue(TrapezoidRTreeSegment(min, max), t.first));
	}
}

void Localizator::query(const Kernel::FT &d, std::vector<Polygon> &res) const {
	auto it =
		std::lower_bound(sorted_by_max.begin(), sorted_by_max.end(), d, [this](const auto &trapezoid, const auto &d) {
			return trapezoids.at(trapezoid).opening_max < d;
		});

	debugln("Single measurement query (d = " << d << "):");
	for (; it != sorted_by_max.end(); ++it) {
		// Union result for each trapezoid
		const auto &trapezoid = trapezoids.at(*it);
		debugln("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]");
		trapezoid.calc_result_m1(d, res);
	}
}

void Localizator::query(const Kernel::FT &d1, const Kernel::FT &d2, Arrangement &res) const {
	const Kernel::FT d = d1 + d2;
	TrapezoidRTreePoint a(d), b(d);
	TrapezoidRTreeSegment s(a, b);
	std::vector<TrapezoidRTreeValue> res_vals;
	rtree.query(boost::geometry::index::intersects(s), std::back_inserter(res_vals));

	debugln("Two measurements query (d1 = " << d1 << ", d2 = " << d2 << "):");
	for (const TrapezoidRTreeValue &rtree_val : res_vals) {
		// Union result for each trapezoid
		const auto &trapezoid = trapezoids.at(rtree_val.second);
		debugln("\tT" << trapezoid.get_id() << " [" << trapezoid.opening_min << ", " << trapezoid.opening_max << "]");
		// TODO
	}
}
