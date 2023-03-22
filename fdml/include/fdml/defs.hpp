#ifndef FDML_DEFS_HPP
#define FDML_DEFS_HPP

#include <list>

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

namespace FDML {

#ifdef __linux__
	#define FDML_LINUX
#elif _WIN32
	#define FDML_WINDOWS
#else
//	#error failed to determine platform type
#endif

typedef CGAL::Exact_predicates_exact_constructions_kernel     Kernel;
typedef Kernel::Segment_2                                     Segment;
typedef Kernel::Point_2                                       Point;
typedef Kernel::Line_2                                        Line;
typedef Kernel::Direction_2                                   Direction;
typedef Kernel::Vector_2                                      Vector;

typedef std::vector<Point>                                    Point_2_container;
typedef CGAL::Polygon_2<Kernel, Point_2_container>            Polygon;
typedef CGAL::Polygon_with_holes_2<Kernel, Point_2_container> Polygon_with_holes;
typedef CGAL::General_polygon_with_holes_2<Polygon>           General_polygon_with_holes;
typedef CGAL::Polygon_set_2<Kernel, Point_2_container>        Polygon_set;

typedef CGAL::Gps_segment_traits_2<Kernel, Point_2_container> Gps_Traits;
typedef CGAL::General_polygon_set_2<Gps_Traits>               General_polygon_set_2;
typedef General_polygon_set_2::Arrangement_2                  Arrangement;
typedef Arrangement::Vertex_const_handle                      Vertex;
typedef Arrangement::Halfedge_const_handle                    Halfedge;
typedef Arrangement::Face_const_handle                        Face;

} // namespace FDML

#endif
