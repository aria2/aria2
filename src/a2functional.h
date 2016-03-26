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
#ifndef D_A2_FUNCTIONAL_H
#define D_A2_FUNCTIONAL_H

#include "common.h"

#include <functional>
#include <string>
#include <algorithm>
#include <memory>
#include <chrono>

#include "A2STR.h"

namespace aria2 {

class Deleter {
public:
  template <class T> void operator()(T* ptr) { delete ptr; }
};

template <typename T, typename F> struct Defer {
  Defer(T t, F f) : t(t), f(std::move(f)) {}

  ~Defer() { f(t); }

  T t;
  F f;
};

template <typename T, typename F> Defer<T, F> defer(T&& t, F f)
{
  return Defer<T, F>(std::forward<T>(t), std::forward<F>(f));
}

template <typename InputIterator, typename DelimiterType>
std::string strjoin(InputIterator first, InputIterator last,
                    const DelimiterType& delim)
{
  std::string result;
  if (first == last) {
    return result;
  }
  InputIterator beforeLast = last - 1;
  for (; first != beforeLast; ++first) {
    result += *first;
    result += delim;
  }
  result += *beforeLast;
  return result;
}

// Applies unaryOp through first to last and joins the result with
// delimiter delim.
template <typename InputIterator, typename DelimiterType, typename UnaryOp>
std::string strjoin(InputIterator first, InputIterator last,
                    const DelimiterType& delim, const UnaryOp& unaryOp)
{
  std::string result;
  if (first == last) {
    return result;
  }
  InputIterator beforeLast = last - 1;
  for (; first != beforeLast; ++first) {
    result += unaryOp(*first);
    result += delim;
  }
  result += unaryOp(*beforeLast);
  return result;
}

template <typename T>
class LeastRecentAccess : public std::binary_function<T, T, bool> {
public:
  bool operator()(const std::shared_ptr<T>& lhs,
                  const std::shared_ptr<T>& rhs) const
  {
    return lhs->getLastAccessTime() < rhs->getLastAccessTime();
  }

  bool operator()(const T& lhs, const T& rhs) const
  {
    return lhs.getLastAccessTime() < rhs.getLastAccessTime();
  }
};

template <typename T, typename S> bool in(T x, S s, S t)
{
  return s <= x && x <= t;
}

template <typename T> struct DerefLess {
  bool operator()(const T& lhs, const T& rhs) const { return *lhs < *rhs; }
};

template <typename T> struct DerefEqualTo {
  bool operator()(const T& lhs, const T& rhs) const { return *lhs == *rhs; }
};

template <typename T> struct DerefEqual {
  T target;

  DerefEqual(const T& t) : target(t) {}
  bool operator()(const T& other) const { return *target == *other; }
};

template <typename T> struct DerefEqual<T> derefEqual(const T& t) {
  return DerefEqual<T>(t);
}

template <typename T>
struct RefLess {
  bool operator()(const std::shared_ptr<T>& lhs,
                  const std::shared_ptr<T>& rhs) const
  {
    return lhs.get() < rhs.get();
  }
};

#if __cplusplus > 201103L
using std::make_unique;
#else  // __cplusplus <= 201103L
template <typename T, typename... U>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(U&&... u)
{
  return std::unique_ptr<T>(new T(std::forward<U>(u)...));
}

template <typename T>
typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(size_t size)
{
  return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}
#endif // __cplusplus <= 201103L

// User-defined literals for K, M, and G (powers of 1024)

constexpr unsigned long long operator"" _k(unsigned long long k)
{
  return k * 1024;
}

constexpr unsigned long long operator"" _m(unsigned long long m)
{
  return m * 1024 * 1024;
}

constexpr unsigned long long operator"" _g(unsigned long long g)
{
  return g * 1024 * 1024 * 1024;
}

constexpr std::chrono::hours operator"" _h(unsigned long long h)
{
  return std::chrono::hours(h);
}

constexpr std::chrono::minutes operator"" _min(unsigned long long min)
{
  return std::chrono::minutes(min);
}

constexpr std::chrono::seconds operator"" _s(unsigned long long s)
{
  return std::chrono::seconds(s);
}

constexpr std::chrono::milliseconds operator"" _ms(unsigned long long ms)
{
  return std::chrono::milliseconds(ms);
}

} // namespace aria2

#endif // D_A2_FUNCTIONAL_H
