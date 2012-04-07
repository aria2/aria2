/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
#ifndef D_ARRAY_FUN_H
#define D_ARRAY_FUN_H

#include "common.h"

#include <cstdlib>
#include <functional>

namespace aria2 {

// calculate length of array
template<typename T, size_t N>
char (&char_array_ref_fun(T (&)[N]))[N];

// For 0 length array
template<typename T>
char (&char_array_ref_fun(T (&)[0u]))[0u];

// To calculate size of array at compile time, we use macro here.
#define A2_ARRAY_LEN(X) sizeof(char_array_ref_fun(X))

template<typename T, size_t N>
T* vbegin(T (&a)[N])
{
  return a;
}

template<typename T, size_t N>
T* vend(T (&a)[N])
{
  return a+N;
}

template<typename T>
class array_ptr {
private:
  T* array_;

  // Copies are not allowed. Let's make them private.
  array_ptr(const array_ptr& s);

  array_ptr& operator=(const array_ptr& s);

  template<typename S>
  array_ptr& operator=(const array_ptr<S>& s);

public:
  array_ptr():array_(0) {}

  explicit array_ptr(T* array):array_(array) {}

  ~array_ptr()
  {
    delete [] array_;
  }

  operator T*()
  {
    return array_;
  }

  operator const T*() const
  {
    return array_;
  }

  void reset(T* array)
  {
    array_ = array;
  }
};

template<typename T, size_t N>
class array_wrapper {
private:
  T array_[N];
public:
  array_wrapper() {}

  operator T*()
  {
    return array_;
  }

  operator const T*() const
  {
    return array_;
  }

  size_t size() const
  {
    return N;
  }
};

// Expression Template for array

namespace expr {

template<typename L, typename OpTag, typename R>
struct BinExpr {
  BinExpr(const L& l, const R& r):l_(l), r_(r) {}

  typedef typename OpTag::returnType returnType;

  returnType operator[](size_t index) const
  {
    return OpTag::apply(l_[index], r_[index]);
  }

  const L& l_;
  const R& r_;
};

template<typename OpTag, typename A>
struct UnExpr {
  UnExpr(const A& a):a_(a) {}

  typedef typename OpTag::returnType returnType;

  returnType operator[](size_t index) const
  {
    return OpTag::apply(a_[index]);
  }

  const A& a_;
};

template<typename T>
struct And
{
  typedef T returnType;
  static inline returnType apply(T lhs, T rhs) { return lhs&rhs; }
};

template<typename T>
struct Or
{
  typedef T returnType;
  static inline returnType apply(T lhs, T rhs) { return lhs|rhs; }
};

template<typename T>
struct Negate
{
  typedef T returnType;
  static inline returnType apply(T a) { return ~a; }
};

template<typename T>
struct Array
{
  typedef T returnType;

  Array(const T* t):t_(t) {}

  const T* t_;

  returnType operator[](size_t index) const { return t_[index]; }
};

template<typename T>
Array<T>
array(const T* t) { return Array<T>(t); }

template<typename L, typename R>
BinExpr<L, And<typename L::returnType>, R>
operator&(const L& l, const R& r)
{
  return BinExpr<L, And<typename L::returnType>, R>(l, r);
}

template<typename L, typename R>
BinExpr<L, Or<typename L::returnType>, R>
operator|(const L& l, const R& r)
{
  return BinExpr<L, Or<typename L::returnType>, R>(l, r);
}

template<typename A>
UnExpr<Negate<typename A::returnType>, A>
operator~(const A& a)
{
  return UnExpr<Negate<typename A::returnType>, A>(a);
}

} // namespace expr

} // namespace aria2

#endif // D_ARRAY_FUN_H
