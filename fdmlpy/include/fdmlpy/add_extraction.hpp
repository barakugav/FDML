
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_ADD_EXTRACTION_HPP
#define FDMLPY_ADD_EXTRACTION_HPP

#include <sstream>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

template <typename PyClass>
void add_extraction(PyClass& cls) {
  cls.def("__init__", [](typename PyClass::Type* self,
                         const std::string& str) {
                        std::istringstream is(str);
                        new (self) typename PyClass::Type();    // placement new
                        is >> *self;
                      });
}
#endif
