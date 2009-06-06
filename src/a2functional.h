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
#ifndef _D_A2_FUNCTIONAL_H_
#define _D_A2_FUNCTIONAL_H_

#include <functional>
#include "SharedHandle.h"
#include "A2STR.h"
#include <string>

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
  T& _to;
  T _delim;
public:
  template<typename S>
  Append(T& to, const S& delim):_to(to), _delim(delim) {}

  template<typename S>
  void operator()(const S& s) {
    _to += s+_delim;
  }
};

typedef Append<std::string> StringAppend;

template<typename T>
class auto_delete {
private:
  T _obj;
  void (*_deleter)(T);
public:
  auto_delete(T obj, void (*deleter)(T)):_obj(obj), _deleter(deleter) {}

  ~auto_delete()
  {
    _deleter(_obj);
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

template<typename T1, typename T2>
inline std::string strconcat(const T1& a1, const T2& a2)
{
  return std::string(a1).append(a2);
}

template<typename T1, typename T2, typename T3>
inline std::string strconcat(const T1& a1, const T2& a2, const T3& a3)
{
  return std::string(a1).append(a2).append(a3);
}

template<typename T1, typename T2, typename T3, typename T4>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
  return std::string(a1).append(a2).append(a3).append(a4);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6, const T7& a7)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7, typename T8>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6, const T7& a7, const T8& a8)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7).append(a8);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7, typename T8, typename T9>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6, const T7& a7, const T8& a8,
	  const T9& a9)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7).append(a8).append(a9);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7, typename T8, typename T9, typename T10>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6, const T7& a7, const T8& a8,
	  const T9& a9, const T10& a10)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7).append(a8).append(a9).append(a10);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7, typename T8, typename T9, typename T10,
	 typename T11>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6, const T7& a7, const T8& a8,
	  const T9& a9, const T10& a10, const T11& a11)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7).append(a8).append(a9).append(a10).append(a11);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7, typename T8, typename T9, typename T10,
	 typename T11, typename T12>
inline std::string
strconcat(const T1& a1, const T2& a2, const T3& a3, const T4& a4,
	  const T5& a5, const T6& a6, const T7& a7, const T8& a8,
	  const T9& a9, const T10& a10, const T11& a11,
	  const T12& a12)
{
  return std::string(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7).append(a8).append(a9).append(a10).append(a11).append(a12);
}

template<typename T1, typename T2>
inline void strappend(std::string& base, const T1& a1, const T2& a2)
{
  base.append(a1).append(a2);
}

template<typename T1, typename T2, typename T3>
inline void strappend(std::string& base,
		       const T1& a1, const T2& a2, const T3& a3)
{
  base.append(a1).append(a2).append(a3);
}

template<typename T1, typename T2, typename T3, typename T4>
inline void strappend(std::string& base,
		       const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
  base.append(a1).append(a2).append(a3).append(a4);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline void strappend(std::string& base,
		       const T1& a1, const T2& a2, const T3& a3, const T4& a4,
		       const T5& a5)
{
  base.append(a1).append(a2).append(a3).append(a4).append(a5);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6>
inline void strappend(std::string& base,
		       const T1& a1, const T2& a2, const T3& a3, const T4& a4,
		      const T5& a5, const T6& a6)
{
  base.append(a1).append(a2).append(a3).append(a4).append(a5).append(a6);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7>
inline void strappend(std::string& base,
		       const T1& a1, const T2& a2, const T3& a3, const T4& a4,
		      const T5& a5, const T6& a6, const T7& a7)
{
  base.append(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5,
	 typename T6, typename T7, typename T8>
inline void strappend(std::string& base,
		      const T1& a1, const T2& a2, const T3& a3, const T4& a4,
		      const T5& a5, const T6& a6, const T7& a7, const T8& a8)
{
  base.append(a1).append(a2).append(a3).append(a4).append(a5).append(a6).append(a7).append(a8);
}

} // namespace aria2

#endif // _D_A2_FUNCTIONAL_H_
