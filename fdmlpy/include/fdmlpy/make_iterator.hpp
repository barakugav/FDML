// Copyright (c) 2019 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_MAKE_ITERATOR_HPP
#define FDMLPY_MAKE_ITERATOR_HPP

#include <nanobind/nanobind.h>

#include "fdmlpy/add_attr.hpp"

namespace py = nanobind;

//
template <typename Iterator, typename Sentinel>
struct iterator_state {
  Iterator it;
  Sentinel end;
  bool first_or_done;
};

template <py::rv_policy Policy,
          typename Iterator, typename Sentinel, typename ValueType,
          typename... Extra,
          typename C>
void add_iterator_impl(const char* name, C& c, Extra&&... extra) {
  using state = iterator_state<Iterator, Sentinel>;
  if (add_attr<state>(c, name)) return;

  py::class_<state>(c, name)
    .def("__iter__", [](state& s)->state& { return s; })
    .def("__next__", [](state& s) -> ValueType {
                       if (! s.first_or_done) ++s.it;
                       else s.first_or_done = false;
                       if (s.it == s.end) {
                         s.first_or_done = true;
                         throw py::stop_iteration();
                       }
                       return *s.it;
                     },
      std::forward<Extra>(extra)..., Policy)
    ;
}

// Add (wrap) an iterator
template <typename Iterator, typename Sentinel,
          typename ValueType = decltype(*std::declval<Iterator>()),
          py::rv_policy Policy = py::rv_policy::reference_internal,
          typename... Extra,
          typename C>
void add_iterator(const char* name, C& c, Extra&&... extra) {
  add_iterator_impl<Policy,
                    Iterator, Sentinel, ValueType,
                    Extra...>(name, c, std::forward<Extra>(extra)...);
}

// Obtain a Python iterator
template <typename Iterator, typename Sentinel>
py::object make_iterator(Iterator begin, Sentinel end) {
  using state = iterator_state<Iterator, Sentinel>;
  return py::cast(state{begin, end, true});
}

#endif
