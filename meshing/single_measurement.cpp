#include "single_measurement.h"
#include "shoot_ray.h"

void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound) {
  // Generate the 2D-complex in 3D-Delaunay triangulation
  Tr tr;
  C2t3 c2t3(tr);

  auto implicit_func = [&arr, &pl, d](Point_3 p) {
    FT theta = p.z() * 2 * M_PI;
    if (theta < 0 || theta > 2 * M_PI)
      return INFTY;
    return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - d);
  };

  // Generate the implicit surface and make surface mesh to c2t3
  Surface_3 surface(implicit_func, Sphere_3(sphere_origin, sphere_radius));

  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(angle_bound, radius_bound, distance_bound);
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  // Convert the c2t3 mesh to a surface mesh
  CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);
}

template <typename TwistFunc>
void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound, TwistFunc twist_func) {
  // Generate the 2D-complex in 3D-Delaunay triangulation
  Tr tr;
  C2t3 c2t3(tr);

  auto implicit_func = [&arr, &pl, d, &twist_func](Point_3 p) {
    p = twist_func(p);
    FT theta = p.z() * 2 * M_PI;
    if (theta < 0 || theta > 2 * M_PI)
      return INFTY;
    return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - d);
  };

  // Generate the implicit surface and make surface mesh to c2t3
  Surface_3 surface(implicit_func, Sphere_3(sphere_origin, sphere_radius));

  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(angle_bound, radius_bound, distance_bound);
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  // Convert the c2t3 mesh to a surface mesh
  CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);
}