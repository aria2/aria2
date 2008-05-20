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
#ifndef _D_CHUNK_CHECKSUM_H_
#define _D_CHUNK_CHECKSUM_H_

#include "common.h"
#include "SharedHandle.h"
#include "A2STR.h"
#include <string>
#include <deque>

namespace aria2 {

class ChunkChecksum {
private:
  std::string _algo;
  std::deque<std::string> _checksums;
  size_t _checksumLength;
public:
  ChunkChecksum():_checksumLength(0) {}    

  ChunkChecksum(const std::string& algo,
		const std::deque<std::string>& checksums,
		size_t checksumLength):
    _algo(algo),
    _checksums(checksums),
    _checksumLength(checksumLength) {}

  bool validateChunk(const std::string& actualChecksum,
		     size_t checksumIndex) const
  {
    if(checksumIndex < _checksums.size()) {
      return actualChecksum == getChecksum(checksumIndex);
    } else {
      return false;
    }
  }

  uint64_t getEstimatedDataLength() const
  {
    return ((uint64_t)_checksumLength)*_checksums.size();
  }

  size_t countChecksum() const
  {
    return _checksums.size();
  }

  const std::string& getChecksum(size_t index) const
  {
    if(index < _checksums.size()) {
      return _checksums[index];
    } else {
      return A2STR::NIL;
    }
  }
  
  const std::deque<std::string>& getChecksums() const
  {
    return _checksums;
  }

  size_t getChecksumLength() const
  {
    return _checksumLength;
  }

  const std::string& getAlgo() const
  {
    return _algo;
  }

  void setAlgo(const std::string& algo)
  {
    _algo = algo;
  }

  void setChecksumLength(size_t length)
  {
    _checksumLength = length;
  }

  void setChecksums(const std::deque<std::string>& mds)
  {
    _checksums = mds;
  }
};

} // namespace aria2

#endif // _D_CHUNK_CHECKSUM_H_
