// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#include <string>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "fdml/defs.hpp"
#include "fdml/export_ft.hpp"
#include "fdml/locator.hpp"

namespace bp = boost::python;

#define STR(s) #s
#define XSTR(s) STR(s)

#define SET_SCOPE(x)  \
std::string module_name = std::string(XSTR(fdml))+std::string(".")+std::string(x); \
bp::object module(bp::handle<>(bp::borrowed(PyImport_AddModule(module_name.c_str())))); \
bp::scope().attr(x) = module; \
bp::scope module_scope = module;

void export_polygon_2();
void export_polygon_with_holes_2();
void export_point_2();
void export_vector_2();

void export_kernel() {
  typedef FDML::Kernel::FT      FT;
  auto ftc = bp::class_<FT>("FT");
  export_ft<FT>(ftc);
}

bp::list query1(const FDML::Locator& locator, const FDML::Kernel::FT& d) {
  std::vector<FDML::Locator::Res1d> pgns = locator.query(d);
  bp::list lst;
  /* TODO return to python the measured edge along with the possible position polygon */
  for (auto pgn : pgns) lst.append(pgn.pos);
  return lst;
}

BOOST_PYTHON_MODULE(fdml) {
  typedef FDML::Kernel::FT      FT;
  typedef FDML::Polygon         Polygon;
  typedef FDML::Segment         Segment;
  typedef FDML::Locator         Locator;

  {
    SET_SCOPE("ker")
    export_kernel();
    export_point_2();
    export_vector_2();
  }

  {
    SET_SCOPE("pol2")
    export_polygon_2();
    export_polygon_with_holes_2();
  };

  // using Query1 = void(Locator::*)(const FT& d, std::vector<Polygon>& res) const;
  // using Query2 = void(Locator::*)(const FT& d1, const FT& d2, std::vector<Segment>& res) const;

  bp::class_<FDML::Locator>("Locator", bp::init<>())
    .def("init", &Locator::init)
    .def("query1", &query1)
    // .def<Query1>("query1", &Locator::query)
    // .def<Query2>("query2", &Locator::query)
    ;
}
