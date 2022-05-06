#ifndef CGAL_INCLUDE_H_
#define CGAL_INCLUDE_H_

#include <boost/optional/optional_io.hpp>

#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/squared_distance_2.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>

#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/intersections.h>

typedef CGAL::Surface_mesh_default_triangulation_3 Tr;
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;
typedef Tr::Geom_traits GT;
typedef GT::Sphere_3 Sphere_3;
typedef GT::Point_3 Point_3;
typedef GT::FT FT;
typedef FT (*Function)(Point_3);
typedef CGAL::Implicit_surface_3<GT, Function> Surface_3;
typedef CGAL::Surface_mesh<Point_3> Surface_mesh;
typedef Surface_mesh::Vertex_index Vertex_descriptor;
typedef Surface_mesh::Face_index Face_descriptor;


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

typedef CGAL::AABB_face_graph_triangle_primitive<Surface_mesh> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> AABB_Traits;
typedef CGAL::AABB_tree<AABB_Traits> Tree;
typedef CGAL::Bbox_3 Bbox_3;

#define INFTY 100000.0

#endif