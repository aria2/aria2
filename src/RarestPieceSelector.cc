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
#include "RarestPieceSelector.h"

#include <limits>
#include <algorithm>

#include "PieceStatMan.h"
#include "bitfield.h"

namespace aria2 {

RarestPieceSelector::RarestPieceSelector
(const SharedHandle<PieceStatMan>& pieceStatMan):pieceStatMan_(pieceStatMan) {}

bool RarestPieceSelector::select
(size_t& index, const unsigned char* bitfield, size_t nbits) const
{
  const std::vector<size_t>& order = pieceStatMan_->getOrder();
  const std::vector<int>& counts = pieceStatMan_->getCounts();
  int min = std::numeric_limits<int>::max();
  size_t bestIdx = nbits;
  for(size_t i = 0; i < nbits; ++i) {
    size_t idx = order[i];
    if(bitfield::test(bitfield, nbits, idx) && counts[idx] < min) {
      min = counts[idx];
      bestIdx = idx;
    }
  }
  if(bestIdx == nbits) {
    return false;
  } else {
    index = bestIdx;
    return true;
  }
}

} // namespace aria2
