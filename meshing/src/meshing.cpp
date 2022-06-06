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
        /****************************************************************
         *   Single measurement mode - return a 2-manifold as output
         *****************************************************************/
        Surface_mesh sm;
        single_measurement(sm, arr, pl, mo.d1, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r*mo.sphere_r,
             mo.angle_bound, mo.radius_bound, mo.distance_bound);

        // single_measurement_marching_cubes(sm, arr, pl, mo.d1, mo.sphere_r, 500);

        std::ofstream out(mo.out_filename);
        out << sm << std::endl;

    } else {
        /************************************************************
         *   Double measurement mode - return a 3D curve as output
         *************************************************************/
        std::cout << "Unsupported mode - DEBUG code only." << std::endl;

        Surface_mesh m_d1, m_d2;

        CGAL::IO::read_polygon_mesh(CGAL::data_file_path("tmp/d1.off"), m_d1); // Load from python::
        CGAL::IO::read_polygon_mesh(CGAL::data_file_path("tmp/d2.off"), m_d2);

        // std::cout << "Computing M_d1..." << std::endl;
        // RUN_TIME(single_measurement, m_d1, arr, pl, mo.d1, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r*mo.sphere_r,
        //          mo.angle_bound, mo.radius_bound, mo.distance_bound);
        // RUN_TIME(single_measurement_marching_cubes, m_d1, arr, pl, mo.d1, mo.sphere_r, mo.mc_n);

        // std::cout << "Computing M_d2..." << std::endl;
        // RUN_TIME(single_measurement_rotate_alpha, m_d2, arr, pl, mo.d2, Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z),
        //          mo.sphere_r*mo.sphere_r, mo.angle_bound, mo.radius_bound, mo.distance_bound, mo.alpha);
        // RUN_TIME(single_measurement_marching_cubes_rotate_alpha, m_d2, arr, pl, mo.d2, mo.alpha, mo.sphere_r, mo.mc_n);

        std::cout << "Computing intersection..." << std::endl;
        Surface_mesh v_square;
        DeltaCube initial_cube(Point_3(-mo.sphere_r, -mo.sphere_r, -mo.sphere_r),
                               Point_3(mo.sphere_r, mo.sphere_r, mo.sphere_r));
        RUN_TIME(manifold_intersection, m_d1, m_d2, v_square, arr, pl, initial_cube, mo.delta, 0.01);

        std::ofstream out(mo.out_filename);
        out << v_square << std::endl;
        std::ofstream out1(mo.out_filename + std::string(".d1.off"));
        out1 << m_d1 << std::endl;
        std::ofstream out2(mo.out_filename + std::string(".d2.off"));
        out2 << m_d2 << std::endl;
    }

    return 0;
}