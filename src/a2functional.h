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

#include "SharedHandle.h"
#include "A2STR.h"

namespace aria2 {

// mem_fun_t for SharedHandle
template <class ReturnType, typename ClassType>
class mem_fun_sh_t:public std::unary_function< SharedHandle<ClassType>, ReturnType>
{
private:
  ReturnType (ClassType::*f)();

public:
  mem_fun_sh_t(ReturnType (ClassType::*f)()):f(f) {}

  ReturnType operator()(const SharedHandle<ClassType>& x) const
  {
    return (x.get()->*f)();
  }
};

// const_mem_fun_t for SharedHandle
template <class ReturnType, typename ClassType>
class const_mem_fun_sh_t:public std::unary_function< SharedHandle<ClassType>, ReturnType>
{
private:
  ReturnType (ClassType::*f)() const;

public:
  const_mem_fun_sh_t(ReturnType (ClassType::*f)() const):f(f) {}

  ReturnType operator()(const SharedHandle<ClassType>& x) const
  {
    return (x.get()->*f)();
  }
};

template <class ReturnType, typename ClassType>
mem_fun_sh_t<ReturnType, ClassType>
mem_fun_sh(ReturnType (ClassType::*f)())
{
  return mem_fun_sh_t<ReturnType, ClassType>(f);
};

template <class ReturnType, typename ClassType>
const_mem_fun_sh_t<ReturnType, ClassType>
mem_fun_sh(ReturnType (ClassType::*f)() const)
{
  return const_mem_fun_sh_t<ReturnType, ClassType>(f);
};

// mem_fun1_t for SharedHandle
template<typename ReturnType, typename ClassType, typename ArgType>
class mem_fun1_sh_t:public std::binary_function<SharedHandle<ClassType>,
                                                ArgType,
                                                ReturnType>
{
private:
  ReturnType (ClassType::*f)(ArgType);

public:
  mem_fun1_sh_t(ReturnType (ClassType::*f)(ArgType)):f(f) {}

  ReturnType operator()(const SharedHandle<ClassType>& x, ArgType a) const
  {
    return (x.get()->*f)(a);
  }
};

template<typename ReturnType, typename ClassType, typename ArgType>
mem_fun1_sh_t<ReturnType, ClassType, ArgType>
mem_fun_sh(ReturnType (ClassType::*f)(ArgType))
{
  return mem_fun1_sh_t<ReturnType, ClassType, ArgType>(f);
};

template<class BinaryOp, class UnaryOp>
class adopt2nd_t:public std::binary_function<typename BinaryOp::first_argument_type,
                                             typename UnaryOp::argument_type,
                                             typename BinaryOp::result_type> {
private:
  BinaryOp binaryOp_;
  UnaryOp unaryOp_;
public:
  adopt2nd_t(const BinaryOp& b, const UnaryOp& u):
    binaryOp_(b), unaryOp_(u) {}

  typename BinaryOp::result_type
  operator()(const typename BinaryOp::first_argument_type& x,
             const typename UnaryOp::argument_type& y)
  {
    return binaryOp_(x, unaryOp_(y));
  }

};

template <class BinaryOp, class UnaryOp>
inline adopt2nd_t<BinaryOp, UnaryOp>
adopt2nd(const BinaryOp& binaryOp, const UnaryOp& unaryOp)
{
  return adopt2nd_t<BinaryOp, UnaryOp>(binaryOp, unaryOp);
};

template<typename Pair>
class Ascend1st:public std::binary_function<Pair, Pair, bool>
{
public:
  bool operator()(const Pair& p1, const Pair& p2) const
  {
    return p1.first < p2.first;
  }
};

template<typename Pair>
class select2nd
{
public:
  typename Pair::second_type operator()(const Pair& p) const
  {
    return p.second;
  }

  typename Pair::second_type operator()(Pair& p) const
  {
    return p.second;
  }
};

class Deleter {
public:
  template<class T>
  void operator()(T* ptr) {
    delete ptr;
  }
};

template<typename T>
class Append {
private:
  T& to_;
  T delim_;
public:
  template<typename S>
  Append(T& to, const S& delim):to_(to), delim_(delim) {}

  template<typename S>
  void operator()(const S& s) {
    to_ += s+delim_;
  }
};

typedef Append<std::string> StringAppend;

template<typename T>
class auto_delete {
private:
  T obj_;
  void (*deleter_)(T);
public:
  auto_delete(T obj, void (*deleter)(T)):obj_(obj), deleter_(deleter) {}

  ~auto_delete()
  {
    deleter_(obj_);
  }
};

template<typename T>
class auto_delete_d {
private:
  T obj_;
public:
  auto_delete_d(T obj):obj_(obj) {}

  ~auto_delete_d()
  {
    delete obj_;
  }
};

template<typename T, typename R>
class auto_delete_r {
private:
  T obj_;
  R (*deleter_)(T);
public:
  auto_delete_r(T obj, R (*deleter)(T)):obj_(obj), deleter_(deleter) {}

  ~auto_delete_r()
  {
    (void)deleter_(obj_);
  }
};

template<class Container>
class auto_delete_container {
private:
  Container* c_;
public:
  auto_delete_container(Container* c):c_(c) {}

  ~auto_delete_container()
  {
    std::for_each(c_->begin(), c_->end(), Deleter());
    delete c_;
  }
};

template<typename InputIterator, typename DelimiterType>
std::string strjoin(InputIterator first, InputIterator last,
                    const DelimiterType& delim)
{
  std::string result;
  if(first == last) {
    return result;
  }
  InputIterator beforeLast = last-1;
  for(; first != beforeLast; ++first) {
    result += *first;
    result += delim;
  }
  result += *beforeLast;
  return result;
}

// Applies unaryOp through first to last and joins the result with
// delimiter delim.
template<typename InputIterator, typename DelimiterType, typename UnaryOp>
std::string strjoin(InputIterator first, InputIterator last,
                    const DelimiterType& delim, const UnaryOp& unaryOp)
{
  std::string result;
  if(first == last) {
    return result;
  }
  InputIterator beforeLast = last-1;
  for(; first != beforeLast; ++first) {
    result += unaryOp(*first);
    result += delim;
  }
  result += unaryOp(*beforeLast);
  return result;
}

template<typename T>
class LeastRecentAccess:public std::binary_function<T, T, bool> {
public:
  bool operator()(const SharedHandle<T>& lhs,
                  const SharedHandle<T>& rhs) const
  {
    return lhs->getLastAccessTime() < rhs->getLastAccessTime();
  }

  bool operator()(const T& lhs, const T& rhs) const
  {
    return lhs.getLastAccessTime() < rhs.getLastAccessTime();
  }
};

template<typename T, typename S>
bool in(T x, S s, S t)
{
  return s <= x && x <= t;
}

template<typename T>
struct DerefLess {
  bool operator()(const T& lhs, const T& rhs) const
  {
    return *lhs < *rhs;
  }
};

template<typename T>
struct DerefEqualTo {
  bool operator()(const T& lhs, const T& rhs) const
  {
    return *lhs == *rhs;
  }
};

template<typename T>
struct DerefEqual {
  T target;

  DerefEqual(const T& t):target(t) {}
  bool operator()(const T& other) const
  {
    return *target == *other;
  }
};

template<typename T>
struct DerefEqual<T> derefEqual(const T& t)
{
  return DerefEqual<T>(t);
}

template<typename T>
struct RefLess {
  bool operator()(const SharedHandle<T>& lhs, const SharedHandle<T>& rhs) const
  {
    return lhs.get() < rhs.get();
  }
};

} // namespace aria2

#endif // D_A2_FUNCTIONAL_H
