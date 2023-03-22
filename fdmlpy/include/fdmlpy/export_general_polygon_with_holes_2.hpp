// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_EXPORT_GENERAL_POLYGON_WITH_HOLES_2_HPP
#define FDMLPY_EXPORT_GENERAL_POLYGON_WITH_HOLES_2_HPP

#include <type_traits>

#include <nanobind/nanobind.h>

#include "fdmlpy/add_insertion.hpp"
#include "fdmlpy/stl_input_iterator.hpp"
#include "fdmlpy/make_iterator.hpp"
#include "fdmlpy/add_extraction.hpp"

namespace py = nanobind;

// Initialize a general polygon with hole from the outer boundary and a list
// of holes.
template <typename GeneralPolygonWithHoles_2>
void init_polygon_with_holes_2(GeneralPolygonWithHoles_2* pwh,
                               typename
                               GeneralPolygonWithHoles_2::General_polygon_2& p,
                               py::list& lst) {
  using Gpwh = GeneralPolygonWithHoles_2;
  using Gpgn = typename Gpwh::General_polygon_2;
  auto begin = stl_input_iterator<Gpgn>(lst);
  auto end = stl_input_iterator<Gpgn>(lst, false);
  new (pwh) Gpwh(p, begin, end);        // placement new
}

// Export the attributes of General_polygon_with_holes_2.
template <typename Type>
void export_general_polygon_with_holes_2(py::class_<Type>& pwh_c) {
  using Gpwh = Type;
  using Gpgn = typename Gpwh::General_polygon_2;

  pwh_c.def(py::init<Gpwh&>())
    .def(py::init<Gpgn&>())
    .def("__init__", &init_polygon_with_holes_2<Gpwh>)
    .def("is_unbounded", &Gpwh::is_unbounded)

    // Use `py::overload_cast` to cast overloaded functions.
    // 1. As a convention, add the suffix `_mutable` to the mutable version.
    // 2. Wrap the mutable method with the `reference_internal` call policy.
    // 3. Add the `const_` tag to the overloaded const function, as the
    //    overloading is based on constness.
    .def("outer_boundary_mutable", py::overload_cast<>(&Gpwh::outer_boundary),
         py::rv_policy::reference_internal)
    .def("outer_boundary",
         py::overload_cast<>(&Gpwh::outer_boundary, py::const_))
    .def("add_hole", py::overload_cast<const Gpgn&>(&Gpwh::add_hole))
    .def("erase_hole", &Gpwh::erase_hole)
    .def("has_holes", &Gpwh::has_holes)
    .def("number_of_holes", &Gpwh::number_of_holes)
    .def("clear", &Gpwh::clear)
    .def("is_plane", &Gpwh::is_plane)
    ;

  using Hci = typename Gpwh::Hole_const_iterator;
  add_iterator<Hci, Hci>("Hole_iterator", pwh_c);
  pwh_c.def("holes",
            [] (const Gpwh& pwh)
            { return make_iterator(pwh.holes_begin(), pwh.holes_end()); },
            py::keep_alive<0, 1>());

  add_insertion(pwh_c, "__str__");
  add_insertion(pwh_c, "__repr__");
  // Compile in only if we use CGAL version > 5.6.0
  // There are geometry traits that do not support the extraction of a curve
  // to an output stream. In particular, the Arr_circle_segment_traits_2.
  // A PR for Arr_circle_segment_traits_2 is on the way.
  // The reamining unsupported traits should be compiled out.
#if CGAL_VERSION_NR > 1050600900
  add_extraction(pwh_c);
#endif
}

/*! Capture the call to export a Polygon_with_holes_2<> and ensure that it
 * is not invoked.
 */
template <typename Kernel, typename Container>
inline
void export_general_polygon_with_holes_2
(py::class_<CGAL::Polygon_with_holes_2<Kernel, Container>>& /* pwh_c */) {
  throw std::runtime_error("Attempting to export Polygon_with_holes_2 as General_polygon_with_holes_2!");
}

#endif
