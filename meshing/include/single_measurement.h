#ifndef SINGLE_MEASUREMENT_H_
#define SINGLE_MEASUREMENT_H_

#define _USE_MATH_DEFINES

#include "cgal_include.h"
#include "shoot_ray.h"
#include <CGAL/Polygon_mesh_processing/clip.h>
#include <boost/function.hpp>
#include <cmath>
#include <functional>

// Generates the surface mesh from the implicit function which gets a distance of `d` from the walls of
// the room defined in `arr`.
// The template `MeshingAlgorithm` is a class or a function that should overload the () operator
// with the following signature:
//      meshing(Surface_mesh& sm, boost::function<FT(Point_3)> f)
template <typename MeshingAlgorithm>
void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, MeshingAlgorithm meshing,
                        boost::function<Point_3(Point_3)> transformation = 0) {
    // Define the implicit function (with or without the pre-transformation)
    boost::function<FT(Point_3)> implicit_function;
    if (transformation) {
        implicit_function = boost::function<FT(Point_3)>([&arr, &pl, d, transformation](Point_3 p) {
            p = Point_3(p.x(), p.y(), p.z() * 2 * M_PI);
            p = transformation(p);
            return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(p.z()), sin(p.z())) - d);
        });
    } else {
        implicit_function = boost::function<FT(Point_3)>([&arr, &pl, d](Point_3 p) {
            p = Point_3(p.x(), p.y(), p.z() * 2 * M_PI);
            return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(p.z()), sin(p.z())) - d);
        });
    }

    // Apply the meshing algorithm
    meshing(sm, implicit_function);
}

class MarchingCubesMeshing {
public:
    MarchingCubesMeshing(unsigned int n, FT sphere_radius);
    void operator()(Surface_mesh& sm, boost::function<FT(Point_3)> f);

private:
    unsigned int n;
    FT sphere_radius;
};

class DelaunayMeshing {
  public:
    DelaunayMeshing(Point_3 sphere_origin, FT sphere_radius, FT angle_bound, FT radius_bound, FT distance_bound);
    void operator()(Surface_mesh& sm, boost::function<FT(Point_3)> f);

  private:
    Point_3 sphere_origin;
    FT sphere_radius;
    FT angle_bound, radius_bound, distance_bound;
};

class DelaunayMeshing3 {
    public:
        DelaunayMeshing3(Point_3 sphere_origin, FT sphere_radius);
        void operator()(Surface_mesh& sm, boost::function<FT(Point_3)> f);
    
    private:
        Point_3 sphere_origin;
        FT sphere_radius;
};

/*
void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound);

void single_measurement_marching_cubes(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, FT sphere_radius,
                                       unsigned int n);

void single_measurement_marching_cubes_rotate_alpha(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, FT alpha,
                                                    FT sphere_radius, unsigned int n);

void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound,
                        boost::function<Point_3(Point_3)> twist_func);

void single_measurement_rotate_alpha(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin,
                                     FT sphere_radius, FT angle_bound, FT radius_bound, FT distance_bound, FT alpha);
*/

#endif