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

FDML::Polygon_with_holes*
init_polygon_with_holes_2(FDML::Polygon& p, bp::list& lst) {
  auto begin = bp::stl_input_iterator<FDML::Polygon>(lst);
  auto end = bp::stl_input_iterator<FDML::Polygon>();
  return new FDML::Polygon_with_holes(p, begin, end);
}

FDML::Polygon_with_holes::Hole_const_iterator
holes_begin(FDML::Polygon_with_holes& p)
{ return p.holes_begin(); }

FDML::Polygon_with_holes::Hole_const_iterator
holes_end(FDML::Polygon_with_holes& p)
{ return p.holes_end(); }

FDML::Polygon& outer_boundary(FDML::Polygon_with_holes& p)
{ return p.outer_boundary(); }

}

void export_polygon_with_holes_2() {
  typedef FDML::Polygon               Polygon;
  typedef FDML::Polygon_with_holes    Polygon_with_holes;

  bp::class_<Polygon_with_holes>("Polygon_with_holes_2")
    .def(bp::init<Polygon&>())
    .def("__init__", make_constructor(&pol2::init_polygon_with_holes_2))
    .def("is_unbounded", &Polygon_with_holes::is_unbounded)
    .def("outer_boundary", &pol2::outer_boundary,
         bp::return_internal_reference<>())
    .def("holes", bp::range<bp::return_internal_reference<>>(&pol2::holes_begin,
                                                             &pol2::holes_end))
    .def("number_of_holes", &Polygon_with_holes::number_of_holes)
    .def("bbox", &Polygon_with_holes::bbox)
    .def("add_hole", &Polygon_with_holes::add_hole)
    .def(bp::self_ns::str(bp::self_ns::self))
    .def(bp::self_ns::repr(bp::self_ns::self))
    .def(bp::self == bp::self)
    .def(bp::self != bp::self)
    ;
}
