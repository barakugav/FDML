// Copyright (c) 2023 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>

#include "fdml/defs.hpp"

#include "fdmlpy/python_iterator_templates.hpp"
#include "fdmlpy/add_insertion.hpp"
#include "fdmlpy/stl_input_iterator.hpp"
#include "fdmlpy/add_attr.hpp"
#include "fdmlpy/export_general_polygon_with_holes_2.hpp"
#include "fdmlpy/add_extraction.hpp"

namespace py = nanobind;

namespace pol2 {

// Initialize a polygon with holes from an outer boundary and a list of holes.
void init_polygon_with_holes_2(FDML::Polygon_with_holes* pwh,
                               FDML::Polygon& p, py::list& lst) {
  auto begin = stl_input_iterator<FDML::Polygon>(lst);
  auto end = stl_input_iterator<FDML::Polygon>(lst, false);
  new (pwh) FDML::Polygon_with_holes(p, begin, end);        // placement new
}

}

/*! Export `CGAL::Polygon_with_holes<>`, which derives from
 * `CGAL::General_polygon_with_holes_2<>`
 */
void export_polygon_with_holes_2(py::module_& m) {
  using Pgn = FDML::Polygon;
  using Pwh = FDML::Polygon_with_holes;
  using Gpwh = FDML::General_polygon_with_holes;

  if (! add_attr<Gpwh>(m, "General_polygon_with_holes_2")) {
    py::class_<Gpwh> gpwh_c(m, "General_polygon_with_holes_2");
    export_general_polygon_with_holes_2(gpwh_c);
  }

  if (! add_attr<Pwh>(m, "Polygon_with_holes_2")) {
    py::class_<Pwh, Gpwh> pwh_c(m, "Polygon_with_holes_2");
    pwh_c.def(py::init<>())
      .def(py::init<Pgn&>())
      .def("__init__", &pol2::init_polygon_with_holes_2)

      .def("bbox", &Pwh::bbox)
      .def(py::self == py::self)
      .def(py::self != py::self)
      ;

    add_insertion(pwh_c, "__str__");
    add_insertion(pwh_c, "__repr__");
    add_extraction(pwh_c);
  }
}
