#include "defs.h"

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

  private:
	Trapezoid::ID id;

  public:
	Direction angle_begin;
	Direction angle_end;
	Halfedge top_edge;
	Halfedge bottom_edge;
	Vertex left_vertex;
	Vertex right_vertex;
	Kernel::FT opening_max;
	Kernel::FT opening_min;

	static const Direction ANGLE_NONE;

	Trapezoid() = default;
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
	 * @brief Calculate the minimum and maximum opening of this trapezoid, and fill the object fields
	 *
	 * This function should be called after all of the trapezoid's defining fields (top edge, bottom edge, left vertex,
	 * right vertex, start angle, end angle) have been assigned.
	 */
	void calc_min_max_openings();
};

#endif
