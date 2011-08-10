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
#include "PieceStatMan.h"

#include <limits>
#include <algorithm>

#include "SimpleRandomizer.h"
#include "bitfield.h"

namespace aria2 {

PieceStatMan::PieceStatMan(size_t pieceNum, bool randomShuffle):
  order_(pieceNum),
  counts_(pieceNum)
{
  for(size_t i = 0; i < pieceNum; ++i) {
    order_[i] = i;
  }
  // we need some randomness in ordering.
  if(randomShuffle) {
    std::random_shuffle(order_.begin(), order_.end(),
                        *(SimpleRandomizer::getInstance().get()));
  }
}

PieceStatMan::~PieceStatMan() {}

namespace {
void inc(int& x)
{
  if(x < std::numeric_limits<int>::max()) {
    ++x;
  }
}
} // namespace

namespace {
void sub(int& x)
{
  if(x > 0) {
    --x;
  }
}
} // namespace

void PieceStatMan::addPieceStats(const unsigned char* bitfield,
                                 size_t bitfieldLength)
{
  for(size_t i = 0, nbits = counts_.size(); i < nbits; ++i) {
    if(bitfield::test(bitfield, nbits, i)) {
      inc(counts_[i]);
    }
  }
}

void PieceStatMan::subtractPieceStats(const unsigned char* bitfield,
                                      size_t bitfieldLength)
{
  for(size_t i = 0, nbits = counts_.size(); i < nbits; ++i) {
    if(bitfield::test(bitfield, nbits, i)) {
      sub(counts_[i]);
    }
  }
}

void PieceStatMan::updatePieceStats(const unsigned char* newBitfield,
                                    size_t newBitfieldLength,
                                    const unsigned char* oldBitfield)
{
  for(size_t i = 0, nbits = counts_.size(); i < nbits; ++i) {
    bool inNew = bitfield::test(newBitfield, nbits, i);
    bool inOld = bitfield::test(oldBitfield, nbits, i);
    if(inNew) {
      if(!inOld) {
        inc(counts_[i]);
      }
    } else if(inOld) {
      sub(counts_[i]);
    }
  }
}

void PieceStatMan::addPieceStats(size_t index)
{
  inc(counts_[index]);
}

} // namespace aria2
