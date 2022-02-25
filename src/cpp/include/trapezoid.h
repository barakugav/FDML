#include "defs.h"

#ifndef __TRAPEZOID_H__
#define __TRAPEZOID_H__

static const Direction ANGLE_NONE(0, 0);

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

	Trapezoid() = default;
	Trapezoid(Trapezoid::ID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex);
	Trapezoid(const Trapezoid &) = default;
	Trapezoid::ID get_id() const;

	Polygon get_bounds_2d() const;
	void calc_min_max_openings();
};

#endif
