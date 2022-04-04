// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDML_PYTHON_ITERATOR_TEMPLATES_HPP
#define FDML_PYTHON_ITERATOR_TEMPLATES_HPP

#include <boost/python.hpp>

namespace bp = boost::python;

inline bp::object pass_through(bp::object const& o) { return o; }

//these template classes are used to allow more natural iteration in python

template <typename circulator>
class Iterator_from_circulator {
private:
  bool first = true;
  circulator m_first;
  circulator m_curr;

public:
  Iterator_from_circulator(circulator first) : m_first(first), m_curr(first) {}
  typename circulator::value_type& next() {
    if (m_curr != 0) {
      if (first || m_curr != m_first) {
        first = false;
        return *(m_curr++);
      }
    }
    PyErr_SetString(PyExc_StopIteration, "No more data.");
    bp::throw_error_already_set();
    return *m_curr;
  }
};

template <typename iterator>
class Iterator_of_circulators {
  typedef Iterator_from_circulator<typename iterator::value_type>
    modified_circulator;
  iterator m_curr;
  iterator m_end;

public:
  Iterator_of_circulators(iterator begin, iterator end) :
    m_curr(begin),
    m_end(end)
  {}

  modified_circulator* next() {
    if (m_curr != m_end)
      return new modified_circulator(modified_circulator(*(m_curr++)));
    PyErr_SetString(PyExc_StopIteration, "No more data.");
    bp::throw_error_already_set();
    return new modified_circulator(modified_circulator(*m_curr));
  }
};

template<typename iterator>
void bind_iterator_of_circulators(const char* python_name) {
  bp::class_<iterator>(python_name, bp::no_init)
    .def("__iter__", &pass_through)
    .def("__next__", &iterator::next,
         bp::return_value_policy<bp::manage_new_object>())
    ;
}

template<typename iterator>
void bind_iterator(const char* python_name) {
  bp::class_<iterator>(python_name, bp::no_init)
    .def("__iter__", &pass_through)
    .def("__next__", &iterator::next, bp::return_value_policy<bp::reference_existing_object>())
    ;
}


// For iterators that don't dereference to a reference of an existing object
template <typename iterator>
class CopyIterator {
private:
  iterator m_curr;
  iterator m_end;
public:
  CopyIterator(iterator begin, iterator end) : m_curr(begin), m_end(end) {}
  typename iterator::value_type next() {
    if (m_curr != m_end) return *(m_curr++);
    PyErr_SetString(PyExc_StopIteration, "No more data.");
    bp::throw_error_already_set();
    return *m_curr;
  }
};

template <typename circulator>
class Copy_iterator_from_circulator {
private:
  bool first = true;
  circulator m_first;
  circulator m_curr;

public:
  Copy_iterator_from_circulator(circulator first) :
    m_first(first),
    m_curr(first)
  {}

  typename circulator::value_type next() {
    if (m_curr != 0) {
      if (first || m_curr != m_first) {
        first = false;
        return *(m_curr++);
      }
    }
    PyErr_SetString(PyExc_StopIteration, "No more data.");
    bp::throw_error_already_set();
    return *m_curr;
  }
};

template<typename iterator>
void bind_copy_iterator(const char* python_name) {
  bp::class_<iterator>(python_name, bp::no_init)
    .def("__iter__", &pass_through)
    .def("__next__", &iterator::next)
    ;
}
#endif
