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
#include "PiecedSegment.h"
#include "Piece.h"

namespace aria2 {

PiecedSegment::PiecedSegment(int32_t pieceLength, const PieceHandle& piece):
  _pieceLength(pieceLength), _overflowLength(0), _piece(piece),
  _writtenLength(_piece->getFirstMissingBlockIndexWithoutLock()*_piece->getBlockLength()) {}

PiecedSegment::~PiecedSegment() {}

bool PiecedSegment::complete() const
{
  return _piece->pieceComplete();
}

int32_t PiecedSegment::getIndex() const
{
  return _piece->getIndex();
}

int64_t PiecedSegment::getPosition() const
{
  return ((int64_t)_piece->getIndex())*_pieceLength;
}

int64_t PiecedSegment::getPositionToWrite() const
{
  return getPosition()+_writtenLength;
}

int32_t PiecedSegment::getLength() const
{
  return _piece->getLength();
}

void PiecedSegment::updateWrittenLength(int32_t bytes)
{
  int32_t newWrittenLength = _writtenLength+bytes;
  if(newWrittenLength > _piece->getLength()) {
    _overflowLength = newWrittenLength-_piece->getLength();
    newWrittenLength = _piece->getLength();
  }
  for(int32_t i = _writtenLength/_piece->getBlockLength(); i < newWrittenLength/_piece->getBlockLength(); ++i) {
    _piece->completeBlock(i);
  }
  if(newWrittenLength == _piece->getLength()) {
    _piece->completeBlock(_piece->countBlock()-1);
  }
  _writtenLength = newWrittenLength;
}

void PiecedSegment::clear()
{
  _writtenLength = 0;
  _overflowLength = 0;
  _piece->clearAllBlock();
}

PieceHandle PiecedSegment::getPiece() const
{
  return _piece;
}

} // namespace aria2
