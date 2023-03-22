// Author(s): Efi Fogel         <efifogel@gmail.com>

#include <string>

#include <CGAL/Gmpz.h>
#include <CGAL/Gmpq.h>

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>

#include "fdmlpy/to_string.hpp"

namespace py = nanobind;

void export_gmpz(py::module_& m) {
  py::class_<CGAL::Gmpz>(m, "Gmpz")
    .def(py::init_implicit<int>())
    .def(py::init_implicit<double>())
    .def(py::init_implicit<const std::string&>())
    .def(py::init<CGAL::Gmpz&>())
    .def("to_double", &CGAL::Gmpz::to_double)
    .def("__str__", to_string<CGAL::Gmpz>)
    .def("__repr__", to_string<CGAL::Gmpz>)
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
}

void export_gmpq(py::module_& m) {
  py::class_<CGAL::Gmpq>(m, "Gmpq")
    .def(py::init_implicit<int>())
    .def(py::init_implicit<double>())
    .def(py::init_implicit<CGAL::Gmpz>())
    .def(py::init<CGAL::Gmpz, CGAL::Gmpz>())
    .def(py::init<const std::string&>())
    .def(py::init<CGAL::Gmpq&>())
    .def("to_double", &CGAL::Gmpq::to_double)
    .def("numerator", &CGAL::Gmpq::numerator)
    .def("denominator", &CGAL::Gmpq::denominator)
    .def("size", &CGAL::Gmpq::size)
    .def("__str__", to_string<CGAL::Gmpq>)
    .def("__repr__", to_string<CGAL::Gmpq>)
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
}
