#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/General_polygon_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Segment_2<Kernel> Segment;
typedef Kernel::Point_2 Point;
typedef Kernel::Line_2 Line;
typedef Kernel::Direction_2 Direction;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits;
typedef CGAL::Arrangement_2<Traits> Arrangement;
typedef CGAL::General_polygon_2<Traits> Polygon;

typedef Arrangement::Vertex_const_handle Vertex_const_handle;
typedef Arrangement::Halfedge_const_handle Halfedge_const_handle;
typedef Arrangement::Face_const_handle Face_const_handle;
