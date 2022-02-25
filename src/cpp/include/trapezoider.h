#include "closer_edge.hpp"
#include "defs.h"
#include "trapezoid.h"

#ifndef __TRAPEZOIDER_H__
#define __TRAPEZOIDER_H__

/**
 * @brief The Trapezoider class is an object used to calculate all trapezoids within a given polygon simple room. It
 * should be held in memory as long as the trapezoids are used, as the trapezoids reference to the stored arrangement.
 */
class Trapezoider {
  private:
	/* Struct containing all the data associated with a vertex during the parallel rotational sweep */
	struct VertexData {
		Trapezoid::ID top_left_trapezoid;
		Trapezoid::ID top_right_trapezoid;
		Trapezoid::ID bottom_left_trapezoid;
		Trapezoid::ID bottom_right_trapezoid;
		std::set<Halfedge, Closer_edge> ray_edges;
		VertexData() {}
		VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits);
	};

	/* Arrangement of the simple polygon room, built from the input points */
	Arrangement arr;
	/* Map of the calculated trapezoids within the room */
	std::map<Trapezoid::ID, Trapezoid> trapezoids;
	/* map containing the data associated with each vertex during the parallel rotational sweep */
	std::unordered_map<Vertex, VertexData> vertices_data;
	/* counter for newly created trapezoids */
	Trapezoid::ID trapezoids_id_counter;

  public:
	Trapezoider() {}
	/**
	 * @brief Calculates all the trapezoids that exists in the given room
	 *
	 * @param points list of points representing a simple polygon room
	 * @param trapezoids output vector for the calculated trapezoids
	 */
	void calc_trapezoids(const std::vector<Point> &points, std::vector<Trapezoid> &trapezoids);

  private:
	Trapezoid::ID create_trapezoid(const Halfedge &top_edge, const Halfedge &bottom_edge, const Vertex &left_vertex,
								   const Vertex &right_vertex);
	void finalize_trapezoid(const Trapezoid &trapezoid);
	void init_trapezoids_with_regular_vertical_decomposition();
	void calc_trapezoids_with_rotational_sweep();
};

#endif
