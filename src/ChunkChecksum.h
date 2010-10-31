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
#include "A2STR.h"

namespace aria2 {

class ChunkChecksum {
private:
  std::string algo_;
  std::vector<std::string> checksums_;
  size_t checksumLength_;
public:
  ChunkChecksum():checksumLength_(0) {}    

  ChunkChecksum(const std::string& algo,
                const std::vector<std::string>& checksums,
                size_t checksumLength):
    algo_(algo),
    checksums_(checksums),
    checksumLength_(checksumLength) {}

  bool validateChunk(const std::string& actualChecksum,
                     size_t checksumIndex) const
  {
    if(checksumIndex < checksums_.size()) {
      return actualChecksum == getChecksum(checksumIndex);
    } else {
      return false;
    }
  }

  uint64_t getEstimatedDataLength() const
  {
    return ((uint64_t)checksumLength_)*checksums_.size();
  }

  size_t countChecksum() const
  {
    return checksums_.size();
  }

  const std::string& getChecksum(size_t index) const
  {
    if(index < checksums_.size()) {
      return checksums_[index];
    } else {
      return A2STR::NIL;
    }
  }
  
  const std::vector<std::string>& getChecksums() const
  {
    return checksums_;
  }

  size_t getChecksumLength() const
  {
    return checksumLength_;
  }

  const std::string& getAlgo() const
  {
    return algo_;
  }

  void setAlgo(const std::string& algo)
  {
    algo_ = algo;
  }

  void setChecksumLength(size_t length)
  {
    checksumLength_ = length;
  }

  void setChecksums(const std::vector<std::string>& mds)
  {
    checksums_ = mds;
  }
};

} // namespace aria2

#endif // D_CHUNK_CHECKSUM_H
