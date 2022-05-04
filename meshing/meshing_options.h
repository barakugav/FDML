#ifndef MESHING_OPTIONS_H_
#define MESHING_OPTIONS_H_

#include "cgal_include.h"
#include <boost/program_options.hpp>
#include <string>

namespace po = boost::program_options;

struct MeshingOptions {
  std::string filename, out_filename;
  FT angle_bound, distance_bound, radius_bound;
  FT d1, d2, alpha;
  FT delta;
  FT sphere_x, sphere_y, sphere_z, sphere_r;
  bool single_measurement;
};

int load_options(MeshingOptions& mo, int argc, char** argv);

#endif