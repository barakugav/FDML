// #include <boost/optional/optional_io.hpp>
#include <boost/program_options.hpp>

#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/squared_distance_2.h>

#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/intersections.h>

#include <fstream>
#include <math.h>
#include <string>
#include <vector>

namespace po = boost::program_options;

// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr;
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;
typedef Tr::Geom_traits GT;
typedef GT::Sphere_3 Sphere_3;
typedef GT::Point_3 Point_3;
typedef GT::FT FT;
typedef FT (*Function)(Point_3);
typedef CGAL::Implicit_surface_3<GT, Function> Surface_3;
typedef CGAL::Surface_mesh<Point_3> Surface_mesh;

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT Number_type;
typedef CGAL::Arr_non_caching_segment_traits_2<Kernel> Traits;
typedef Traits::Point_2 Point;
typedef Traits::X_monotone_curve_2 Segment;
typedef CGAL::Arrangement_2<Traits> Arrangement;
typedef Arrangement::Vertex_handle Vertex_handle;
typedef Arrangement::Halfedge_handle Halfedge_handle;
typedef Arrangement::Face_handle Face_handle;
typedef CGAL::Arr_trapezoid_ric_point_location<Arrangement> Trap_pl;

#define PI    3.14159265
#define INFTY 1000.0

void load_poly_to_arrangement(std::string& filename, Arrangement* arr) {
  std::vector<Point> vertices;
  std::vector<Segment> segments;

  std::ifstream in(filename);
  while (in.good()) {
    float x, y;
    in >> x >> y;

    Point pt(x, y);
    vertices.push_back(pt);
  }

  for (int i = 0; i < vertices.size(); ++i) {
    Point p = vertices[i];
    Point q = vertices[(i + 1) % vertices.size()];

    Segment s(p, q);
    segments.push_back(s);
  }

  CGAL::insert(*arr, segments.begin(), segments.end());
}

FT shoot_ray(Arrangement* arr, Trap_pl& pl, Point p, FT cos_theta, FT sin_theta) {
  Kernel::Compute_squared_distance_2 squared_distance;
  FT dist(INFTY);

  // If we are out of bounds - return infinity
  auto obj = pl.locate(p);
  auto f = boost::get<Arrangement::Face_const_handle>(&obj);
  if (f && (*f)->is_unbounded())
    return dist;

  // Traverse all edges of the arrangement
  for (auto eit = arr->edges_begin(); eit != arr->edges_end(); ++eit) {
    Segment s = eit->curve();

    // If when rotating in (pi/2 - theta) we are in the corresponding x_range
    if ((s.source().x() * sin_theta - s.source().y() * cos_theta <= p.x() * sin_theta - p.y() * cos_theta) &&
            (p.x() * sin_theta - p.y() * cos_theta <= s.target().x() * sin_theta - s.target().y() * cos_theta) ||
        (s.target().x() * sin_theta - s.target().y() * cos_theta <= p.x() * sin_theta - p.y() * cos_theta) &&
            (p.x() * sin_theta - p.y() * cos_theta <= s.source().x() * sin_theta - s.source().y() * cos_theta)) {
      // Compute the ray result in the rotated space
      FT p_x = s.source().x() * sin_theta - s.source().y() * cos_theta;
      FT p_y = s.source().x() * cos_theta + s.source().y() * sin_theta;
      FT q_x = s.target().x() * sin_theta - s.target().y() * cos_theta;
      FT q_y = s.target().x() * cos_theta + s.target().y() * sin_theta;
      FT v_x = p.x() * sin_theta - p.y() * cos_theta;
      FT v_y = p.x() * cos_theta + p.y() * sin_theta;

      FT m = (q_y - p_y) / (q_x - p_x);
      FT b = p_y - m * p_x;

      FT tmp = m * v_x + b - v_y;

      // If we are above and better than what is already found - update
      if (tmp > 0 && tmp < dist)
        dist = tmp;
    }
  }

  return dist;
}

struct MeshingOptions {
  std::string filename, out_filename;
  FT angle_bound, distance_bound, radius_bound;
  FT d1, d2, alpha;
  FT delta;
  FT sphere_x, sphere_y, sphere_z, sphere_r;
  bool single_measurement;
};

int main(int argc, char** argv) {

  /*
   * Load arguments
   */
  MeshingOptions mo;
  po::options_description desc("Robot localization with implicit manifolds");
  desc.add_options()("help", "display help message")

      ("filename", po::value<std::string>(&mo.filename), "file name of input polygon")(
          "out-filename", po::value<std::string>(&mo.out_filename), "file name of output mesh (in *.off format)")

          ("angle-bound", po::value<FT>(&mo.angle_bound)->default_value(30.), "Angle bound for mesh generation")(
              "distance-bound", po::value<FT>(&mo.distance_bound)->default_value(.04),
              "Distance bound for mesh generation")("radius-bound", po::value<FT>(&mo.radius_bound)->default_value(.04),
                                                    "Radius bound for mesh generation")

              ("d1", po::value<FT>(&mo.d1),
               "First measurement d1 to wall in room")("d2", po::value<FT>(&mo.d2)->default_value(-INFTY),
                                                       "Second measurement d2 to wall in room (optional)")(
                  "alpha", po::value<FT>(&mo.alpha)->default_value(-INFTY),
                  "Alpha rotation for achieving second measurement (optional)")(
                  "delta", po::value<FT>(&mo.delta)->default_value(-INFTY),
                  "Delta cutoff for curve intersection (optional)")

                  ("sphere-x", po::value<FT>(&mo.sphere_x)->default_value(0),
                   "Bounding sphere x coordinate (optional, default is origin)")(
                      "sphere-y", po::value<FT>(&mo.sphere_y)->default_value(0),
                      "Bounding sphere y coordinate (optional, default is origin)")(
                      "sphere-z", po::value<FT>(&mo.sphere_z)->default_value(0),
                      "Bounding sphere z coordinate (optional, default is origin)")(
                      "sphere-r", po::value<FT>(&mo.sphere_r)->default_value(2.0),
                      "Bounding sphere radius (optional, default is 2.0 units)")

                      ("single-measurement", po::value<bool>(&mo.single_measurement)->default_value(true),
                       "If true then take into account only the single d1 measurement");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  // Display help message
  if (vm.count("help") || !vm.count("filename") || !vm.count("out-filename") || !vm.count("d1")) {
    std::cout << desc << std::endl;
    return 1;
  }

  // Load polygon
  Arrangement arr;
  load_poly_to_arrangement(mo.filename, &arr);
  Trap_pl pl(arr);

  // Generate the 2D-complex in 3D-Delaunay triangulation
  Tr tr;
  C2t3 c2t3(tr);

  // Generate the implicit surface and make surface mesh to c2t3
  Surface_3 surface(
      [&arr, &pl, mo](Point_3 p) {
        if (mo.single_measurement) {
          // Single measurement implicit manifold
          FT theta = p.z() * 2 * PI;
          if (theta < 0 || theta > 2 * PI)
            return INFTY;
          return shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - mo.d1;
        } else {
          // Double measurement implicit manifold
          FT theta = p.z() * 2 * PI;
          if (theta < 0 || theta > 2 * PI)
            return INFTY;

          FT theta_plus = p.z() * 2 * PI + mo.alpha;
          if (theta_plus < 0 || theta_plus > 2 * PI)
            return INFTY;

          FT f_d1 = shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - mo.d1;
          FT f_d2 = shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta_plus), sin(theta_plus)) - mo.d2;

          return (f_d1 * f_d1) + (f_d2 * f_d2) - mo.delta;
        }
      },
      Sphere_3(Point_3(mo.sphere_x, mo.sphere_y, mo.sphere_z), mo.sphere_r));

  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(mo.angle_bound, mo.radius_bound, mo.distance_bound);
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  // Convert the c2t3 mesh to a mesh
  Surface_mesh sm;
  CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);

  // Export to off file
  std::ofstream out(mo.out_filename);
  out << sm << std::endl;
}