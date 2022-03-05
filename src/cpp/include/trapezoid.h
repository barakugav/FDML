#include "defs.h"
#include "utils.hpp"

#ifndef __TRAPEZOID_H__
#define __TRAPEZOID_H__

/**
 * The Trapezoid class represent a 3D cell in the (x,y,theta) configuration space. A trapezoid is defined by it's top
 * and bottom edges, and the right and left vertices and defines its imaginary rotated parallel edges. Each trapezoid
 * exists in a single angle interval, which we represent by two direction vectors. Along all the possible angles a
 * trapezoid exists in, we calculate the maximum and minimum openings - the distance from the top and bottom edge within
 * the trapezoid.
 */
class Trapezoid {
  public:
	typedef unsigned int ID;
	Trapezoid::ID id;
	Halfedge top_edge;
	Halfedge bottom_edge;
	Vertex left_vertex;
	Vertex right_vertex;
	Direction angle_begin;
	Direction angle_end;

	static const Direction ANGLE_NONE;

	Trapezoid(Trapezoid::ID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex);
	Trapezoid(const Trapezoid &) = default;
	Trapezoid::ID get_id() const;

	/**
	 * @brief Get the 2D bounds polygon of the trapezoid, which should be intersected with the calculated result area
	 * during a localization query.
	 *
	 * @return Polygon that represent the 2D bounds of the trapezoid
	 */
	Polygon get_bounds_2d() const;

	/**
	 * @brief Calculates all the points a sensor might be within the trapezoid measering distance 'd' at the top edge
	 *
	 * @param d the measurement value
	 * @param res output result
	 */
	void calc_result_m1(const Kernel::FT &d, std::vector<Polygon> &res) const;

	/**
	 * @brief Calculate the minimum and maximum opening of this trapezoid
	 *
	 * This function should be called after all of the trapezoid's defining fields (top edge, bottom edge, left vertex,
	 * right vertex, start angle, end angle) have been assigned.
	 *
	 * @param opening_min output for the minimum opening of the trapezoid
	 * @param opening_max output for the maximum opening of the trapezoid
	 */
	void calc_min_max_openings(Kernel::FT &opening_min, Kernel::FT &opening_max) const;
};

template <class OutputStream> OutputStream &operator<<(OutputStream &os, const Trapezoid &trapezoid) {
	os << 'T' << trapezoid.get_id();
	int angle_begin = trapezoid.angle_begin != Trapezoid::ANGLE_NONE ? direction_to_angles(trapezoid.angle_begin) : 0;
	int angle_end = trapezoid.angle_end != Trapezoid::ANGLE_NONE ? direction_to_angles(trapezoid.angle_end) : 0;
	os << " (" << angle_begin << ", " << angle_end << ')';
	os << " t(" << trapezoid.top_edge->curve() << ") b(" << trapezoid.bottom_edge->curve() << ") l("
	   << trapezoid.left_vertex->point() << ") r(" << trapezoid.right_vertex->point() << ')';
	return os;
}

#endif
