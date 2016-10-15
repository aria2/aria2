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

template <typename T, size_t N> constexpr size_t arraySize(T (&)[N])
{
  return N;
}

template <typename T, size_t N> class array_wrapper {
private:
  T array_[N];

public:
  array_wrapper() = default;

  operator T*() { return array_; }

  operator const T*() const { return array_; }

  size_t size() const { return N; }
};

// Expression Template for array

namespace expr {

template <typename L, typename R, typename Op> struct BinExpr {
  typedef typename Op::result_type value_type;

  BinExpr(L lhs, R rhs, Op op)
      : lhs(std::move(lhs)), rhs(std::move(rhs)), op(std::move(op))
  {
  }

  value_type operator[](size_t i) const { return op(lhs[i], rhs[i]); }

  L lhs;
  R rhs;
  Op op;
};

template <typename L, typename R,
          typename Op = std::bit_and<typename L::value_type>>
BinExpr<L, R, Op> operator&(L lhs, R rhs)
{
  return BinExpr<L, R, Op>(std::forward<L>(lhs), std::forward<R>(rhs), Op());
}

template <typename L, typename R,
          typename Op = std::bit_or<typename L::value_type>>
BinExpr<L, R, Op> operator|(L lhs, R rhs)
{
  return BinExpr<L, R, Op>(std::forward<L>(lhs), std::forward<R>(rhs), Op());
}

template <typename Arg, typename Op> struct UnExpr {
  typedef typename Op::result_type value_type;

  UnExpr(Arg arg, Op op) : arg(std::move(arg)), op(std::move(op)) {}

  value_type operator[](size_t i) const { return op(arg[i]); }

  Arg arg;
  Op op;
};

template <typename T> struct bit_neg : std::function<T(T)> {
  T operator()(T t) const { return ~t; }
};

template <typename Arg, typename Op = bit_neg<typename Arg::value_type>>
UnExpr<Arg, Op> operator~(Arg arg)
{
  return UnExpr<Arg, Op>(std::forward<Arg>(arg), Op());
}

template <typename T> struct Array {
  typedef T value_type;

  Array(T* t) : t(t) {}

  T operator[](size_t i) const { return t[i]; }

  T* t;
};

template <typename T> Array<T> array(T* t) { return Array<T>(t); }

} // namespace expr

} // namespace aria2

#endif // D_ARRAY_FUN_H
