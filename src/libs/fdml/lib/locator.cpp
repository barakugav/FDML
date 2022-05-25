#include "fdml/locator.hpp"
#include "fdml/internal/utils.hpp"

namespace FDML {

void Locator::init(const Polygon_with_holes& scene) {
    fdml_infoln("[Locator] init...");
    sorted_by_max.clear();
    rtree.clear();

    /* Calculate all trapezoids */
    trapezoider.calc_trapezoids(scene);

    /* Fill trapezoids data structure and calculate min and max opening */
    for (unsigned int i = 0; i < trapezoider.number_of_trapezoids(); i++) {
        const auto& trapezoid = *trapezoider.get_trapezoid(i);
        Kernel::FT min, max;
        trapezoid.calc_min_max_openings(min, max);
        openings.emplace_back(min.exact(), max.exact());
    }

    fdml_debugln("[Locator] Trapezoids openings:");
    for (auto it = trapezoider.trapezoids_begin(); it != trapezoider.trapezoids_end(); ++it) {
        struct TrapezoidOpening& opening = openings.at(it->get_id());
        fdml_debugln("\tT" << it->get_id() << " [" << opening.min << ", " << opening.max << "]");
    }

    /* Populate the array of trapezoids sorted by their max opening. used for fast queries with one measurement */
    for (auto it = trapezoider.trapezoids_begin(); it != trapezoider.trapezoids_end(); ++it)
        sorted_by_max.push_back(it->get_id());
    sort(sorted_by_max.begin(), sorted_by_max.end(),
         [this](const auto& t1, const auto& t2) { return openings.at(t1).max < openings.at(t2).max; });
    fdml_debugln("[Locator] sorted_by_max:");
    for (const auto& t_id : sorted_by_max) {
        const auto& opening = openings.at(t_id);
        fdml_debugln("\tT" << t_id << " [" << opening.min << ", " << opening.max << "]");
    }

    /* Populate interval tree of trapezoids, where each interval is [min opening, max opening] used for fast queries
     * with two measurements. */
    for (auto it = trapezoider.trapezoids_begin(); it != trapezoider.trapezoids_end(); ++it) {
        const auto& opening = openings.at(it->get_id());
        TrapezoidRTreePoint min(CGAL::to_double(opening.min));
        TrapezoidRTreePoint max(CGAL::to_double(opening.max));
        rtree.insert(TrapezoidRTreeValue(TrapezoidRTreeSegment(min, max), it->get_id()));
    }

    fdml_infoln("[Locator] init done");
}

std::vector<Locator::Res1d> Locator::query(const Kernel::FT& d) const {
    /* Single measurement query. Perform binary search on the sorted array for output sensitive running time */
    fdml_infoln("[Locator] Single measurement query (d = " << d << "):");
    auto it = std::lower_bound(sorted_by_max.begin(), sorted_by_max.end(), d,
                               [this](const auto& t_id, const auto& d) { return openings.at(t_id).max < d; });

    std::vector<Locator::Res1d> res;
    for (; it != sorted_by_max.end(); ++it) {
        const auto& trapezoid = *trapezoider.get_trapezoid(*it);
        const auto& opening = openings.at(trapezoid.get_id());
        fdml_debugln("\tT" << trapezoid.get_id() << " [" << opening.min << ", " << opening.max << "]");

        std::pair<Point, Point> edge_pair(
            {trapezoid.top_edge->source()->point(), trapezoid.top_edge->target()->point()});
        for (const Polygon& res_p : trapezoid.calc_result_m1(d))
            res.emplace_back(edge_pair, res_p);
    }

    fdml_infoln("[Locator] result consist of " << res.size() << " polygons.");
    return res;
}

std::vector<Locator::Res2d> Locator::query(const Kernel::FT& d1, const Kernel::FT& d2) const {
    /* Double measurement query. Use the interval tree for output sensitive running time */
    fdml_infoln("[Locator] Double measurement query (d1 = " << d1 << ", d2 = " << d2 << "):");
    const Kernel::FT d = d1 + d2;
    TrapezoidRTreePoint a(d), b(d);
    TrapezoidRTreeSegment query_interval(a, b);
    std::vector<TrapezoidRTreeValue> res_vals;
    rtree.query(boost::geometry::index::intersects(query_interval), std::back_inserter(res_vals));

    std::vector<Locator::Res2d> res;
    for (const TrapezoidRTreeValue& rtree_val : res_vals) {
        const auto& trapezoid = *trapezoider.get_trapezoid(rtree_val.second);
        const auto& opening = openings.at(rtree_val.second);
        fdml_debugln("\tT" << rtree_val.second << " [" << opening.min << ", " << opening.max << "]");

        std::pair<Point, Point> top_edge_pair(
            {trapezoid.top_edge->source()->point(), trapezoid.top_edge->target()->point()});
        std::pair<Point, Point> bottom_edge_pair(
            {trapezoid.bottom_edge->source()->point(), trapezoid.bottom_edge->target()->point()});
        res.emplace_back(top_edge_pair, bottom_edge_pair, trapezoid.calc_result_m2(d1, d2));
    }

    return res;
}

} // namespace FDML
