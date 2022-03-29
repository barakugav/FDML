#ifndef FDML_DEFS_HPP
#define FDML_DEFS_HPP

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Polygon_2.h>

namespace FDML {

#ifdef __linux__
	#define FDML_LINUX
#elif _WIN32
	#define FDML_WINDOWS
#else
//	#error failed to determine platform type
#endif

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Segment_2<Kernel>                 Segment;
typedef Kernel::Point_2                         Point;
typedef Kernel::Line_2                          Line;
typedef Kernel::Direction_2                     Direction;
typedef Kernel::Vector_2                        Vector;
typedef CGAL::Arr_segment_traits_2<Kernel>      Traits;
typedef CGAL::Arrangement_2<Traits>             Arrangement;
typedef CGAL::Polygon_2<Kernel>                 Polygon;
typedef Arrangement::Vertex_const_handle        Vertex;
typedef Arrangement::Halfedge_const_handle      Halfedge;
typedef Arrangement::Face_const_handle          Face;

} // namespace FDML

#endif
