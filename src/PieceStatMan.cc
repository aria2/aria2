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

#include <algorithm>

#include "SimpleRandomizer.h"
#include "bitfield.h"

namespace aria2 {

PieceStat::PieceStat(size_t index):order_(0), index_(index), count_(0) {}

void PieceStat::addCount()
{
  if(count_ < SIZE_MAX) {
    ++count_;
  }
}

void PieceStat::subCount()
{
  if(count_ > 0) {
    --count_;
  }
}

class GenPieceStat {
private:
  size_t index_;
public:
  GenPieceStat():index_(0) {}

  SharedHandle<PieceStat> operator()()
  {
    return SharedHandle<PieceStat>(new PieceStat(index_++));
  }
};

PieceStatMan::PieceStatMan(size_t pieceNum, bool randomShuffle):
  pieceStats_(pieceNum),
  sortedPieceStatIndexes_(pieceNum)
{
  std::generate(pieceStats_.begin(), pieceStats_.end(), GenPieceStat());
  std::vector<SharedHandle<PieceStat> > sortedPieceStats(pieceStats_);
  // we need some randomness in ordering.
  if(randomShuffle) {
    std::random_shuffle(sortedPieceStats.begin(), sortedPieceStats.end(),
                        *(SimpleRandomizer::getInstance().get()));
  }
  {
    size_t order = 0;
    for(std::vector<SharedHandle<PieceStat> >::const_iterator i =
          sortedPieceStats.begin(), eoi = sortedPieceStats.end();
        i != eoi; ++i) {
      sortedPieceStatIndexes_[order] = (*i)->getIndex();
      (*i)->setOrder(order++);
    }
  }  
}

class PieceStatRarer {
private:
  const std::vector<SharedHandle<PieceStat> >& pieceStats_;
public:
  PieceStatRarer(const std::vector<SharedHandle<PieceStat> >& ps):
    pieceStats_(ps) {}

  bool operator()(size_t lhs, size_t rhs) const
  {
    return pieceStats_[lhs] < pieceStats_[rhs];
  }
};

void PieceStatMan::addPieceStats(const unsigned char* bitfield,
                                 size_t bitfieldLength)
{
  const size_t nbits = pieceStats_.size();
  assert(nbits <= bitfieldLength*8);
  for(size_t i = 0; i < nbits; ++i) {
    if(bitfield::test(bitfield, nbits, i)) {
      pieceStats_[i]->addCount();
    }
  }
  std::sort(sortedPieceStatIndexes_.begin(), sortedPieceStatIndexes_.end(),
            PieceStatRarer(pieceStats_));
}

void PieceStatMan::subtractPieceStats(const unsigned char* bitfield,
                                      size_t bitfieldLength)
{
  const size_t nbits = pieceStats_.size();
  assert(nbits <= bitfieldLength*8);
  for(size_t i = 0; i < nbits; ++i) {
    if(bitfield::test(bitfield, nbits, i)) {
      pieceStats_[i]->subCount();
    }
  }
  std::sort(sortedPieceStatIndexes_.begin(), sortedPieceStatIndexes_.end(),
            PieceStatRarer(pieceStats_));
}

void PieceStatMan::updatePieceStats(const unsigned char* newBitfield,
                                    size_t newBitfieldLength,
                                    const unsigned char* oldBitfield)
{
  const size_t nbits = pieceStats_.size();
  assert(nbits <= newBitfieldLength*8);
  for(size_t i = 0; i < nbits; ++i) {
    if(bitfield::test(newBitfield, nbits, i) &&
       !bitfield::test(oldBitfield, nbits, i)) {
      pieceStats_[i]->addCount();
    } else if(!bitfield::test(newBitfield, nbits, i) &&
              bitfield::test(oldBitfield, nbits, i)) {
      pieceStats_[i]->subCount();
    }
  }
  std::sort(sortedPieceStatIndexes_.begin(), sortedPieceStatIndexes_.end(),
            PieceStatRarer(pieceStats_));
}

void PieceStatMan::addPieceStats(size_t index)
{
  std::vector<size_t>::iterator cur =
    std::lower_bound(sortedPieceStatIndexes_.begin(),
                     sortedPieceStatIndexes_.end(),
                     index, PieceStatRarer(pieceStats_));

  pieceStats_[index]->addCount();

  std::vector<size_t>::iterator to =
    std::upper_bound(cur+1, sortedPieceStatIndexes_.end(),
                     index, PieceStatRarer(pieceStats_));
  
  std::rotate(cur, cur+1, to);
}

} // namespace aria2
