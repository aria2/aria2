/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#ifndef D_BASE32_H
#define D_BASE32_H

#include "common.h"

#include <string>

#include "util.h"

namespace aria2 {

namespace base32 {

std::string encode(const std::string& src);

template<typename InputIterator>
std::string decode(InputIterator first, InputIterator last)
{
  std::string ret;
  size_t len = last-first;
  if(len%8) {
    return ret;
  }
  bool done = false;
  for(; first != last && !done; first += 8) {
    uint64_t buf = 0;
    size_t bits = 0;
    for(size_t i = 0; i < 8; ++i) {
      char ch = *(first+i);
      unsigned char value;
      if('A' <= ch && ch <= 'Z') {
        value = ch-'A';
      } else if('2' <= ch && ch <= '7') {
        value = ch-'2'+26;
      } else if(ch == '=') {
        done = true;
        break;
      } else {
        ret.clear();
        return ret;
      }
      buf <<= 5;
      buf += value;
      bits += 5;
    }
    buf >>= (bits%8);
    bits = bits/8*8;
    buf = hton64(buf);
    char* p = reinterpret_cast<char*>(&buf);
    ret.append(&p[(64-bits)/8], &p[8]);
  }
  return ret;
}

} // namespace base32

} // namespace aria2

#endif // D_BASE32_H
