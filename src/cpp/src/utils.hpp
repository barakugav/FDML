#ifndef M_PI
// windows
#define _USE_MATH_DEFINES
#include <cmath>
#endif
#include "defs.h"
#include <math.h>

#include "CGAL/determinant_of_vectors.h"
#include "CGAL/enum.h"
#include "CGAL/number_utils.h"

#ifndef __FDML_UTILS_HPP__
#define __FDML_UTILS_HPP__

namespace FDML {

#define DEBUG_PRINT_EN 1

#define info(args)                                                                                                     \
	do {                                                                                                               \
		std::cout << args;                                                                                             \
	} while (false)
#define infoln(args) info(args << std::endl)

#define debug(args)                                                                                                    \
	do {                                                                                                               \
		if (DEBUG_PRINT_EN)                                                                                            \
			std::cout << args;                                                                                         \
	} while (false)
#define debugln(args) debug(args << std::endl)
#define debug_line() debug(__FILE__ << ":" << __LINE__ << std::endl)

#define UNUSED(var) (void)var

enum HalfPlaneSide {
	None, // exactly on plane
	Left,
	Right,
};

inline enum HalfPlaneSide calc_half_plane_side(const Direction &angle, const Direction &p) {
	CGAL::Sign s = CGAL::sign_of_determinant_of_vectors<Kernel::FT>(angle.vector(), p.vector());
	return s == CGAL::NEGATIVE ? HalfPlaneSide::Right : s == CGAL::POSITIVE ? HalfPlaneSide::Left : HalfPlaneSide::None;
}

template <typename _Vector> static _Vector normalize(_Vector v) {
	Kernel::FT norm = CGAL::approximate_sqrt(v.squared_length());
	return norm > 1e-30 ? v / norm : v;
}

/* This function should be used only for debug uses */
inline int direction_to_angles(const Direction &dir) {
	double x = CGAL::to_double(dir.dx());
	double y = CGAL::to_double(dir.dy());
	return (int)(std::atan2(y, x) * 180 / M_PI);
}

inline bool is_free(Face face) { return !face->is_unbounded(); }

} // namespace FDML

#endif
