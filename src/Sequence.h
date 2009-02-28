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
#ifndef _D_SEQUENCE_H_
#define _D_SEQUENCE_H_

#include <deque>

namespace aria2 {

template<typename T>
class Sequence
{
public:
  // Generates value in [_first, _last). _last is not included.
  class Value {
  private:
    T _first;
    T _last;
  public:
    Value(const T& first, const T& last):_first(first), _last(last) {}

    T next()
    {
      return _first++;
    }
    
    bool hasNext() const
    {
      return _first != _last;
    }
  };

  typedef std::deque<Value> Values;
private:
  Values _values;
public:
  Sequence(const Values& values):
    _values(values) {}

  Sequence() {}

  T next()
  {
    if(_values.empty()) {
      return T();
    }
    T t = _values.front().next();
    if(!_values.front().hasNext()) {
      _values.pop_front();
    }
    return t;
  }

  bool hasNext()
  {
    return !_values.empty();
  }

  std::deque<T> flush()
  {
    std::deque<T> r;
    while(hasNext()) {
      r.push_back(next());
    }
    return r;
  }
};

} // namespace aria2

#endif // _D_SEQUENCE_H_
