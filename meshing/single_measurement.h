#ifndef SINGLE_MEASUREMENT_H_
#define SINGLE_MEASUREMENT_H_

#include "cgal_include.h"
#include <math.h>

/*
 * Get a room and a measured distance, and return (by reference) an approximating
 * surface mesh to the implicit manifold of possible locations.
 * Also support twists of the input domain.
 */
void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound);

template <typename TwistFunc>
void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound, TwistFunc twist_func);

#endif