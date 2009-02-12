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

template<typename T>
class bit_negate:public std::unary_function<T, T> {
public:
  T operator()(const T& t) const
  {
    return ~t;
  }
};

template<typename T>
class bit_and:public std::binary_function<T, T, T> {
public:
  T operator()(const T& t1, const T& t2) const
  {
    return t1&t2;
  }
};

template<typename R>
class array_function_base {
public:
  virtual ~array_function_base() {}

  virtual R operator[](size_t index) const = 0;

  virtual array_function_base* clone() const = 0;
};

template<typename A, typename F>
class array_unary_function:public array_function_base<typename F::result_type> {
private:
  A _a;
  F _f;
public:
  array_unary_function(A a, F f):_a(a), _f(f) {}

  virtual typename F::result_type operator[](size_t index) const
  {
    return _f(_a[index]);
  }

  virtual array_function_base<typename F::result_type>* clone() const
  {
    return new array_unary_function(*this);
  }
};

template<typename A, typename B, typename F>
class array_binary_function:public array_function_base<typename F::result_type>{
private:
  A _a;
  B _b;
  F _f;
public:
  array_binary_function(A a, B b, F f):_a(a), _b(b), _f(f) {}

  virtual typename F::result_type operator[](size_t index) const
  {
    return _f(_a[index], _b[index]);
  }

  virtual array_function_base<typename F::result_type>* clone() const
  {
    return new array_binary_function(*this);
  }
};

template<typename R>
class array_fun {
private:
  array_function_base<R>* _p;
public:
  template<typename A, typename F>
  array_fun(A a, F f):_p(new array_unary_function<A, F>(a, f)) {}

  template<typename A, typename B, typename F>
  array_fun(A a, B b, F f):_p(new array_binary_function<A, B, F>(a, b, f)) {}

  array_fun(const array_fun& af):_p(af._p->clone()) {}

  ~array_fun()
  {
    delete _p;
  }

  array_fun& operator=(const array_fun& af)
  {
    if(this != &af) {
      delete _p;
      _p = af._p->clone();
    }
    return *this;
  }

  R operator[](size_t index) const
  {
    return (*_p)[index];
  }

  typedef R result_type;
};

template<typename R, typename A>
array_fun<R>
array_negate(A a)
{
  return array_fun<R>(a, bit_negate<R>());
}

template<typename T>
array_fun<T>
array_negate(T* a)
{
  return array_fun<T>(a, bit_negate<T>());
}

template<typename A>
array_fun<typename A::result_type>
array_negate(A a)
{
  return array_fun<typename A::result_type>(a, bit_negate<typename A::result_type>());
}

template<typename R, typename A, typename B>
array_fun<R>
array_and(A a, B b)
{
  return array_fun<R>(a, b, bit_and<R>());
}

template<typename T>
array_fun<T>
array_and(T* a, T* b)
{
  return array_fun<T>(a, b, bit_and<T>());
}

template<typename T>
array_fun<typename T::result_type>
array_and(T a, T b)
{
  return array_fun<typename T::result_type>(a, b, bit_and<typename T::result_type>());
}

template<typename A, typename B>
array_fun<typename A::result_type>
array_and(A a, B b)
{
  return array_fun<typename A::result_type>(a, b, bit_and<typename A::result_type>());
}

template<typename A, typename B>
array_fun<typename B::result_type>
array_and(A a, B b)
{
  return array_fun<typename B::result_type>(a, b, bit_and<typename B::result_type>());
}

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

  T& operator[](size_t index)
  {
    return _array[index];
  }

  const T& operator[](size_t index) const
  {
    return _array[index];
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

  T& operator[](size_t index)
  {
    return _array[index];
  }

  const T& operator[](size_t index) const
  {
    return _array[index];
  }

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

} // namespace aria2

#endif // _D_ARRAY_FUN_H_
