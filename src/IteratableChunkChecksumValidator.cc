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
#include "IteratableChunkChecksumValidator.h"
#include "Util.h"
#include "message.h"

void IteratableChunkChecksumValidator::validateChunk()
{
  if(!finished()) {
    string actualChecksum = calculateActualChecksum();


    if(!_chunkChecksum->validateChunk(actualChecksum, _currentIndex)) {
      int64_t offset = ((int64_t)_currentIndex)*_chunkChecksum->getChecksumLength();
      // wrong checksum
      logger->info(EX_INVALID_CHUNK_CHECKSUM,
		   _currentIndex,
		   Util::llitos(offset, true).c_str(),
		   _chunkChecksum->getChecksum(_currentIndex).c_str(),
		   actualChecksum.c_str());
      int32_t startIndex;
      int32_t endIndex;
      Util::indexRange(startIndex, endIndex, offset,
		       _chunkChecksum->getChecksumLength(),
		       _bitfield->getBlockLength());
      _bitfield->unsetBitRange(startIndex, endIndex);
    }
    _currentIndex++;	  
  }
}

string IteratableChunkChecksumValidator::calculateActualChecksum()
{
  int64_t offset = ((int64_t)_currentIndex)*_chunkChecksum->getChecksumLength();
  int32_t length = _diskWriter->size() < offset+_chunkChecksum->getChecksumLength() ? _diskWriter->size()-offset : _chunkChecksum->getChecksumLength();
  return _diskWriter->messageDigest(offset, length, _chunkChecksum->getAlgo());
}

bool IteratableChunkChecksumValidator::canValidate() const
{
  // We assume file is already opened using DiskWriter::open or openExistingFile.
  return _chunkChecksum->getEstimatedDataLength() >= _diskWriter->size();
}

void IteratableChunkChecksumValidator::init()
{
  _bitfield->setAllBit();
  _currentIndex = 0;
}
