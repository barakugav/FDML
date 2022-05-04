#include <fstream>
#include <math.h>
#include <string>
#include <vector>

#include "cgal_include.h"
#include "meshing_options.h"
#include "read_input.h"
#include "shoot_ray.h"
#include "single_measurement.h"
#include "utils.h"

int main(int argc, char** argv) {
  MeshingOptions mo;
  if (load_options(mo, argc, argv) < 0)
    return 1;

  Arrangement arr;
  load_poly_to_arrangement(mo.filename, &arr);
  Trap_pl pl(arr);

  Surface_mesh sm;
  single_measurement(sm, arr, pl, mo.d1, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r, mo.angle_bound,
                     mo.radius_bound, mo.distance_bound, twist);

  std::ofstream out(mo.out_filename);
  out << sm << std::endl;

  return 0;
}