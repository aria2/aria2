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
#ifndef _D_SEQUENCE_H_
#define _D_SEQUENCE_H_

#include <vector>

namespace aria2 {

template<typename T>
class Sequence
{
public:
  // Generates value in [first_, last_). last_ is not included.
  class Value {
  private:
    T first_;
    T last_;
  public:
    Value(const T& first, const T& last):first_(first), last_(last) {}

    T next()
    {
      return first_++;
    }
    
    bool hasNext() const
    {
      return first_ != last_;
    }
  };

  typedef std::vector<Value> Values;
private:
  Values values_;
  typename Values::iterator cur_;
public:
  Sequence(const Values& values):
    values_(values), cur_(values_.begin())  {}

  Sequence() {}

  T next()
  {
    if(cur_ == values_.end()) {
      return T();
    }
    T t = (*cur_).next();
    if(!(*cur_).hasNext()) {
      ++cur_;
    }
    return t;
  }

  bool hasNext()
  {
    return cur_ != values_.end();
  }

  std::vector<T> flush()
  {
    std::vector<T> r;
    while(hasNext()) {
      r.push_back(next());
    }
    return r;
  }
};

} // namespace aria2

#endif // _D_SEQUENCE_H_
