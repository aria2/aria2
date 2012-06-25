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
#ifndef D_CHUNK_CHECKSUM_H
#define D_CHUNK_CHECKSUM_H

#include "common.h"

#include <string>
#include <vector>

#include "SharedHandle.h"

namespace aria2 {

class ChunkChecksum {
private:
  std::string hashType_;
  std::vector<std::string> pieceHashes_;
  int32_t pieceLength_;
public:
  ChunkChecksum();

  ChunkChecksum
  (const std::string& hashType,
   const std::vector<std::string>& pieceHashes,
   int32_t pieceLength);

  bool validateChunk(const std::string& actualDigest,
                     size_t index) const;

  int64_t getEstimatedDataLength() const;

  size_t countPieceHash() const;

  const std::string& getPieceHash(size_t index) const;
  
  void setPieceHashes(const std::vector<std::string>& pieceHashes);
  const std::vector<std::string>& getPieceHashes() const
  {
    return pieceHashes_;
  }

  void setHashType(const std::string& hashType);
  const std::string& getHashType() const
  {
    return hashType_;
  }

  int32_t getPieceLength() const
  {
    return pieceLength_;
  }

  void setPieceLength(int32_t length)
  {
    pieceLength_ = length;
  }
};

} // namespace aria2

#endif // D_CHUNK_CHECKSUM_H
