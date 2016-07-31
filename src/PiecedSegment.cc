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
#include "PiecedSegment.h"
#include "Piece.h"
#include <cassert>

namespace aria2 {

PiecedSegment::PiecedSegment(int32_t pieceLength,
                             const std::shared_ptr<Piece>& piece)
    : piece_(piece), pieceLength_(pieceLength)
{
  size_t index;
  bool t = piece_->getFirstMissingBlockIndexWithoutLock(index);
  assert(t);
  writtenLength_ = index * piece_->getBlockLength();
}

PiecedSegment::~PiecedSegment() = default;

bool PiecedSegment::complete() const { return piece_->pieceComplete(); }

size_t PiecedSegment::getIndex() const { return piece_->getIndex(); }

int64_t PiecedSegment::getPosition() const
{
  return ((int64_t)piece_->getIndex()) * pieceLength_;
}

int64_t PiecedSegment::getPositionToWrite() const
{
  return getPosition() + writtenLength_;
}

int64_t PiecedSegment::getLength() const { return piece_->getLength(); }

void PiecedSegment::updateWrittenLength(int64_t bytes)
{
  auto newWrittenLength = writtenLength_ + bytes;
  assert(newWrittenLength <= piece_->getLength());
  for (auto i = writtenLength_ / piece_->getBlockLength(),
            end = newWrittenLength / piece_->getBlockLength();
       i < end; ++i) {
    piece_->completeBlock(i);
  }
  if (newWrittenLength == piece_->getLength()) {
    piece_->completeBlock(piece_->countBlock() - 1);
  }
  writtenLength_ = newWrittenLength;
}

bool PiecedSegment::updateHash(int64_t begin, const unsigned char* data,
                               size_t dataLength)
{
  return piece_->updateHash(begin, data, dataLength);
}

bool PiecedSegment::isHashCalculated() const
{
  return piece_->isHashCalculated();
}

std::string PiecedSegment::getDigest() { return piece_->getDigest(); }

void PiecedSegment::clear(WrDiskCache* diskCache)
{
  writtenLength_ = 0;
  piece_->clearAllBlock(diskCache);

  piece_->destroyHashContext();
}

std::shared_ptr<Piece> PiecedSegment::getPiece() const { return piece_; }

} // namespace aria2
