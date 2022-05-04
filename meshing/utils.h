#ifndef UTILS_H_
#define UTILS_H_

#include "cgal_include.h"
#include <string>

#define DBG_PRINT_LINE()                                                                                               \
  do {                                                                                                                 \
    std::cout << "line " << __LINE__ << std::endl;                                                                     \
  } while (0)

template <typename ManifoldFunc>
static void write_grid_to_csv(ManifoldFunc manifold_func, unsigned int grid_size, const std::string& outfile);

#endif