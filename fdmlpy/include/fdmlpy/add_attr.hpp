// Copyright (c) 2023 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_ADD_ATTR_HPP
#define FDMLPY_ADD_ATTR_HPP

#include <boost/static_assert.hpp>

#include <nanobind/nanobind.h>

namespace py = nanobind;

template <typename Type, typename PyClass>
bool add_attr(PyClass& cls, const char* name) {
  const py::handle info = py::type<Type>();
  if (! info.is_valid() || ! py::type_check(info)) return false;
  cls.attr(name) = info;
  return true;
}

#endif
