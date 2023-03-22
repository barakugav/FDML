// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDML_EXPORT_FT_HPP
#define FDML_EXPORT_FT_HPP

#include <boost/python.hpp>

#include <CGAL/basic.h>

namespace bp = boost::python;

template <typename FT_> const typename FT_::Exact_type& ft_exact(const FT_& ft) {
    return ft.exact();
}

template <typename FT_> const typename FT_::Approximate_type& ft_approx(const FT_& ft) {
    return ft.approx();
}

template <typename FT_> double ft_to_double(const FT_& ft) {
    return CGAL::to_double(ft);
}

template <typename FT_, typename C> void export_ft(C c) {
    c.def(bp::init<double>())
        .def(bp::init<typename FT_::Exact_type>())
        .def(bp::init<FT_>())
        .def("exact", &ft_exact<FT_>, bp::return_internal_reference<>())
        //.def("approx", &ft__approx<FT_>, bp::return_internal_reference<>())
        .def("to_double", &ft_to_double<FT_>)
        .def(bp::self_ns::str(bp::self_ns::self))
        .def(bp::self_ns::repr(bp::self_ns::self))
        .def(bp::self == bp::self)
        .def(bp::self != bp::self)
        .def(bp::self < bp::self)
        .def(bp::self > bp::self)
        .def(bp::self <= bp::self)
        .def(bp::self >= bp::self)
        .def(bp::self + bp::self)
        .def(bp::self += bp::self)
        .def(bp::self - bp::self)
        .def(bp::self -= bp::self)
        .def(bp::self * bp::self)
        .def(bp::self *= bp::self)
        .def(bp::self / bp::self)
        .def(bp::self /= bp::self)
        .def(-bp::self);
}

#endif
