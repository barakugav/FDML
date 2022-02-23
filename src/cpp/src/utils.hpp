#include "defs.h"


#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#define DEBUG_PRINT_EN 1
#define DEBUG_PRINT(args)                                                                                              \
	do {                                                                                                               \
		if (DEBUG_PRINT_EN)                                                                                            \
			std::cout << args;                                                                                         \
	} while (false)
#define DEBUG_PRINTLN(args) \
	DEBUG_PRINT(args << std::endl)
#define DEBUG_PRINT_LINE() \
	DEBUG_PRINT(__FILE__ << ":" << __LINE__ << std::endl)

enum HalfPlaneSide {
	None, // exactly on plane
	Left,
	Right,
};

static int sign(Kernel::FT val) { return (Kernel::FT(0) < val) - (val < Kernel::FT(0)); }

static enum HalfPlaneSide calc_half_plane_side(const Direction &angle, const Direction &p) {
	// determinant of vectors
	int s = sign(angle.dx() * p.dy() - angle.dy() * p.dx());
	return s == -1 ? HalfPlaneSide::Right : s == 1 ? HalfPlaneSide::Left : HalfPlaneSide::None;
}

static bool is_free(Face_const_handle face) { return !face->is_unbounded(); }

#endif
