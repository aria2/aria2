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
#include "SegmentMan.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "File.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"
#include "BitfieldManFactory.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "a2io.h"
#include <errno.h>

SegmentMan::SegmentMan():logger(LogFactory::getInstance()),
			 bitfield(0),
			 totalSize(0),
			 isSplittable(true), 
			 downloadStarted(false),
			 dir("."),
			 errors(0),
			 diskWriter(0)
{}

SegmentMan::~SegmentMan() {
  delete bitfield;
}

bool SegmentMan::segmentFileExists() const {
  if(!isSplittable) {
    return false;
  }
  string segFilename = getSegmentFilePath();
  File f(segFilename);
  if(f.isFile()) {
    logger->info(MSG_SEGMENT_FILE_EXISTS, segFilename.c_str());
    return true;
  } else {
    logger->info(MSG_SEGMENT_FILE_DOES_NOT_EXIST, segFilename.c_str());
    return false;
  }
}

void SegmentMan::load() {
  if(!isSplittable) {
    return;
  }
  string segFilename = getSegmentFilePath();
  logger->info(MSG_LOADING_SEGMENT_FILE, segFilename.c_str());
  FILE* segFile = openSegFile(segFilename, "r+b");
  try {
    read(segFile);
    fclose(segFile);
  } catch(string ex) {
    fclose(segFile);
    throw new DlAbortEx(EX_SEGMENT_FILE_READ,
			segFilename.c_str(), strerror(errno));
  }
  logger->info(MSG_LOADED_SEGMENT_FILE);
}

void SegmentMan::save() const {
  if(!isSplittable || totalSize == 0 || !bitfield) {
    return;
  }
  string segFilename = getSegmentFilePath();
  logger->info(MSG_SAVING_SEGMENT_FILE, segFilename.c_str());

  string segFilenameTemp = segFilename+"__temp";
  FILE* segFile = openSegFile(segFilenameTemp, "wb");
  try {
    if(fwrite(&totalSize, sizeof(totalSize), 1, segFile) < 1) {
      throw string("writeError");
    }
    int32_t segmentLength = bitfield->getBlockLength();
    if(fwrite(&segmentLength, sizeof(segmentLength), 1, segFile) < 1) {
      throw string("writeError");
    }
    if(bitfield) {
      int32_t bitfieldLength = bitfield->getBitfieldLength();
      if(fwrite(&bitfieldLength, sizeof(bitfieldLength), 1, segFile) < 1) {
	throw string("writeError");
      }
      if(fwrite(bitfield->getBitfield(), bitfield->getBitfieldLength(),
		1, segFile) < 1) {
	throw string("writeError");
      }					 
    } else {
      int32_t i = 0;
      if(fwrite(&i, sizeof(i), 1, segFile) < 1) {
	throw string("writeError");
      }
    }
    int32_t usedSegmentCount = usedSegmentEntries.size();
    if(fwrite(&usedSegmentCount, sizeof(usedSegmentCount), 1, segFile) < 1) {
      throw string("writeError");
    }
    for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
	itr != usedSegmentEntries.end(); itr++) {
      if(fwrite((*itr)->segment.get(), sizeof(Segment), 1, segFile) < 1) {
	throw string("writeError");
      }
    }
    fclose(segFile);
    logger->info(MSG_SAVED_SEGMENT_FILE);
  } catch(string ex) {
    fclose(segFile);
    throw new DlAbortEx(EX_SEGMENT_FILE_WRITE,
			segFilename.c_str(), strerror(errno));
  }

  if(!File(segFilenameTemp).renameTo(segFilename)) {
    throw new DlAbortEx(EX_SEGMENT_FILE_WRITE,
			segFilename.c_str(), strerror(errno));
  }
}

FILE* SegmentMan::openSegFile(const string& segFilename, const string& mode) const {
  FILE* segFile = fopen(segFilename.c_str(), mode.c_str());
  if(segFile == NULL) {
    throw new DlAbortEx(EX_SEGMENT_FILE_OPEN,
			segFilename.c_str(), strerror(errno));
  }
  return segFile;
}

void SegmentMan::read(FILE* file) {
  assert(file != NULL);
  if(fread(&totalSize, sizeof(totalSize), 1, file) < 1) {
    throw string("readError");
  }
  int32_t segmentSize;
  if(fread(&segmentSize, sizeof(segmentSize), 1, file) < 1) {
    throw string("readError");
  }
  int32_t bitfieldLength;
  if(fread(&bitfieldLength, sizeof(bitfieldLength), 1, file) < 1) {
    throw string("readError");
  }
  if(bitfieldLength > 0) {
    initBitfield(segmentSize, totalSize);
    unsigned char* savedBitfield = new unsigned char[bitfield->getBitfieldLength()];
    if(fread(savedBitfield, bitfield->getBitfieldLength(), 1, file) < 1) {
      delete [] savedBitfield;
      throw string("readError");
    } else {
      bitfield->setBitfield(savedBitfield, bitfield->getBitfieldLength());
      delete [] savedBitfield;
    }
  }
  int32_t segmentCount;
  if(fread(&segmentCount, sizeof(segmentCount), 1, file) < 1) {
    throw string("readError");
  }
  while(segmentCount--) {
    SegmentHandle seg;
    if(fread(seg.get(), sizeof(Segment), 1, file) < 1) {
      throw string("readError");
    }
    usedSegmentEntries.push_back(SegmentEntryHandle(new SegmentEntry(0, seg)));
  }
}

void SegmentMan::remove() const {
  if(!isSplittable) {
    return;
  }
  if(segmentFileExists()) {
    File f(getSegmentFilePath());
    f.remove();
  }
}

bool SegmentMan::finished() const {
  if(!downloadStarted) {
    return false;
  }
  if(!bitfield) {
    return false;
  }
  assert(bitfield);
  return bitfield->isAllBitSet();
}

void SegmentMan::removeIfFinished() const {
  if(finished()) {
    remove();
  }
}

void SegmentMan::init() {
  totalSize = 0;
  isSplittable = false;
  downloadStarted = false;
  errors = 0;
  //segments.clear();
  usedSegmentEntries.clear();
  delete bitfield;
  bitfield = 0;
  peerStats.clear();
  diskWriter->closeFile();
  
}

void SegmentMan::initBitfield(int32_t segmentLength, int64_t totalLength) {
  delete bitfield;
  this->bitfield = BitfieldManFactory::getFactoryInstance()->createBitfieldMan(segmentLength, totalLength);
}

SegmentHandle SegmentMan::checkoutSegment(int32_t cuid, int32_t index) {
  logger->debug("Attach segment#%d to CUID#%d.", index, cuid);
  bitfield->setUseBit(index);
  SegmentEntryHandle segmentEntry = getSegmentEntryByIndex(index);
  SegmentHandle segment(0);
  if(segmentEntry.isNull()) {
    segment = new Segment(index, bitfield->getBlockLength(index),
			  bitfield->getBlockLength());
    SegmentEntryHandle entry = new SegmentEntry(cuid, segment);
    usedSegmentEntries.push_back(entry);
  } else {
    segmentEntry->cuid = cuid;
    segment = segmentEntry->segment;
  }
  logger->debug("index=%d, length=%d, segmentLength=%d, writtenLength=%d",
		segment->index, segment->length, segment->segmentLength,
		segment->writtenLength);
  return segment;
}

SegmentHandle SegmentMan::onNullBitfield(int32_t cuid) {
  if(usedSegmentEntries.size() == 0) {
    SegmentHandle segment = new Segment(0, 0, 0);
    usedSegmentEntries.push_back(SegmentEntryHandle(new SegmentEntry(cuid, segment)));
    return segment;
  } else {
    SegmentEntryHandle segmentEntry = getSegmentEntryByCuid(cuid);
    if(segmentEntry.isNull()) {
      return 0;
    } else {
      return segmentEntry->segment;
    }
  }
}

SegmentEntryHandle SegmentMan::findSlowerSegmentEntry(const PeerStatHandle& peerStat) const {
  int32_t speed = (int32_t)(peerStat->getAvgDownloadSpeed()*0.8);
  SegmentEntryHandle slowSegmentEntry(0);
  for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == 0) {
      continue;
    }
    PeerStatHandle p = getPeerStat(segmentEntry->cuid);
    if(!p.get() || p->getCuid() == peerStat->getCuid() ||
       p->getStatus() != PeerStat::ACTIVE ||
       !p->getDownloadStartTime().elapsed(option->getAsInt(PREF_STARTUP_IDLE_TIME))) {
      continue;
    }
    int32_t pSpeed = p->calculateDownloadSpeed(); 
    if(pSpeed < speed) {
      speed = pSpeed;
      slowSegmentEntry = segmentEntry;
    }
  }
  return slowSegmentEntry;
}

SegmentHandle SegmentMan::getSegment(int32_t cuid) {
  if(!bitfield) {
    return onNullBitfield(cuid);
  }
  SegmentEntryHandle segmentEntry = getSegmentEntryByCuid(cuid);
  if(!segmentEntry.isNull()) {
    return segmentEntry->segment;
  }
  int32_t index = bitfield->getSparseMissingUnusedIndex();
  if(index == -1) {
    PeerStatHandle myPeerStat = getPeerStat(cuid);
    if(!myPeerStat.get()) {
      return 0;
    }
    SegmentEntryHandle slowSegmentEntry = findSlowerSegmentEntry(myPeerStat);
    if(slowSegmentEntry.get()) {
      logger->info(MSG_SEGMENT_FORWARDING,
		   slowSegmentEntry->cuid,
		   slowSegmentEntry->segment->index,
		   cuid);
      PeerStatHandle slowPeerStat = getPeerStat(slowSegmentEntry->cuid);
      slowPeerStat->requestIdle();
      cancelSegment(slowSegmentEntry->cuid);
      return checkoutSegment(cuid, slowSegmentEntry->segment->index);
    } else {
      return 0;
    }
  } else {
    return checkoutSegment(cuid, index);
  }
}

SegmentHandle SegmentMan::getSegment(int32_t cuid, int32_t index) {
  if(!bitfield) {
    return onNullBitfield(cuid);
  }
  if(index < 0 || (int32_t)bitfield->countBlock() <= index) {
    return 0;
  }
  if(bitfield->isBitSet(index) || bitfield->isUseBitSet(index)) {
    return 0;
  } else {
    return checkoutSegment(cuid, index);
  }
}
/*
bool SegmentMan::updateSegment(int cuid, const Segment& segment) {
  if(segment.isNull()) {
    return false;
  }
  SegmentEntryHandle segmentEntry = getSegmentEntryByCuid(cuid);
  if(segmentEntry.isNull()) {
    return false;
  } else {
    segmentEntry->segment = segment;
    return true;
  }
}
*/

void SegmentMan::cancelSegment(int32_t cuid) {
  if(bitfield) {
    for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
	itr != usedSegmentEntries.end(); ++itr) {
      if((*itr)->cuid == cuid) {
	bitfield->unsetUseBit((*itr)->segment->index);
	(*itr)->cuid = 0;
	break;
      }
    }
  } else {
    usedSegmentEntries.clear();
  }
}

bool SegmentMan::completeSegment(int32_t cuid, const SegmentHandle& segment) {
  if(segment->isNull()) {
    return false;
  }
  if(bitfield) {
    bitfield->unsetUseBit(segment->index);
    bitfield->setBit(segment->index);
  } else {
    initBitfield(option->getAsInt(PREF_SEGMENT_SIZE), segment->writtenLength);
    bitfield->setAllBit();
  }
  SegmentEntries::iterator itr = getSegmentEntryIteratorByCuid(cuid);
  if(itr == usedSegmentEntries.end()) {
    return false;
  } else {
    usedSegmentEntries.erase(itr);
    return true;
  }
}

bool SegmentMan::hasSegment(int32_t index) const {
  if(bitfield) {
    return bitfield->isBitSet(index);
  } else {
    return false;
  }
}

int64_t SegmentMan::getDownloadLength() const {
  int64_t dlLength = 0;
  if(bitfield) {
    dlLength += bitfield->getCompletedLength();
  }
  for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); itr++) {
    dlLength += (*itr)->segment->writtenLength;
  }
  return dlLength;
}

void SegmentMan::registerPeerStat(const PeerStatHandle& peerStat) {
  PeerStatHandle temp = getPeerStat(peerStat->getCuid());
  if(!temp.get()) {
    peerStats.push_back(peerStat);
  }
}

int32_t SegmentMan::calculateDownloadSpeed() const {
  int32_t speed = 0;
  for(PeerStats::const_iterator itr = peerStats.begin();
      itr != peerStats.end(); itr++) {
    const PeerStatHandle& peerStat = *itr;
    if(peerStat->getStatus() == PeerStat::ACTIVE) {
      speed += peerStat->calculateDownloadSpeed();
    }
  }
  return speed;
}

bool SegmentMan::fileExists() const {
  return File(getFilePath()).exists();
}

void SegmentMan::markAllPiecesDone()
{
  if(bitfield) {
    bitfield->setAllBit();
  }
}

void SegmentMan::markPieceDone(int64_t length)
{
  if(bitfield) {
    if(length == bitfield->getTotalLength()) {
      bitfield->setAllBit();
    } else {
      bitfield->clearAllBit();
      int32_t numSegment = length/bitfield->getBlockLength();
      int32_t remainingLength = length%bitfield->getBlockLength();
      bitfield->setBitRange(0, numSegment-1);
      if(remainingLength > 0) {
	SegmentHandle segment = new Segment();
	segment->index = numSegment;
	segment->length = bitfield->getBlockLength(numSegment);
	segment->segmentLength = bitfield->getBlockLength();
	segment->writtenLength = remainingLength;
	usedSegmentEntries.push_back(new SegmentEntry(0, segment));
      }
    }
  }
}

#ifdef ENABLE_MESSAGE_DIGEST
bool SegmentMan::isChunkChecksumValidationReady(const ChunkChecksumHandle& chunkChecksum) const {
  return !chunkChecksum.isNull() && bitfield && totalSize > 0 &&
    chunkChecksum->getEstimatedDataLength() >= totalSize;
}
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_MESSAGE_DIGEST
void SegmentMan::tryChunkChecksumValidation(const SegmentHandle& segment, const ChunkChecksumHandle& chunkChecksum)
{
  if(!isChunkChecksumValidationReady(chunkChecksum)) {
    return;
  }
  int32_t hashStartIndex;
  int32_t hashEndIndex;
  Util::indexRange(hashStartIndex, hashEndIndex,
		   segment->getPosition(),
		   segment->writtenLength,
		   chunkChecksum->getChecksumLength());
  if(!bitfield->isBitSetOffsetRange((int64_t)hashStartIndex*chunkChecksum->getChecksumLength(),
				    chunkChecksum->getChecksumLength())) {
    ++hashStartIndex;
  }
  if(!bitfield->isBitSetOffsetRange((int64_t)hashEndIndex*chunkChecksum->getChecksumLength(),
				    chunkChecksum->getChecksumLength())) {
    --hashEndIndex;
  }
  logger->debug("hashStartIndex=%d, hashEndIndex=%d",
		hashStartIndex, hashEndIndex);
  if(hashStartIndex > hashEndIndex) {
    logger->debug(MSG_NO_CHUNK_CHECKSUM);
    return;
  }
  int64_t hashOffset = ((int64_t)hashStartIndex)*chunkChecksum->getChecksumLength();
  int32_t startIndex;
  int32_t endIndex;
  Util::indexRange(startIndex, endIndex,
		   hashOffset,
		   (hashEndIndex-hashStartIndex+1)*chunkChecksum->getChecksumLength(),
		   bitfield->getBlockLength());
  logger->debug("startIndex=%d, endIndex=%d", startIndex, endIndex);
  if(bitfield->isBitRangeSet(startIndex, endIndex)) {
    for(int32_t index = hashStartIndex; index <= hashEndIndex; ++index) {
      int64_t offset = ((int64_t)index)*chunkChecksum->getChecksumLength();
      int32_t dataLength =
	offset+chunkChecksum->getChecksumLength() <= totalSize ?
	chunkChecksum->getChecksumLength() : totalSize-offset;
      string actualChecksum = MessageDigestHelper::digest(chunkChecksum->getAlgo(), diskWriter, offset, dataLength);
      if(chunkChecksum->validateChunk(actualChecksum, index)) {
	logger->info(MSG_GOOD_CHUNK_CHECKSUM, actualChecksum.c_str());
      } else {
	logger->info(EX_INVALID_CHUNK_CHECKSUM,
		     index, Util::llitos(offset, true).c_str(),
		     chunkChecksum->getChecksum(index).c_str(), actualChecksum.c_str());
	logger->debug("Unset bit from %d to %d(inclusive)", startIndex, endIndex);
	bitfield->unsetBitRange(startIndex, endIndex);
	break;
      }
    }
  }
}
#endif // ENABLE_MESSAGE_DIGEST
