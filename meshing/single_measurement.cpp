#include "single_measurement.h"
#include "shoot_ray.h"

#define MC_IMPLEM_ENABLE
#define MC_CPP_USE_DOUBLE_PRECISION
#include "MC.h"

void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound) {
  // Generate the 2D-complex in 3D-Delaunay triangulation
  Tr tr;
  C2t3 c2t3(tr);

  auto implicit_func = [&arr, &pl, d](Point_3 p) {
    return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(p.z()), sin(p.z())) - d);
  };

  // Generate the implicit surface and make surface mesh to c2t3
  Surface_3 surface(implicit_func, Sphere_3(sphere_origin, sphere_radius));

  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(angle_bound, radius_bound, distance_bound);
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  // Convert the c2t3 mesh to a surface mesh
  CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);


  // Clip angle value in [0, 2pi]
  /*
  CGAL::Polygon_mesh_processing::clip(sm, 
      Plane_3(
          Point_3(FT(0), FT(0), FT(0)), 
          Vector_3(FT(0), FT(0), FT(-1))
      ));
  CGAL::Polygon_mesh_processing::clip(sm, 
      Plane_3(
          Point_3(FT(0), FT(0), FT(1)), 
          Vector_3(FT(0), FT(0), FT(1))
      ));
  std::cout << "I'm here" << std::endl;*/
}

void single_measurement_marching_cubes(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, FT sphere_radius, unsigned int n)
{
   auto implicit_func = [&arr, &pl, d](Point_3 p) {
    return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(p.z()), sin(p.z())) - d);
  };

    // Based on the code from https://github.com/aparis69/MarchingCubeCpp#readme
  FT* field = new FT[n * n * n];
   for (int i = 0; i < n; i++) {
    std::cout << i << std::endl;
     for (int j = 0; j < n; j++)
       for (int k = 0; k < n; k++) {
         FT x = ((FT)i / ((FT)n - 1) * 2 - 1) * sphere_radius;
         FT y = ((FT)j / ((FT)n - 1) * 2 - 1) * sphere_radius;
         FT z = ((FT)k / ((FT)n - 1) * 2 - 1) * 2 * M_PI;
         field[(k * n + j) * n + i] = implicit_func(Point_3(x, y, z));
         //std::cout << field[(k * n + j) * n + i] << std::endl;
       }
   }
    MC::mcMesh mesh;
    MC::marching_cube(field, n, n, n, mesh);

    std::cout << mesh.indices.size() << std::endl;

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
      Point_3 p1(
          mesh.vertices.at(mesh.indices.at(i)).x, 
          mesh.vertices.at(mesh.indices.at(i)).y,
          mesh.vertices.at(mesh.indices.at(i)).z);
      Point_3 p2(mesh.vertices.at(mesh.indices.at(i+1)).x, mesh.vertices.at(mesh.indices.at(i+1)).y,
                 mesh.vertices.at(mesh.indices.at(i+1)).z);
      Point_3 p3(mesh.vertices.at(mesh.indices.at(i+2)).x, mesh.vertices.at(mesh.indices.at(i+2)).y,
                 mesh.vertices.at(mesh.indices.at(i+2)).z);

      Surface_mesh::Vertex_index u = sm.add_vertex(p1);
      Surface_mesh::Vertex_index v = sm.add_vertex(p2);
      Surface_mesh::Vertex_index w = sm.add_vertex(p3);
      sm.add_face(u, v, w);
    }
}

void single_measurement_rotate_alpha(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin,
                                     FT sphere_radius, FT angle_bound, FT radius_bound, FT distance_bound, FT alpha) {
  // Generate the 2D-complex in 3D-Delaunay triangulation
  Tr tr;
  C2t3 c2t3(tr);

  auto implicit_func = [&arr, &pl, d, alpha](Point_3 p) {
    return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(p.z() + alpha), sin(p.z() + alpha)) - d);
  };

  // Generate the implicit surface and make surface mesh to c2t3
  Surface_3 surface(implicit_func, Sphere_3(sphere_origin, sphere_radius));

  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(angle_bound, radius_bound, distance_bound);
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  // Convert the c2t3 mesh to a surface mesh
  CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);

  // Clip angle value in [alpha, 2pi+alpha]
  /* CGAL::Polygon_mesh_processing::clip(
      sm, Plane_3(Point_3(FT(0), FT(0), FT(alpha / (2.0 * M_PI))), Vector_3(FT(0), FT(0), FT(-1))));
  CGAL::Polygon_mesh_processing::clip(
      sm, Plane_3(Point_3(FT(0), FT(0), FT(1 + alpha / (2.0 * M_PI))), Vector_3(FT(0), FT(0), FT(1))));*/
}

void single_measurement(Surface_mesh& sm, Arrangement& arr, Trap_pl& pl, FT d, Point_3 sphere_origin, FT sphere_radius,
                        FT angle_bound, FT radius_bound, FT distance_bound,
                        boost::function<Point_3(Point_3)> twist_func) {
  // Generate the 2D-complex in 3D-Delaunay triangulation
  Tr tr;
  C2t3 c2t3(tr);

  auto implicit_func = [&arr, &pl, d, twist_func](Point_3 p) {
    // First push theta to [0,2pi] and clamp
    FT theta = p.z() * 2 * M_PI;
    if (theta < 0 || theta > 2 * M_PI)
      return INFTY;

    // Now we can twist the (x,y,theta) domain however we'd like
    p = twist_func(p);
    return (FT)(shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - d);
  };

  // Generate the implicit surface and make surface mesh to c2t3
  Surface_3 surface(implicit_func, Sphere_3(sphere_origin, sphere_radius));

  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(angle_bound, radius_bound, distance_bound);
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  // Convert the c2t3 mesh to a surface mesh
  CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);
}