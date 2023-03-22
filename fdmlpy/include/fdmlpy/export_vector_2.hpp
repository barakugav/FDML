
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_EXPORT_VECTOR_2_HPP
#define FDMLPY_EXPORT_VECTOR_2_HPP

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>

#include "fdmlpy/add_insertion.hpp"
#include "fdmlpy/make_iterator.hpp"

namespace bp = nanobind;

// Export a two-dimensional vector of a kernel.
template <typename Kernel, typename C>
void export_vector_2(C& c) {
  using FT = typename Kernel::FT;
  using RT = typename Kernel::RT;
  using Pnt = typename Kernel::Point_2;
  using Vec = typename Kernel::Vector_2;
  using Line = typename Kernel::Line_2;
  using Ray = typename Kernel::Ray_2;
  using Seg = typename Kernel::Segment_2;

  c.def(bp::init<Pnt&, Pnt&>())
    .def(bp::init<Line>())
    .def(bp::init<Ray>())
    .def(bp::init<Seg>())
    .def(bp::init<FT&, FT&, FT&>())
    .def(bp::init<FT&, FT&>())
    .def(bp::init<double, double>())
    .def("hx", &Vec::hx)
    .def("hy", &Vec::hy)
    .def("hw", &Vec::hw)
    .def("x", &Vec::x)
    .def("y", &Vec::y)
    .def("squared_length", &Vec::squared_length)
    .def("homogeneous", &Vec::homogeneous)
    .def("cartesian", &Vec::cartesian)
    .def("__getitem__", &Vec::operator[])
    .def("dimension", &Vec::dimension)
    .def("direction", &Vec::direction)
    .def("transform", &Vec::transform)
    .def("perpendicular", &Vec::perpendicular)
    .def(bp::self == bp::self)
    .def(bp::self != bp::self)
    .def(bp::self != bp::self)
    .def(bp::self + bp::self)
    .def(bp::self += bp::self)
    .def(bp::self - bp::self)
    .def(bp::self -= bp::self)
    .def(-bp::self)
    .def(bp::self * bp::self)
    .def(bp::self * FT())
    .def(FT() * bp::self)
    .def(bp::self *= FT())
    .def(bp::self / FT())
    .def(bp::self /= FT())
    //.def(py::self * RT())
    //.def(RT() * bp::self)
    //.def(bp::self *= RT())
    //.def(bp::self / RT())
    //.def(bp::self /= RT())
    //.setattr("__hash__", &hash<Vec>)
    ;

  add_insertion(c, "__str__");
  add_insertion(c, "__repr__");
}

#endif
