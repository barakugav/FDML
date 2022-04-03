// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "fdml/locator.hpp"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(fdml) {
  bp::class_<FDML::Locator>("Locator", bp::init<>())
    // .def("query1",
    //      static_cast<void(Locator::*)(const Kernel::FT& d,
    //                                   std::vector<Polygon>& res)>(&Locator::query))
    ;
}
