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
#ifndef _D_ALPHA_NUMBER_DECORATOR_H_
#define _D_ALPHA_NUMBER_DECORATOR_H_

#include "NumberDecorator.h"
#include "DlAbortEx.h"
#include <algorithm>

namespace aria2 {

class AlphaNumberDecorator : public NumberDecorator
{
private:

  int32_t _width;

  std::string _zero;

  std::string widen(const std::string& s, int32_t width)
  {
    std::string t = s;
    while(t.size() < (size_t)width) {
      t.insert(0, _zero);
    }
    return t;
  }

public:
  AlphaNumberDecorator(int32_t width, bool uppercase = false):
    _width(width), _zero(uppercase?"A":"a") {}

  virtual ~AlphaNumberDecorator() {}

  virtual std::string decorate(int32_t number)
  {
    if(number < 0) {
      throw new DlAbortEx("The number must be greater than 0.");
    }
    if(number == 0) {
      return widen(_zero, _width);
    }
    int32_t base = 26;
    std::string x;
    while(number > 0) {
      int32_t r = number%base;
      char alpha = _zero[0]+r;
      x.insert(0, std::string(1, alpha));
      number /= base;
    }
    return widen(x, _width);
  }
};

} // namespace aria2

#endif // _D_ALPHA_NUMBER_DECORATOR_H_
