
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
