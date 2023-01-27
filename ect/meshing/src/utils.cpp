#include "utils.h"

template <typename ManifoldFunc>
static void write_grid_to_csv(ManifoldFunc manifold_func, unsigned int grid_size, const std::string& outfile) {
    std::ofstream grid_file(outfile);
    for (unsigned int x = 0; x < grid_size; x++) {
        std::cout << x << std::endl;
        for (unsigned int y = 0; y < grid_size; y++) {
            for (unsigned int t = 0; t < grid_size; t++) {
                double val = manifold_func(
                    Point_3((x / (FT)grid_size - 0.5) * 2.0, (y / (FT)grid_size - 0.5) * 2.0, (t / (FT)grid_size)));
                grid_file << x << "," << y << "," << t << "," << val << std::endl;
            }
        }
    }
}