// Copyright (c) 2023 Israel.
// All rights reserved to Tel Aviv University.
//
// SPDX-License-Identifier: LGPL-3.0-or-later.
// Commercial use is authorized only through a concession contract to purchase a commercial license for CGAL.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_HASH_RATIONAL_POINT_HPP
#define FDMLPY_HASH_RATIONAL_POINT_HPP

#include <boost/functional/hash_fwd.hpp>
#include <boost/functional/hash/hash.hpp>

template <bool b>
struct Hash_rational_point {};

// exact_impl() dispatches between the case where the type of the number
// passed has the exact() method. The second parameter is used to resolve
// overload ambiguity in case the exact() method does exist. In this case
// 'int' is preferred over ellipsis (...).

// Fall through; T::exact() does not exist
template <typename T> const T& exact_impl(const T& val, ...) { return val; }

// T::exact() exists
template <typename T, typename = decltype(std::declval<const T&>().exact())>
decltype(std::declval<const T&>().exact())
exact_impl(const T& val, int) { return val.exact(); }

//
template <>
struct Hash_rational_point<true> {
  template <typename Point>
  static size_t hash_rational_point(Point& p) {
    size_t seed = 0;
    for (auto c = p.cartesian_begin(); c != p.cartesian_end(); ++c) {
      auto q = exact_impl(*c, 0);
      auto simplify =
        typename CGAL::Algebraic_structure_traits<decltype(q)>::Simplify();
      CGAL::Rational_traits<decltype(q)> traits;
      simplify(q);
      boost::hash_combine(seed, CGAL::to_double(traits.numerator(q)));
      boost::hash_combine(seed, CGAL::to_double(traits.denominator(q)));
    }
    return seed;
  }
};

template <>
struct Hash_rational_point<false> {
  template <typename Point>
  static size_t hash_rational_point(Point& p)
  { return boost::hash_range(p.cartesian_begin(), p.cartesian_end()); }
};

template<bool b, typename Point>
size_t hash_rational_point(Point& p) {
  return Hash_rational_point<b>::hash_rational_point(p);
};

// #if (CGALPY_KERNEL == CGALPY_KERNEL_EPEC) || \
//   (CGALPY_KERNEL == CGALPY_KERNEL_EPEC_WITH_SQRT) || \
//   (CGALPY_KERNEL == CGALPY_KERNEL_FILTERED_SIMPLE_CARTESIAN_LAZY_GMPQ) || \
//   (CGALPY_KERNEL_D == CGALPY_KERNEL_D_EPEC_D) || \
//   (CGALPY_KERNEL_D == CGALPY_KERNEL_D_CARTESIAN_D_LAZY_GMPQ)
// template <typename Point>
// size_t hash_rational_point(Point& p) {
//   size_t seed = 0;
//   for (auto c = p.cartesian_begin(); c != p.cartesian_end(); ++c) {
//     auto q = (*c).exact();
//     auto simplify =
//       typename CGAL::Algebraic_structure_traits<decltype(q)>::Simplify();
//     CGAL::Rational_traits<decltype(q)> traits;
//     simplify(q);
//     boost::hash_combine(seed, CGAL::to_double(traits.numerator(q)));
//     boost::hash_combine(seed, CGAL::to_double(traits.denominator(q)));
//   }
//   return seed;
// }
// #endif

// #if (CGALPY_KERNEL == CGALPY_KERNEL_EPIC) || \
//   (CGALPY_KERNEL == CGALPY_KERNEL_FILTERED_SIMPLE_CARTESIAN_DOUBLE) || \
//   (CGALPY_KERNEL_D == CGALPY_KERNEL_D_EPIC_D) || \
//   (CGALPY_KERNEL_D == CGALPY_KERNEL_D_CARTESIAN_D_DOUBLE)
// template <typename Point>
// size_t hash_rational_point(Point& p)
// { return boost::hash_range(p.cartesian_begin(), p.cartesian_end()); }
// #endif

#endif
