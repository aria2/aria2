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
#ifndef _D_BITFIELD_H_
#define _D_BITFIELD_H_

#include "common.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "util.h"

namespace aria2 {

namespace bitfield {

// Returns the bit mask for the last byte. For example, nbits = 9,
// then 0x80 is returned. nbits = 12, then 0xf0 is returned.
inline unsigned char lastByteMask(size_t nbits)
{
  if(nbits == 0) {
    return 0;
  } else {
    return -256 >> (8-((nbits+7)/8*8-nbits));
  }
}

// Returns true if index-th bits is set. Otherwise returns false.
template<typename Array>
inline bool test(const Array& bitfield, size_t nbits, size_t index)
{
  assert(index < nbits);
  unsigned char mask = 128 >> (index%8);
  return (bitfield[index/8]&mask) != 0;
}

inline size_t countBit32(uint32_t n)
{
  static const int nbits[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8, 
  };
  return
    nbits[n&0xffu]+
    nbits[(n >> 8)&0xffu]+
    nbits[(n >> 16)&0xffu]+
    nbits[(n >> 24)&0xffu];
}

// Counts set bit in bitfield.
inline size_t countSetBit(const unsigned char* bitfield, size_t nbits)
{
  if(nbits == 0) {
    return 0;
  }
  size_t count = 0;
  size_t size = sizeof(uint32_t);
  size_t len = (nbits+7)/8;
  if(nbits%32 != 0) {
    --len;
    count +=
      countBit32(static_cast<uint32_t>(bitfield[len]&lastByteMask(nbits)));
  }
  size_t to = len/size;
  for(size_t i = 0; i < to; ++i) {
    uint32_t v;
    memcpy(&v, &bitfield[i*size], sizeof(v));
    count += countBit32(v);
  }
  for(size_t i = len-len%size; i < len; ++i) {
    count += countBit32(static_cast<uint32_t>(bitfield[i]));
  }
  return count;
}

void flipBit(unsigned char* data, size_t length, size_t bitIndex);

// Stores first missing bit index of bitfield to index.  bitfield
// contains nbits. Returns true if missing bit index is
// found. Otherwise returns false.
template<typename Array>
bool getFirstMissingIndex
(size_t& index, const Array& bitfield, size_t nbits)
{
  const size_t bitfieldLength = (nbits+7)/8;
  for(size_t i = 0; i < bitfieldLength; ++i) {
    unsigned char mask = 128;
    size_t tindex = i*8;
    for(size_t bi = 0; bi < 8 && tindex < nbits; ++bi, mask >>= 1, ++tindex) {
      if(bitfield[i] & mask) {
        index = tindex;
        return true;
      }
    }
  }
  return false;
}

// Appends first at most n set bit index in bitfield to out.  bitfield
// contains nbits bits.  Returns the number of appended bit indexes.
template<typename Array, typename OutputIterator>
size_t getFirstNMissingIndex
(OutputIterator out, size_t n, const Array& bitfield, size_t nbits)
{
  if(n == 0) {
    return 0;
  }
  const size_t origN = n;
  const size_t bitfieldLength = (nbits+7)/8;
  for(size_t i = 0; i < bitfieldLength; ++i) {
    unsigned char mask = 128;
    size_t tindex = i*8;
    for(size_t bi = 0; bi < 8 && tindex < nbits; ++bi, mask >>= 1, ++tindex) {
      if(bitfield[i] & mask) {
        *out++ = tindex;
        if(--n == 0) {
          return origN;
        }
      }
    }
  }
  return origN-n;
}

} // namespace bitfield

} // namespace aria2

#endif // _D_BITFIELD_H_
