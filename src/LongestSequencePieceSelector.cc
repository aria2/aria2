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
#include "LongestSequencePieceSelector.h"
#include "bitfield.h"

namespace aria2 {

namespace {
size_t getStartIndex
(size_t from, const unsigned char* bitfield, size_t nbits)
{
  while(from < nbits && !bitfield::test(bitfield, nbits, from)) {
    ++from;
  }
  if(nbits <= from) {
    return nbits;
  } else {
    return from;
  }
}
} // namespace

namespace {
size_t getEndIndex
(size_t from, const unsigned char* bitfield, size_t nbits)
{
  while(from < nbits && bitfield::test(bitfield, nbits, from)) {
    ++from;
  }
  return from;
}
} // namespace

bool LongestSequencePieceSelector::select
(size_t& index, const unsigned char* bitfield, size_t nbits) const
{
  size_t mstartindex = 0;
  size_t mendindex = 0;
  size_t nextIndex = 0;
  while(nextIndex < nbits) {
    size_t startindex = getStartIndex(nextIndex, bitfield, nbits);
    if(startindex == nbits) {
      break;
    }
    size_t endindex = getEndIndex(startindex, bitfield, nbits);
    if(mendindex-mstartindex < endindex-startindex) {
      mstartindex = startindex;
      mendindex = endindex;
    }
    nextIndex = endindex;
  }
  if(mendindex-mstartindex > 0) {
    index = mendindex-1;
    return true;
  } else {
    return false;
  }
}

} // namespace aria2
