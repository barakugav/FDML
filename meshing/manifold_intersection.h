#ifndef MANIFOLD_INTERSECTION_
#define MANIFOLD_INTERSECTION_

#include "cgal_include.h"
#include <vector>

struct DeltaCube {
public:
  DeltaCube(Point_3 bottom_left, Point_3 top_right);

  void to_surface_mesh(Surface_mesh& sm);
  void split(std::vector<DeltaCube>& list);
  Bbox_3 to_bbox_3();
  FT size();

  Point_3 bottom_left, top_right;
};

bool is_cube_intersecting(Surface_mesh& M, DeltaCube cube);

void manifold_intersection(Surface_mesh& M_1, Surface_mesh& M_2, Surface_mesh& M_isect,
    DeltaCube initial_cube, FT delta, FT epsilon);

#endif