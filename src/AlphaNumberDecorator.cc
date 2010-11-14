/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "AlphaNumberDecorator.h"

#include <algorithm>

namespace aria2 {

namespace {
std::string widen(const std::string& s, size_t width, char zeroChar)
{
  std::string t = s;
  std::string zero(1, zeroChar);
  while(t.size() < width) {
    t.insert(0, zero);
  }
  return t;
}
} // namespace

AlphaNumberDecorator::AlphaNumberDecorator(size_t width, bool uppercase)
  : width_(width),
    zero_(uppercase?'A':'a')
{}

AlphaNumberDecorator::~AlphaNumberDecorator() {}

std::string AlphaNumberDecorator::decorate(unsigned int number)
{
  if(number == 0) {
    return widen(std::string(1, zero_), width_, zero_);
  }

  int base = 26;
  char u[14]; // because if unsigned int is 64bit, which is the
              // biggest integer for the time being and number is
              // UINT64_MAX, you get "HLHXCZMXSYUMQP"
  size_t index = 0;
  do {
    unsigned int quot = number/base;
    unsigned int rem = number%base;
    u[index++] = zero_+rem;
    number = quot;
  } while(number);
  std::reverse(&u[0], &u[index]);

  return widen(std::string(&u[0], &u[index]), width_, zero_);
}

} // namespace aria2
