// Copyright (c) 2023 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_STL_INPUT_ITERATOR_HPP
#define FDMLPY_STL_INPUT_ITERATOR_HPP

#include <boost/iterator/iterator_facade.hpp>

#include <nanobind/nanobind.h>

namespace py = nanobind;

template <typename T>
struct stl_input_iterator :
  boost::iterator_facade<stl_input_iterator<T>, T, std::forward_iterator_tag, T> {

  // Default constructor.
  // Workaround the lack of default constructor for py::detail::fast_iterator.
  // stl_input_iterator() {}
  stl_input_iterator() : m_it(py::list().end()) {}

  stl_input_iterator(const py::list& lst, bool isbegin = true) :
    m_it((isbegin) ? lst.begin() : lst.end())
  {}

  void increment() { ++m_it; }
  T dereference() const { return py::cast<T>(*m_it); }

  bool equal(stl_input_iterator<T> const& o) const { return m_it == o.m_it; }

private:
  py::detail::fast_iterator m_it;
};

#endif
