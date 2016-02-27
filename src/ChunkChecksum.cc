/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "ChunkChecksum.h"
#include "A2STR.h"

namespace aria2 {

ChunkChecksum::ChunkChecksum() : pieceLength_(0) {}

ChunkChecksum::ChunkChecksum(std::string hashType,
                             std::vector<std::string> pieceHashes,
                             int32_t pieceLength)
    : hashType_(std::move(hashType)),
      pieceHashes_(std::move(pieceHashes)),
      pieceLength_(pieceLength)
{
}

bool ChunkChecksum::validateChunk(const std::string& actualDigest,
                                  size_t index) const
{
  const std::string& digest = getPieceHash(index);
  return !digest.empty() && actualDigest == digest;
}

int64_t ChunkChecksum::getEstimatedDataLength() const
{
  return static_cast<int64_t>(pieceLength_) * pieceHashes_.size();
}

size_t ChunkChecksum::countPieceHash() const { return pieceHashes_.size(); }

const std::string& ChunkChecksum::getPieceHash(size_t index) const
{
  if (index < pieceHashes_.size()) {
    return pieceHashes_[index];
  }
  else {
    return A2STR::NIL;
  }
}

void ChunkChecksum::setHashType(std::string hashType)
{
  hashType_ = std::move(hashType);
}

void ChunkChecksum::setPieceHashes(std::vector<std::string> pieceHashes)
{
  pieceHashes_ = std::move(pieceHashes);
}

} // namespace aria2
