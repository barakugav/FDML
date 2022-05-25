#include <chrono>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>

#include "cgal_include.h"
#include "manifold_intersection.h"
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

  if (mo.single_measurement) {
    Surface_mesh sm;
    //single_measurement(sm, arr, pl, mo.d1, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r, mo.angle_bound,
    //                   mo.radius_bound, mo.distance_bound);

    single_measurement_marching_cubes(sm, arr, pl, mo.d1, mo.sphere_r, 500);

    std::ofstream out(mo.out_filename);
    out << sm << std::endl;
  } else {
    std::cout << "Unsupported mode - DEBUG code only." << std::endl;

    boost::function<Point_3(Point_3)> rotate_alpha = [&](Point_3 p) {
      FT theta = p.z() + mo.alpha;
      // if (theta > 1.0)
      //   theta -= 1.0;
      return Point_3(p.x(), p.y(), theta);
    };

    std::chrono::steady_clock::time_point start, end;

    Surface_mesh m_d1, m_d2;

    // Load from python::
    CGAL::IO::read_polygon_mesh(CGAL::data_file_path("d1_.off"), m_d1);
    CGAL::IO::read_polygon_mesh(CGAL::data_file_path("d2_.off"), m_d2);

    // std::cout << "Computing M_d1..." << std::endl;
    // start = std::chrono::steady_clock::now();
    // single_measurement(m_d1, arr, pl, mo.d1, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r,
    //                    mo.angle_bound, mo.radius_bound, mo.distance_bound);
    // end = std::chrono::steady_clock::now();
    // std::cout << "Done computing M_d1. Time took: "
    //           << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0 << "[sec]"
    //           << std::endl;

    // std::cout << "Computing M_d2..." << std::endl;
    // start = std::chrono::steady_clock::now();
    // single_measurement_rotate_alpha(m_d2, arr, pl, mo.d2, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r,
    //                    mo.angle_bound, mo.radius_bound, mo.distance_bound, mo.alpha);
    // end = std::chrono::steady_clock::now();
    // std::cout << "Done computing M_d2. Time took: "
    //           << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0 << "[sec]"
    //           << std::endl;

    Surface_mesh v_square;
    DeltaCube initial_cube(Point_3(-mo.sphere_r, -mo.sphere_r, -mo.sphere_r), Point_3(mo.sphere_r, mo.sphere_r, mo.sphere_r));
    manifold_intersection(m_d1, m_d2, v_square, initial_cube, mo.delta, 0.01);

    std::ofstream out(mo.out_filename);
    out << v_square << std::endl;
    std::ofstream out1("out/m_d1.off");
    out1 << m_d1 << std::endl;
    std::ofstream out2("out/m_d2.off");
    out2 << m_d2 << std::endl;
  }

  return 0;
}