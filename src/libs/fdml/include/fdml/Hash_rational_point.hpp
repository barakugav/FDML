// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDML_HASH_RATIONAL_POINT_HPP
#define FDML_HASH_RATIONAL_POINT_HPP

#include <boost/functional/hash_fwd.hpp>
#include <boost/functional/hash/hash.hpp>

template <bool b>
struct Hash_rational_point {};

template <>
struct Hash_rational_point<true> {
  template <typename Point>
  static size_t hash_rational_point(Point& p) {
    size_t seed = 0;
    for (auto c = p.cartesian_begin(); c != p.cartesian_end(); ++c) {
      auto q = (*c).exact();
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

#endif
