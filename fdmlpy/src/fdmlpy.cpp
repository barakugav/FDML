// Author(s): Efi Fogel         <efifogel@gmail.com>

#include <string>

#include <nanobind/nanobind.h>

#include "fdml/defs.hpp"
#include "fdml/locator.hpp"

#include "fdmlpy/export_ft.hpp"
#include "fdmlpy/export_point_2.hpp"
#include "fdmlpy/export_vector_2.hpp"
#include "fdmlpy/add_attr.hpp"

namespace py = nanobind;

extern void export_gmpz(py::module_&);
extern void export_gmpq(py::module_&);

void export_polygon_2(py::module_&);
void export_polygon_with_holes_2(py::module_&);

void export_kernel(py::module_& m) {
  using FT = FDML::Kernel::FT;
  using Pnt_2 = FDML::Kernel::Point_2;
  using Vec_2 = FDML::Kernel::Vector_2;

  if (! add_attr<CGAL::Gmpz>(m, "Gmpz")) export_gmpz(m);
  if (! add_attr<CGAL::Gmpq>(m, "Gmpq")) export_gmpq(m);

  if (! add_attr<FT>(m, "FT")) {
    using Fte = FT::Exact_type;
    using Fta = FT::Approximate_type;

    py::class_<FT> ft_c(m, "FT");
    export_ft(ft_c);
    ft_c.def(py::init<Fte>())
      .def("__init__", [](FT* self, const std::string& str)
                       { new (self) FT(Fte(str)); })
      .def("__init__", [](FT* self, int nom, int den)
                       { new (self) FT(Fte(nom, den)); })
      .def("to_double", [](const FT& ft)->double { return CGAL::to_double(ft); })
      .def("exact", [](const FT& ft)->const Fte& { return ft.exact();} )
      .def("approx", [](const FT& ft)->const Fta& { return ft.approx();} )
      ;
  }

  // Point_2
  if (! add_attr<Pnt_2>(m, "Point_2")) {
    py::class_<Pnt_2> pnt2_c(m, "Point_2");
    export_point_2<FDML::Kernel>(pnt2_c);

    if (! is_exact_ft()) {
      using Cci = FDML::Kernel::Cartesian_const_iterator_2;
      add_iterator<Cci, Cci>("Cartesian_iterator", pnt2_c);
      pnt2_c.def("cartesians",
                [] (const Pnt_2& p)
                { return make_iterator(p.cartesian_begin(), p.cartesian_end()); },
                py::keep_alive<0, 1>());
    }
  }

  // Vector_2
  if (! add_attr<Vec_2>(m, "Vector_2")) {
    py::class_<Vec_2> vec2_c(m, "Vector_2");
    export_vector_2<FDML::Kernel>(vec2_c);

    if (! is_exact_ft()) {
      using Cci = FDML::Kernel::Cartesian_const_iterator_2;
      add_iterator<Cci, Cci>("Cartesian_iterator", vec2_c);
      vec2_c.def("cartesians",
                 [] (const Vec_2& v)
                 { return make_iterator(v.cartesian_begin(), v.cartesian_end()); },
                 py::keep_alive<0, 1>());
    }
  }
}

py::list query1(const FDML::Locator& locator, const FDML::Kernel::FT& d) {
  std::vector<FDML::Locator::Res1d> pgns = locator.query(d);
  py::list lst;
  /* TODO return to python the measured edge along with the possible position polygon */
  for (auto pgn : pgns) {
    py::tuple edge = py::make_tuple(pgn.edge.first, pgn.edge.second);
    py::list res;
    res.append(pgn.pos);
    res.append(edge);
    lst.append(res);
  }
  return lst;
}

py::list query2(const FDML::Locator& locator, const FDML::Kernel::FT& d1, const FDML::Kernel::FT& d2) {
  std::vector<FDML::Locator::Res2d> pls = locator.query(d1, d2);
  py::list lst;
  /* TODO return to python the measured edge along with the possible position polygon */
  for (auto pl : pls) {
    py::tuple edge1 = py::make_tuple(pl.edge1.first, pl.edge1.second);
    py::tuple edge2 = py::make_tuple(pl.edge2.first, pl.edge2.second);
    py::list res;
    res.append(edge1);
    res.append(edge2);
    py::list polyline;
    for (const auto& e : pl.pos) polyline.append(e);
    res.append(polyline);
    lst.append(res);
  }
  return lst;
}

NB_MODULE(fdmlpy, m) {
  typedef FDML::Kernel::FT FT;
  typedef FDML::Polygon Polygon;
  typedef FDML::Segment Segment;
  typedef FDML::Locator Locator;

  auto ker_m = m.def_submodule("Ker");
  export_kernel(ker_m);

  auto pol2_m = m.def_submodule("Pol2");
  export_polygon_2(pol2_m);
  export_polygon_with_holes_2(pol2_m);

  // using Query1 = void(Locator::*)(const FT& d, std::vector<Polygon>& res) const;
  // using Query2 = void(Locator::*)(const FT& d1, const FT& d2, std::vector<Segment>& res) const;

  py::class_<FDML::Locator>(m, "Locator")
    .def(py::init<>())
    .def("init", &Locator::init)
    .def("query1", &query1)
    .def("query2", &query2)
    // .def<Query1>("query1", &Locator::query)
    // .def<Query2>("query2", &Locator::query)
    ;
}
