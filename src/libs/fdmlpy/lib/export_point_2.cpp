// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <boost/python.hpp>

#include "fdml/defs.hpp"
#include "fdml/bindings_types.hpp"
#include "fdml/Hash_rational_point.hpp"

namespace bp = boost::python;

constexpr bool is_epec_type() {
  return true;
}

void export_point_2() {
  typedef FDML::Kernel::FT              FT;
  typedef FDML::Kernel::RT              RT;
  typedef FDML::Kernel::Point_2         Point_2;
  typedef FDML::Kernel::Vector_2        Vector_2;

  // const bp::type_info info = bp::type_id<Point_2>();
  // const auto* reg = bp::converter::registry::query(info);
  // if ((reg != nullptr) && ((*reg).m_to_python != nullptr)) {
  //   bp::scope().attr("Point_2") = bp::handle<>(reg->m_class_object);
  //   return;
  // }

  bp::class_<Point_2>("Point_2")
    .def(bp::init<>())
    .def(bp::init<double, double>())
    .def(bp::init<FT&, FT&>())
    .def(bp::init<RT&, RT&>())
    .def(bp::init<Point_2&>())
    .def("x", &Point_2::x, Kernel_return_value_policy())
    .def("y", &Point_2::y, Kernel_return_value_policy())
    .def("hx", &Point_2::hx, Kernel_return_value_policy())
    .def("hy", &Point_2::hy, Kernel_return_value_policy())
    .def("hw", &Point_2::hw, Kernel_return_value_policy())
    .def("bbox", &Point_2::bbox)
    .def("cartesian", &Point_2::cartesian, Kernel_return_value_policy())
    .def("__getitem__", &Point_2::operator[], Kernel_return_value_policy())
// #if ((CGALPY_KERNEL == CGALPY_KERNEL_EPIC) ||                           \
//      (CGALPY_KERNEL == CGALPY_KERNEL_FILTERED_SIMPLE_CARTESIAN_DOUBLE))
//     // TODO: Returning address of local variable or temporary with EPEC kernel
    .def("coordinates", bp::range<>(&Point_2::cartesian_begin, &Point_2::cartesian_end))
// #endif
    .def("dimension", &Point_2::dimension)
    .def(bp::self_ns::str(bp::self_ns::self))
    .def(bp::self_ns::repr(bp::self_ns::self))
    .def(bp::self == bp::self)
    .def(bp::self != bp::self)
    .def(bp::self > bp::self)
    .def(bp::self < bp::self)
    .def(bp::self <= bp::self)
    .def(bp::self >= bp::self)
    .def(bp::self - bp::self)
    // .def(bp::self += Vector_2())
    // .def(bp::self -= Vector_2())
    // .def(bp::self + Vector_2())
    // .def(bp::self - Vector_2())
    .setattr("__hash__", &hash_rational_point<is_epec_type(), Point_2>)
    .setattr("__doc__", "Point_2")
    ;
}
