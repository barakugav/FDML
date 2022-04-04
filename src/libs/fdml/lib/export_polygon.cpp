// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <boost/python.hpp>

#include "fdml/defs.hpp"
#include "fdml/bindings_types.hpp"
#include "fdml/python_iterator_templates.hpp"

namespace bp = boost::python;

namespace pol2 {

FDML::Point& left_vertex(FDML::Polygon& P) { return *(P.left_vertex()); }
FDML::Point& right_vertex(FDML::Polygon& P) { return *(P.right_vertex()); }
FDML::Point& top_vertex(FDML::Polygon& P) { return *(P.top_vertex()); }
FDML::Point& bottom_vertex(FDML::Polygon& P) { return *(P.bottom_vertex()); }

static FDML::Polygon* init_from_list(bp::list& lst) {
  auto begin = bp::stl_input_iterator<FDML::Point>(lst);
  auto end = bp::stl_input_iterator<FDML::Point>();
  return new FDML::Polygon(begin, end);
}

CopyIterator<FDML::Polygon::Edge_const_iterator>*
edges_iterator(FDML::Polygon& P) {
  typedef FDML::Polygon::Edge_const_iterator    Edge_const_iterator;
  return new CopyIterator<Edge_const_iterator>(P.edges_begin(), P.edges_end());
}

}

void export_polygon_2() {
  typedef FDML::Polygon         Polygon;
  typedef FDML::Point           Point;

  bp::class_<Polygon>("Polygon_2")
    .def(bp::init<>())
    .def(bp::init<const Polygon&>())
    .def("__init__", make_constructor(&pol2::init_from_list))
    .def("push_back", &Polygon::push_back)
    .def("is_simple", &Polygon::is_simple)
    .def("is_convex", &Polygon::is_convex)
    .def("orientation", &Polygon::orientation)
    .def("oriented_side", &Polygon::oriented_side)
    .def("bounded_side", &Polygon::bounded_side)
    .def("is_empty", &Polygon::is_empty)
    .def("is_counterclockwise_oriented",
         &Polygon::is_counterclockwise_oriented)
    .def("is_clockwise_oriented", &Polygon::is_clockwise_oriented)
    .def("is_collinear_oriented", &Polygon::is_collinear_oriented)
    .def("has_on_positive_side", &Polygon::has_on_positive_side)
    .def("has_on_negative_side", &Polygon::has_on_negative_side)
    .def("has_on_boundary", &Polygon::has_on_boundary)
    .def("has_on_bounded_side", &Polygon::has_on_bounded_side)
    .def("has_on_unbounded_side", &Polygon::has_on_unbounded_side)
    .def("size", &Polygon::size)
    .def("area", &Polygon::area)
    .def("bbox", &Polygon::bbox)
    .def("vertices",
         bp::range<bp::return_internal_reference<>>(&Polygon::vertices_begin,
                                                    &Polygon::vertices_end))
    .def("edges", &pol2::edges_iterator, Manage_new_object())
    .def<const Point& (Polygon::*)(std::size_t) const>
      ("__getitem__", &Polygon::operator[], Copy_const_reference())
    .def("left_vertex", &pol2::left_vertex, bp::return_internal_reference<>())
    .def("right_vertex", &pol2::right_vertex, bp::return_internal_reference<>())
    .def("top_vertex", &pol2::top_vertex, bp::return_internal_reference<>())
    .def("bottom_vertex", &pol2::bottom_vertex, bp::return_internal_reference<>())
    .def<const Point& (Polygon::*)(std::size_t) const>
      ("vertex", &Polygon::vertex, Copy_const_reference())
    .def("edge", &Polygon::edge)
    .def("clear", &Polygon::clear)
    .def("reverse_orientation", &Polygon::reverse_orientation)
    .def(bp::self_ns::str(bp::self_ns::self))
    .def(bp::self_ns::repr(bp::self_ns::self))
    .def(bp::self == bp::self)
    .def(bp::self != bp::self)
    ;

  bind_copy_iterator<CopyIterator<Polygon::Edge_const_iterator>>("Polygon_edges_iterator");
}
