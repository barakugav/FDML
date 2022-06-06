#ifndef SHOOT_RAY_H_
#define SHOOT_RAY_H_

#include "cgal_include.h"
#include <cmath>

// Returns the distance to a wall in the arrangement when 
// casting a ray in a given direction.
FT shoot_ray(Arrangement* arr, Trap_pl& pl, Point p, FT cos_theta, FT sin_theta);

#endif