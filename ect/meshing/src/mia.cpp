#define CGAL_NDEBUG

#include <chrono>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <string>

#include "cgal_include.h"
#include "manifold_intersection.h"
#include "meshing_options.h"
#include "read_input.h"
#include "shoot_ray.h"
#include "single_measurement.h"
#include "utils.h"


namespace po = boost::program_options;

int main(int argc, char** argv) {

    std::string m_1_path, m_2_path, m_out_path;
    FT radius, delta;

    po::options_description desc("MIA (Manifold Intersection Algorithm)");
    desc.add_options()("help", "display help message");
    desc.add_options()("m1-path", po::value<std::string>(&m_1_path), "file name of first mesh");
    desc.add_options()("m2-path", po::value<std::string>(&m_2_path), "file name of second mesh");
    desc.add_options()("mout-path", po::value<std::string>(&m_out_path), "file name of output mesh");
    desc.add_options()("radius", po::value<FT>(&radius), "Bounding sphere radius");
    desc.add_options()("delta", po::value<FT>(&delta), "Intersection delta size");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return -1;
    }

    // Blank arrangement as we do not take account into intersection general meshes
    Arrangement arr;
    Trap_pl pl(arr);

    Surface_mesh m_1, m_2;

    CGAL::IO::read_OBJ(CGAL::data_file_path(m_1_path), m_1);
    CGAL::IO::read_OBJ(CGAL::data_file_path(m_2_path), m_2);

    std::cout << "Computing intersection..." << std::endl;
    Surface_mesh v_square;
    DeltaCube initial_cube(Point_3(-radius, -radius, -radius),
                            Point_3(radius, radius, radius));
    RUN_TIME(manifold_intersection, m_1, m_2, v_square, arr, pl, initial_cube, delta, 0.01);

    CGAL::IO::write_OBJ(m_out_path.c_str(), v_square);

    return 0;
}