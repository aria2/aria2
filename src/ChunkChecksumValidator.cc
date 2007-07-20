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
#include "ChunkChecksumValidator.h"
#include "Util.h"
#include "Exception.h"
#include "TimeA2.h"
#include "message.h"

#ifdef ENABLE_MESSAGE_DIGEST
void ChunkChecksumValidator::validateSameLengthChecksum(BitfieldMan* bitfieldMan,
							int32_t index,
							const string& expectedChecksum,
							int32_t dataLength,
							int32_t checksumLength)
{
  int64_t offset = ((int64_t)index)*checksumLength;
  string actualChecksum = diskWriter->messageDigest(offset, dataLength, algo);
  if(actualChecksum != expectedChecksum) {
    logger->info(EX_INVALID_CHUNK_CHECKSUM,
		 index, offset,
		 expectedChecksum.c_str(), actualChecksum.c_str());
    bitfieldMan->unsetBit(index);
  }
}

void ChunkChecksumValidator::validateDifferentLengthChecksum(BitfieldMan* bitfieldMan,
							     int32_t index,
							     const string& expectedChecksum,
							     int32_t dataLength,
							     int32_t checksumLength)
{
  int64_t offset = ((int64_t)index)*checksumLength;
  int32_t startIndex;
  int32_t endIndex;
  Util::indexRange(startIndex, endIndex, offset,
		   checksumLength, bitfieldMan->getBlockLength());
  if(bitfieldMan->isBitRangeSet(startIndex, endIndex)) {
    string actualChecksum = diskWriter->messageDigest(offset, dataLength, algo);
    if(expectedChecksum != actualChecksum) {
      // wrong checksum
      logger->info(EX_INVALID_CHUNK_CHECKSUM,
		   index, offset,
		   expectedChecksum.c_str(), actualChecksum.c_str());
      bitfieldMan->unsetBitRange(startIndex, endIndex);
    }
  }
}

void ChunkChecksumValidator::validate(BitfieldMan* bitfieldMan,
				      const Strings& checksums,
				      int32_t checksumLength)
{
  // We assume file is already opened using DiskWriter::open or openExistingFile.
  if(((int64_t)checksumLength*checksums.size()) < bitfieldMan->getTotalLength()) {
    // insufficient checksums.
    logger->error(MSG_INSUFFICIENT_CHECKSUM,
		  checksumLength, checksums.size());
    return;
  }
  assert(bitfieldMan->getTotalLength()/checksumLength <= INT32_MAX);
  int32_t x = bitfieldMan->getTotalLength()/checksumLength;
  int32_t r = bitfieldMan->getTotalLength()%checksumLength;
  void (ChunkChecksumValidator::*f)(BitfieldMan*, int32_t, const string&, int32_t, int32_t);

  if(checksumLength == bitfieldMan->getBlockLength()) {
    f = &ChunkChecksumValidator::validateSameLengthChecksum;
  } else {
    f = &ChunkChecksumValidator::validateDifferentLengthChecksum;
  }

  fileAllocationMonitor->setMinValue(0);
  fileAllocationMonitor->setMaxValue(bitfieldMan->getTotalLength());
  fileAllocationMonitor->setCurrentValue(0);
  fileAllocationMonitor->showProgress();
  Time cp;
  for(int32_t i = 0; i < x; ++i) {
    (this->*f)(bitfieldMan, i, checksums[i], checksumLength, checksumLength);
    if(cp.elapsedInMillis(500)) {
      fileAllocationMonitor->setCurrentValue(i*checksumLength);
      fileAllocationMonitor->showProgress();
      cp.reset();
    }
  }
  if(r) {
    (this->*f)(bitfieldMan, x, checksums[x], r, checksumLength);
  }
  fileAllocationMonitor->setCurrentValue(bitfieldMan->getTotalLength());
  fileAllocationMonitor->showProgress();
}
#endif // ENABLE_MESSAGE_DIGEST
