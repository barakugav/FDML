#include "closer_edge.hpp"
#include "defs.h"
#include "trapezoid.h"

#ifndef __TRAPEZOIDER_H__
#define __TRAPEZOIDER_H__

struct VertexData {
	Trapezoid::ID top_left_trapezoid;
	Trapezoid::ID top_right_trapezoid;
	Trapezoid::ID bottom_left_trapezoid;
	Trapezoid::ID bottom_right_trapezoid;
	std::set<Halfedge, Closer_edge> ray_edges;
	VertexData() {}
	VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits);
};

class Trapezoider {

  private:
	Arrangement arr;
	std::map<Trapezoid::ID, Trapezoid> trapezoids;
	std::unordered_map<Vertex, VertexData> vertices_data;
	Trapezoid::ID trapezoids_id_counter;

  public:
	Trapezoider() {}
	void calc_trapezoids(const std::vector<Point> &points, std::vector<Trapezoid> &trapezoids);

  private:
	Trapezoid::ID create_trapezoid(const Halfedge &top_edge, const Halfedge &bottom_edge, const Vertex &left_vertex,
								   const Vertex &right_vertex);
	void finalize_trapezoid(const Trapezoid &trapezoid);
	void init_trapezoids_with_regular_vertical_decomposition();
	void calc_trapezoids_with_rotational_sweep();
};

#endif
