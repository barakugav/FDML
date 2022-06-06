#ifndef MANIFOLD_INTERSECTION_
#define MANIFOLD_INTERSECTION_

#include "cgal_include.h"
#include <vector>

// Represents a $\delta$-cube in 3D space, i.e. a cube with edge length of delta
// at each dimension. Note that this length delta is implicit in the implementation of the cube
// as we store only two extreme vertices of the cube.
struct DeltaCube {
  public:
    DeltaCube(Point_3 bottom_left, Point_3 top_right);

    // Appends the cube geometry to a given surface mesh,
    // old geometry in surface mesh remains unmodified.
    void to_surface_mesh(Surface_mesh& sm) const;

    // Appends to the given list eight new cubes, which are generated
    // by splitting equally the given cube in each dimension. 
    void split(std::vector<DeltaCube>& list) const;

    // Casts to CGAL's Bbox_3 object
    Bbox_3 to_bbox_3() const;

    // Returns the delta size of the cube. We assume that all dimensions are equal.
    FT size() const;

    // Returns the midpoint of the cube
    Point_3 midpoint() const;

    Point_3 bottom_left, top_right;
};

// Appends the delta-close manifold intersection approximation, 
// given two manifold approximations
void manifold_intersection(Surface_mesh& M_1, Surface_mesh& M_2, Surface_mesh& M_isect, Arrangement& arr, Trap_pl& pl,
                           DeltaCube initial_cube, FT delta, FT epsilon);

#endif