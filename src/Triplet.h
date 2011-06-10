/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef D_TRIPLET_H
#define D_TRIPLET_H

#include <cstdlib>
#include <utility>

namespace aria2 {

template<typename T1, typename T2, typename T3>
struct Triplet {
  typedef T1 first_type;
  typedef T2 second_type;
  typedef T3 third_type;

  T1 first;
  T2 second;
  T3 third;

  Triplet() {}

  Triplet(const T1& t1, const T2& t2, const T3& t3):
    first(t1), second(t2), third(t3) {}

  template<typename U1, typename U2, typename U3>
  Triplet(const Triplet<U1, U2, U3>& t):
    first(t.first), second(t.second), third(t.third) {}

  Triplet& operator=(const Triplet& tri)
  {
    if(this != &tri) {
      first = tri.first;
      second = tri.second;
      third = tri.third;
    }
    return *this;
  }
};

template<typename T1, typename T2, typename T3>
bool operator<(const Triplet<T1, T2, T3>& lhs, const Triplet<T1, T2, T3>& rhs)
{
  return lhs.first < rhs.first ||
    (!(rhs.first < lhs.first) && (lhs.second < rhs.second ||
                                  (!(rhs.second < lhs.second) &&
                                   lhs.third < rhs.third)));
}

template<typename T1, typename T2, typename T3>
Triplet<T1, T2, T3> makeTriplet(const T1& t1, const T2& t2, const T3& t3)
{
  return Triplet<T1, T2, T3>(t1, t2, t3);
}

template<class Tuple, size_t N>
struct TupleNthType;

template<class Tuple>
struct TupleNthType<Tuple, 1> {
  typedef typename Tuple::first_type type;
};

template<class Tuple>
struct TupleNthType<Tuple, 2> {
  typedef typename Tuple::second_type type;
};

template<class Tuple>
struct TupleNthType<Tuple, 3> {
  typedef typename Tuple::third_type type;
};

template<size_t N>
struct TupleGet;

template<>
struct TupleGet<1> {
  template<class Tuple>
  static typename TupleNthType<Tuple, 1>::type get(const Tuple& tri)
  {
    return tri.first;
  }
};

template<>
struct TupleGet<2> {
  template<class Tuple>
  static typename TupleNthType<Tuple, 2>::type get(const Tuple& tri)
  {
    return tri.second;
  }
};

template<>
struct TupleGet<3> {
  template<class Tuple>
  static typename TupleNthType<Tuple, 3>::type get(const Tuple& tri)
  {
    return tri.third;
  }
};

template<size_t N1, size_t N2>
class Tuple2Pair {
public:
  template<class Tuple>
  std::pair<typename TupleNthType<Tuple, N1>::type,
            typename TupleNthType<Tuple, N2>::type>
  operator()(const Tuple& tri) const
  {
    return std::make_pair(TupleGet<N1>::get(tri), TupleGet<N2>::get(tri));
  }
};

} // namespace aria2

#endif // D_TRIPLET_H
