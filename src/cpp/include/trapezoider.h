#include "closer_edge.hpp"
#include "defs.h"

typedef unsigned int TrapezoidID;

class Trapezoid {
  private:
	TrapezoidID id;

  public:
	Direction angle_begin;
	Direction angle_end;
	Halfedge_const_handle top_edge;
	Halfedge_const_handle bottom_edge;
	Vertex_const_handle left_vertex;
	Vertex_const_handle right_vertex;
	Kernel::FT opening_max;
	Kernel::FT opening_min;

	Trapezoid() = default;
	Trapezoid(TrapezoidID id, Halfedge_const_handle top_edge, Halfedge_const_handle bottom_edge,
			  Vertex_const_handle left_vertex, Vertex_const_handle right_vertex);
	Trapezoid(const Trapezoid &) = default;
	TrapezoidID get_id() const;
};

struct VertexData {
	TrapezoidID top_left_trapezoid;
	TrapezoidID top_right_trapezoid;
	TrapezoidID bottom_left_trapezoid;
	TrapezoidID bottom_right_trapezoid;
	std::set<Halfedge_const_handle, Closer_edge> ray_edges;
	VertexData() {}
	VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits);
};

class Trapezoider {

  private:
	Arrangement arr;
	std::map<TrapezoidID, Trapezoid> trapezoids;
	std::unordered_map<Vertex_const_handle, VertexData> vertices_data;
	TrapezoidID trapezoids_id_counter;

  public:
	Trapezoider() {}
	void calc_trapezoids(const std::vector<Point> &points, std::vector<Trapezoid> &trapezoids);

  private:
	TrapezoidID create_trapezoid(const Halfedge_const_handle &top_edge, const Halfedge_const_handle &bottom_edge,
								 const Vertex_const_handle &left_vertex, const Vertex_const_handle &right_vertex);
	void finalize_trapezoid(const Trapezoid &trapezoid);
	void init_trapezoids_with_regular_vertical_decomposition();
	void calc_trapezoids_with_rotational_sweep();
};
