#include "trapezoid.h"
#include "CGAL/Boolean_set_operations_2.h"
#include "CGAL/Boolean_set_operations_2/Gps_polygon_validation.h"
#include "utils.hpp"

Trapezoid::Trapezoid(Trapezoid::ID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex)
	: id(id), top_edge(top_edge), bottom_edge(bottom_edge), left_vertex(left_vertex), right_vertex(right_vertex),
	  angle_begin(ANGLE_NONE), angle_end(ANGLE_NONE) {}

Trapezoid::ID Trapezoid::get_id() const { return id; }

void Trapezoid::calc_min_max_openings() {
	opening_min = id * 2;			// TODO
	opening_max = opening_min + 10; // TODO
}

Polygon Trapezoid::get_bounds_2d() const {
	Point t1 = top_edge->source()->point(), t2 = top_edge->target()->point();
	Point b1 = bottom_edge->source()->point(), b2 = bottom_edge->target()->point();
	if (cmp(t1, t2) > 0)
		std::swap(t1, t2);
	if (cmp(b1, b2) > 0)
		std::swap(b1, b2);
	std::vector<Point> points;
	if (t1 == b1)
		points = {t1, t2, b2};
	else if (t2 == b2)
		points = {t1, t2, b1};
	else {
		points = {t1, t2, b2, b1};
		if (CGAL::do_intersect(Segment(t1, b2), Segment(t2, b1)))
			std::swap(t1, t2);
		assert(!CGAL::do_intersect(Segment(t1, b2), Segment(t2, b1)));
	}

	Polygon bounds(points.begin(), points.end());
	CGAL::Gps_default_traits<Polygon>::Traits traits;
	if (!CGAL::has_valid_orientation_polygon(bounds, traits)) {
		bounds.clear();
		bounds.insert(bounds.vertices_end(), points.rbegin(), points.rend());
	}
	return bounds;
}