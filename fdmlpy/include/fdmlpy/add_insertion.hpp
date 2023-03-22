
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_ADD_INSERTION_HPP
#define FDMLPY_ADD_INSERTION_HPP

#include <sstream>

#include <CGAL/basic.h>
// #include <CGAL/IO/io.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

template <typename PyClass, bool pretty = false>
void add_insertion(PyClass& cls, const char* method) {
  cls.def(method, [](typename PyClass::Type const& self) {
                    std::ostringstream os;
                    if (pretty) CGAL::IO::set_pretty_mode(os);
                    os << self;
                    return os.str();
                  });
}

#endif
