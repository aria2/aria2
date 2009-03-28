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
#ifndef _D_RAREST_PIECE_SELECTOR_H_
#define _D_RAREST_PIECE_SELECTOR_H_

#include "PieceSelector.h"

#include <vector>

namespace aria2 {

class PieceStat {
private:
  size_t _order;
  size_t _index;
  size_t _count;
public:
  PieceStat(size_t index);

  bool operator<(const PieceStat& pieceStat) const;

  void addCount();
  void subCount();

  size_t getOrder() const;
  void setOrder(size_t order);
  size_t getIndex() const;
  size_t getCount() const;
};

class RarestPieceSelector:public PieceSelector {
private:
  std::vector<SharedHandle<PieceStat> > _pieceStats;

  std::vector<size_t> _sortedPieceStatIndexes;
public:
  RarestPieceSelector(size_t pieceNum, bool randomShuffle);

  virtual bool select
  (size_t& index, const unsigned char* bitfield, size_t nbits) const;

  virtual void addPieceStats(size_t index);

  virtual void addPieceStats(const unsigned char* bitfield,
			     size_t bitfieldLength);
  
  virtual void subtractPieceStats(const unsigned char* bitfield,
				  size_t bitfieldLength);

  virtual void updatePieceStats(const unsigned char* newBitfield,
				size_t newBitfieldLength,
				const unsigned char* oldBitfield);

  const std::vector<size_t>& getSortedPieceStatIndexes() const;

  const std::vector<SharedHandle<PieceStat> >& getPieceStats() const;
};

} // namespace aria2

#endif // _D_RAREST_PIECE_SELECTOR_H_

