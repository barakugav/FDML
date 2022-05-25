#include "meshing_options.h"

int load_options(MeshingOptions& mo, int argc, char** argv) {
    po::options_description desc("Robot localization with implicit manifolds");
    desc.add_options()("help", "display help message");

    desc.add_options()("filename", po::value<std::string>(&mo.filename), "file name of input polygon");
    desc.add_options()("out-filename", po::value<std::string>(&mo.out_filename),
                       "file name of output mesh (in *.off format)");
    desc.add_options()("angle-bound", po::value<FT>(&mo.angle_bound)->default_value(30.),
                       "Angle bound for mesh generation");
    desc.add_options()("distance-bound", po::value<FT>(&mo.distance_bound)->default_value(.04),
                       "Distance bound for mesh generation");
    desc.add_options()("radius-bound", po::value<FT>(&mo.radius_bound)->default_value(.04),
                       "Radius bound for mesh generation");

    desc.add_options()("d1", po::value<FT>(&mo.d1), "First measurement d1 to wall in room");
    desc.add_options()("d2", po::value<FT>(&mo.d2)->default_value(-INFTY),
                       "Second measurement d2 to wall in room (optional)");
    desc.add_options()("alpha", po::value<FT>(&mo.alpha)->default_value(-INFTY),
                       "Alpha rotation for achieving second measurement (optional)");
    desc.add_options()("delta", po::value<FT>(&mo.delta)->default_value(-INFTY),
                       "Delta cutoff for curve intersection (optional)");

    desc.add_options()("sphere-x", po::value<FT>(&mo.sphere_x)->default_value(0),
                       "Bounding sphere x coordinate (optional, default is origin)");
    desc.add_options()("sphere-y", po::value<FT>(&mo.sphere_y)->default_value(0),
                       "Bounding sphere y coordinate (optional, default is origin)");
    desc.add_options()("sphere-z", po::value<FT>(&mo.sphere_z)->default_value(0),
                       "Bounding sphere z coordinate (optional, default is origin)");
    desc.add_options()("sphere-r", po::value<FT>(&mo.sphere_r)->default_value(2.0),
                       "Bounding sphere radius (optional, default is 2.0 units)");

    desc.add_options()("single-measurement", po::value<bool>(&mo.single_measurement)->default_value(true),
                       "If true then take into account only the single d1 measurement");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // if (mo.alpha < 0)
    //   throw ...

    // Display help message
    if (vm.count("help") || !vm.count("filename") || !vm.count("out-filename") || !vm.count("d1")) {
        std::cout << desc << std::endl;
        return -1;
    }

    return 0;
}