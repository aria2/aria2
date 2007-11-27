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
#include <functional>
#include "SharedHandle.h"

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

template<class BinaryOp, class UnaryOp>
class adopt2nd_t:public std::binary_function<typename BinaryOp::first_argument_type,
					     typename UnaryOp::argument_type,
					     typename BinaryOp::result_type> {
private:
  BinaryOp _binaryOp;
  UnaryOp _unaryOp;
public:
  adopt2nd_t(const BinaryOp& b, const UnaryOp& u):
    _binaryOp(b), _unaryOp(u) {}

  typename BinaryOp::result_type
  operator()(const typename BinaryOp::first_argument_type& x,
	     const typename UnaryOp::argument_type& y)
  {
    return _binaryOp(x, _unaryOp(y));
  }

};

template <class BinaryOp, class UnaryOp>
inline adopt2nd_t<BinaryOp, UnaryOp>
adopt2nd(const BinaryOp& binaryOp, const UnaryOp& unaryOp)
{
  return adopt2nd_t<BinaryOp, UnaryOp>(binaryOp, unaryOp);
};

template<typename T, std::size_t N>
char (&char_array_ref(T (&)[N]))[N];

template<typename T, std::size_t N>
std::size_t
arrayLength(T (&a)[N])
{
  return sizeof(char_array_ref(a));
}

template<typename T>
std::size_t
arrayLength(T (&a)[0u])
{
  return 0;
}
