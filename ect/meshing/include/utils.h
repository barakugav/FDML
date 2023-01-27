#ifndef UTILS_H_
#define UTILS_H_

#include "cgal_include.h"
#include <chrono>
#include <string>
#include <utility>

#define DBG_PRINT_LINE()                                                                                               \
    do {                                                                                                               \
        std::cout << "line " << __LINE__ << std::endl;                                                                 \
    } while (0)

#define RUN_TIME(func, args...)                                                                                        \
    do {                                                                                                               \
        std::chrono::steady_clock::time_point start, end;                                                              \
        start = std::chrono::steady_clock::now();                                                                      \
        func(args);                                                                                                    \
        end = std::chrono::steady_clock::now();                                                                        \
        std::cout << "Done computing. Tike took: "                                                                     \
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0 << "[sec]" \
                  << std::endl;                                                                                        \
    } while (0)

template <typename ManifoldFunc>
static void write_grid_to_csv(ManifoldFunc manifold_func, unsigned int grid_size, const std::string& outfile);

#endif