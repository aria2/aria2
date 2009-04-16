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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_ARRAY_FUN_H_
#define _D_ARRAY_FUN_H_

#include <cstdlib>
#include <functional>

namespace aria2 {

// calculate length of array

template<typename T, size_t N>
char (&char_array_ref(T (&)[N]))[N];

template<typename T, size_t N>
size_t arrayLength(T (&a)[N])
{
  return sizeof(char_array_ref(a));
}

template<typename T>
size_t arrayLength(T (&a)[0u])
{
  return 0;
}

template<typename T>
class array_ptr {
private:
  T* _array;

  // Copies are not allowed. Let's make them private.
  array_ptr(const array_ptr& s);

  array_ptr& operator=(const array_ptr& s);

  template<typename S>
  array_ptr& operator=(const array_ptr<S>& s);

public:
  array_ptr():_array(0) {}

  explicit array_ptr(T* array):_array(array) {}

  ~array_ptr()
  {
    delete [] _array;
  }

  operator T*()
  {
    return _array;
  }

  operator const T*() const
  {
    return _array;
  }
};

template<typename T, size_t N>
class array_wrapper {
private:
  T _array[N];
public:
  array_wrapper() {}

  operator T*()
  {
    return _array;
  }

  operator const T*() const
  {
    return _array;
  }

  size_t size() const
  {
    return N;
  }
};

// Expression Template for array

namespace expr {

template<typename T>
struct Expr {
  Expr(const T& expOp):_expOp(expOp) {}

  typename T::returnType operator[](size_t index) const
  {
    return _expOp(index);
  }

  const T& _expOp;
};

template<typename T>
struct And
{
  typedef T returnType;
  static inline T apply(T lhs, T rhs) { return lhs&rhs; }
};

template<typename T>
struct Noop
{
  typedef T returnType;
  static inline T apply(T arg) { return arg; }
};

template<typename T>
struct Negate
{
  typedef T returnType;
  static inline T apply(T arg) { return ~arg; }
};

template<typename T1, typename T2, typename BinOp>
struct ExpBinOp
{
  typedef typename BinOp::returnType returnType;

  ExpBinOp(const T1& lhs, const T2& rhs):_lhs(lhs), _rhs(rhs) {}

  returnType operator()(size_t index) const
  {
    return BinOp::apply(_lhs[index], _rhs[index]);
  }

  const T1& _lhs;
  const T2& _rhs;
};

template<typename T, typename UnOp>
struct ExpUnOp
{
  typedef typename UnOp::returnType returnType;

  ExpUnOp(const T& arg):_arg(arg) {}

  returnType operator()(size_t index) const
  {
    return UnOp::apply(_arg[index]);
  }

  const T& _arg;
};

// Partial specialization for pointers
template<typename T, typename UnOp>
struct ExpUnOp<T*, UnOp>
{
  typedef typename UnOp::returnType returnType;

  ExpUnOp(const T* arg):_arg(arg) {}

  returnType operator()(size_t index) const
  {
    return UnOp::apply(_arg[index]);
  }

  const T* _arg;
};

template<typename T, size_t N>
Expr<ExpUnOp<T(&)[N], Noop<T> > > arrayRef(T (&t)[N])
{
  typedef ExpUnOp<T(&)[N], Noop<T> > ExpUnOpT;
  return Expr<ExpUnOpT>(ExpUnOpT(t));
}

template<typename T>
Expr<ExpUnOp<T*, Noop<T> > > array(T* a)
{
  typedef ExpUnOp<T*, Noop<T> > ExpUnOpT;
  return Expr<ExpUnOpT>(ExpUnOpT(a));
}

template<typename T>
Expr<ExpUnOp<Expr<T>, Negate<typename T::returnType> > >
operator~(const Expr<T>& arg)
{
  typedef ExpUnOp<Expr<T>, Negate<typename T::returnType> > ExpUnOpT;
  return Expr<ExpUnOpT>(ExpUnOpT(arg));
}

template<typename T1, typename T2>
Expr<ExpBinOp<Expr<T1>, Expr<T2>, And<typename T1::returnType> > >
operator&(const Expr<T1>& lhs, const Expr<T2>& rhs)
{
  typedef ExpBinOp<Expr<T1>, Expr<T2>, And<typename T1::returnType> > ExpBinOpT;
  return Expr<ExpBinOpT>(ExpBinOpT(lhs, rhs));
}

} // namespace expr

} // namespace aria2

#endif // _D_ARRAY_FUN_H_
