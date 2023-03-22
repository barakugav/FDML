// Copyright (c) 2023 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_EXPORT_FT_HPP
#define FDMLPY_EXPORT_FT_HPP

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>

#include "fdmlpy/to_string.hpp"
#include "fdmlpy/add_insertion.hpp"

namespace py = nanobind;

//
template <typename C>
void export_ft(C& c) {
  using FT = typename C::Type;

  c.def(py::init<const FT&>())
    .def(py::init_implicit<double>())
    .def(py::init_implicit<int>())
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def(py::self < py::self)
    .def(py::self > py::self)
    .def(py::self <= py::self)
    .def(py::self >= py::self)
    .def(py::self + py::self)
    .def(py::self += py::self)
    .def(py::self - py::self)
    .def(py::self -= py::self)
    .def(py::self * py::self)
    .def(py::self *= py::self)
    .def(py::self / py::self)
    .def(py::self /= py::self)
    .def(-py::self)
    ;

  add_insertion(c, "__str__");
  add_insertion(c, "__repr__");
}

#endif
