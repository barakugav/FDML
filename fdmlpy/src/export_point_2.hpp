// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_EXPORT_POINT_2_HPP
#define FDMLPY_EXPORT_POINT_2_HPP

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>

#include "add_insertion.hpp"
#include "make_iterator.hpp"
#include "add_extraction.hpp"
#include "Hash_rational_point.hpp"

namespace py = nanobind;

#define CGALPY_KERNEL_EPIC                                  0
#define CGALPY_KERNEL_EPEC                                  1
#define CGALPY_KERNEL_EPEC_WITH_SQRT                        2
#define CGALPY_KERNEL_FILTERED_SIMPLE_CARTESIAN_DOUBLE      3
#define CGALPY_KERNEL_FILTERED_SIMPLE_CARTESIAN_LAZY_GMPQ   4

#ifndef CGALPY_KERNEL
#define CGALPY_KERNEL 1
#endif

// Determine whether the dD kernel is an an EPEC type.
// An EPEC type has a non trivial FT
constexpr bool is_exact_ft() {
  return ((CGALPY_KERNEL == CGALPY_KERNEL_EPEC) ||                      \
          (CGALPY_KERNEL == CGALPY_KERNEL_EPEC_WITH_SQRT) ||            \
          (CGALPY_KERNEL == CGALPY_KERNEL_FILTERED_SIMPLE_CARTESIAN_LAZY_GMPQ));
}

// Export a two-dimensional point of a kernel.
template <typename Kernel, typename C>
void export_point_2(C& c) {
  using FT = typename Kernel::FT;
  using RT = typename Kernel::RT;
  using Pnt = typename Kernel::Point_2;
  using Vec = typename Kernel::Vector_2;

  c.def(py::init<>())
    .def(py::init<Pnt&>())
    .def(py::init<double, double>())
    .def(py::init<double, FT>())
    .def(py::init<FT, double>())
    .def(py::init<FT&, FT&>())
    .def(py::init<RT&, RT&>())
    .def("x", &Pnt::x)
    .def("y", &Pnt::y)
    .def("hx", &Pnt::hx)
    .def("hy", &Pnt::hy)
    .def("hw", &Pnt::hw)
    .def("bbox", &Pnt::bbox)
    .def("cartesian", &Pnt::cartesian)
    .def("__getitem__", &Pnt::operator[])
    .def("dimension", &Pnt::dimension)
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def(py::self > py::self)
    .def(py::self < py::self)
    .def(py::self <= py::self)
    .def(py::self >= py::self)
    .def(py::self - py::self)
    .def(py::self += Vec())
    .def(py::self -= Vec())
    .def(py::self + Vec())
    .def(py::self - Vec())
    .def("__hash__", &hash_rational_point<is_exact_ft(), Pnt>)
    // .setattr("__doc__", "Point_2") NB
    ;

  add_insertion(c, "__str__");
  add_insertion(c, "__repr__");
  add_extraction(c);
}

#endif
