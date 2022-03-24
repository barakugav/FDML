#ifndef FDML_UTILS_HPP
#define FDML_UTILS_HPP

#ifndef M_PI
// windows
#define _USE_MATH_DEFINES
#include <cmath>
#endif
#include <math.h>

#include "fdml/defs.hpp"

#include "CGAL/determinant_of_vectors.h"
#include "CGAL/enum.h"
#include "CGAL/number_utils.h"

namespace FDML {

static const bool DEBUG_PRINT_EN = false;

#define fdml_info(args)                                                 \
  do {                                                                  \
    std::cout << args;                                                  \
  } while (false)
#define fdml_infoln(args) fdml_info(args << std::endl)

#define fdml_err(args)                                                  \
  do {                                                                  \
    std::cerr << args;                                                  \
  } while (false)
#define fdml_errln(args) fdml_err(args << std::endl)

#define fdml_debug(args)                                                \
  do {                                                                  \
    if (DEBUG_PRINT_EN)                                                 \
      std::cout << args;                                                \
  } while (false)
#define fdml_debugln(args) fdml_debug(args << std::endl)
#define fdml_debug_line() fdml_debug(__FILE__ << ":" << __LINE__ << std::endl)

#define FDML_UNUSED(var) (void)var

class Utils {
public:
  template <typename _Vector> static _Vector normalize(_Vector v) {
    Kernel::FT norm = CGAL::approximate_sqrt(v.squared_length());
    return norm > 1e-30 ? v / norm : v;
  }

  /* This function should be used only for debug uses */
  static inline int direction_to_angles(const Direction &dir) {
    double x = CGAL::to_double(dir.dx());
    double y = CGAL::to_double(dir.dy());
    return (int)(std::atan2(y, x) * 180 / M_PI);
  }

  static inline bool is_free(const Face &face) { return !face->is_unbounded(); }
};

} // namespace FDML

#endif
