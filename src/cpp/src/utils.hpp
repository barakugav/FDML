#include "defs.h"
#include <math.h>
#ifndef M_PI
// windows
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#define DEBUG_PRINT_EN 1
#define debug_print(args)                                                                                              \
	do {                                                                                                               \
		if (DEBUG_PRINT_EN)                                                                                            \
			std::cout << args;                                                                                         \
	} while (false)
#define debug_println(args) debug_print(args << std::endl)
#define debug_print_line() debug_print(__FILE__ << ":" << __LINE__ << std::endl)

enum HalfPlaneSide {
	None, // exactly on plane
	Left,
	Right,
};

static int sign(Kernel::FT val) { return (Kernel::FT(0) < val) - (val < Kernel::FT(0)); }

static int cmp(const Kernel::FT &x1, const Kernel::FT &x2) { return x1 == x2 ? 0 : x1 < x2 ? -1 : 1; }

static int cmp(const Point &p1, const Point &p2) {
	int c;
	return (c = cmp(p1.hx(), p2.hx())) != 0 ? c : cmp(p1.hy(), p2.hy());
}

static enum HalfPlaneSide calc_half_plane_side(const Direction &angle, const Direction &p) {
	// determinant of vectors
	int s = sign(angle.dx() * p.dy() - angle.dy() * p.dx());
	return s == -1 ? HalfPlaneSide::Right : s == 1 ? HalfPlaneSide::Left : HalfPlaneSide::None;
}

static Kernel::FT sqrt(Kernel::FT x) {
	// TODO this function losses precision, maybe can use CGAL::Algebraic_structure_traits<Kernel::FT>::Sqrt
	return std::sqrt(x.exact().convert_to<double>());
}

template <typename _Vector> static _Vector normalize(_Vector v) {
	Kernel::FT norm = sqrt(v.squared_length());
	return norm > 1e-30 ? v / norm : v;
}

static bool is_free(Face face) { return !face->is_unbounded(); }

#endif
