/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#ifndef D_BASE64_H
#define D_BASE64_H

#include <string>

namespace aria2 {

namespace base64 {

template<typename InputIterator>
std::string encode(InputIterator first, InputIterator last)
{
  static const char CHAR_TABLE[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
  };
  std::string res;
  size_t len = last-first;
  if(len == 0) {
    return res;
  }
  size_t r = len%3;
  InputIterator j = last-r;
  char temp[4];
  while(first != j) {
    int n = static_cast<unsigned char>(*first++) << 16;
    n += static_cast<unsigned char>(*first++) << 8;
    n += static_cast<unsigned char>(*first++);
    temp[0] = CHAR_TABLE[n >> 18];
    temp[1] = CHAR_TABLE[(n >> 12) & 0x3fu];
    temp[2] = CHAR_TABLE[(n >> 6) & 0x3fu];
    temp[3] = CHAR_TABLE[n & 0x3fu];
    res.append(temp, sizeof(temp));
  }
  if(r == 2) {
    int n = static_cast<unsigned char>(*first++) << 16;
    n += static_cast<unsigned char>(*first++) << 8;
    temp[0] = CHAR_TABLE[n >> 18];
    temp[1] = CHAR_TABLE[(n >> 12) & 0x3fu];
    temp[2] = CHAR_TABLE[(n >> 6) & 0x3fu];
    temp[3] = '=';
    res.append(temp, sizeof(temp));
  } else if(r == 1) {
    int n = static_cast<unsigned char>(*first++) << 16;
    temp[0] = CHAR_TABLE[n >> 18];
    temp[1] = CHAR_TABLE[(n >> 12) & 0x3fu];
    temp[2] = '=';
    temp[3] = '=';
    res.append(temp, sizeof(temp));
  }
  return res;
}

template<typename InputIterator>
InputIterator getNext
(InputIterator first,
 InputIterator last,
 const int* tbl)
{
  for(; first != last; ++first) {
    if(tbl[static_cast<size_t>(*first)] != -1 || *first == '=') {
      break;
    }
  }
  return first;
}

template<typename InputIterator>
std::string decode(InputIterator first, InputIterator last)
{
  static const int INDEX_TABLE[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
  };
  std::string res;
  InputIterator k[4];
  int eq = 0;
  for(; first != last;) {
    for(int i = 1; i <= 4; ++i) {
      k[i-1] = getNext(first, last, INDEX_TABLE);
      if(k[i-1] == last) {
        // If i == 1, input may look like this: "TWFu\n" (i.e.,
        // garbage at the end)
        if(i != 1) {
          res.clear();
        }
        return res;
      } else if(*k[i-1] == '=' && eq == 0) {
        eq = i;
      }
      first = k[i-1]+1;
    }
    if(eq) {
      break;
    }
    int n = (INDEX_TABLE[static_cast<unsigned char>(*k[0])] << 18)+
      (INDEX_TABLE[static_cast<unsigned char>(*k[1])] << 12)+
      (INDEX_TABLE[static_cast<unsigned char>(*k[2])] << 6)+
      INDEX_TABLE[static_cast<unsigned char>(*k[3])];
    res += n >> 16;
    res += n >> 8 & 0xffu;
    res += n & 0xffu;
  }
  if(eq) {
    if(eq <= 2) {
      res.clear();
      return res;
    } else {
      for(int i = eq; i <= 4; ++i) {
        if(*k[i-1] != '=') {
          res.clear();
          return res;
        }
      }
      if(eq == 3) {
        int n = (INDEX_TABLE[static_cast<unsigned char>(*k[0])] << 18)+
          (INDEX_TABLE[static_cast<unsigned char>(*k[1])] << 12);
        res += n >> 16;
      } else if(eq == 4) {
        int n = (INDEX_TABLE[static_cast<unsigned char>(*k[0])] << 18)+
          (INDEX_TABLE[static_cast<unsigned char>(*k[1])] << 12)+
          (INDEX_TABLE[static_cast<unsigned char>(*k[2])] << 6);
        res += n >> 16;
        res += n >> 8 & 0xffu;
      }
    }
  }
  return res;
}

} // namespace base64

} // namespace aria2

#endif // D_BASE64_H
