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
#include "PiecedSegment.h"
#include "GrowSegment.h"
#include "DiskAdaptor.h"
#include "LogFactory.h"
#include "PieceStorage.h"
#include "PeerStat.h"
#include "Option.h"
#include "DownloadContext.h"
#include "Piece.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "a2io.h"
#include <errno.h>

SegmentEntry::SegmentEntry(int32_t cuid, const SegmentHandle& segment):
  cuid(cuid), segment(segment) {}

SegmentEntry::~SegmentEntry() {}

SegmentMan::SegmentMan(const Option* option,
		       const DownloadContextHandle& downloadContext,
		       const PieceStorageHandle& pieceStorage):
  _option(option),
  logger(LogFactory::getInstance()),
  _downloadContext(downloadContext),
  _pieceStorage(pieceStorage)
{}

SegmentMan::~SegmentMan() {}

bool SegmentMan::downloadFinished() const
{
  if(_pieceStorage.isNull()) {
    return false;
  } else {
    return _pieceStorage->downloadFinished();
  }
}

void SegmentMan::init()
{
  // TODO Do we have to do something about DownloadContext and PieceStorage here?  
}

int64_t SegmentMan::getTotalLength() const
{
  if(_pieceStorage.isNull()) {
    return 0;
  } else {
      return _pieceStorage->getTotalLength();
  }
}

void SegmentMan::setPieceStorage(const PieceStorageHandle& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

void SegmentMan::setDownloadContext(const DownloadContextHandle& downloadContext)
{
  _downloadContext = downloadContext;
}

SegmentHandle SegmentMan::checkoutSegment(int32_t cuid,
					  const PieceHandle& piece)
{
  if(piece.isNull()) {
    return 0;
  }
  logger->debug("Attach segment#%d to CUID#%d.", piece->getIndex(), cuid);

  SegmentHandle segment = 0;
  if(piece->getLength() == 0) {
    segment = new GrowSegment(piece);
  } else {
    segment = new PiecedSegment(_downloadContext->getPieceLength(), piece);
  }
  SegmentEntryHandle entry = new SegmentEntry(cuid, segment);
  usedSegmentEntries.push_back(entry);

  logger->debug("index=%d, length=%d, segmentLength=%d, writtenLength=%d",
		segment->getIndex(),
		segment->getLength(),
		segment->getSegmentLength(),
		segment->getWrittenLength());
  return segment;
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
       !p->getDownloadStartTime().elapsed(_option->getAsInt(PREF_STARTUP_IDLE_TIME))) {
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
  SegmentEntryHandle segmentEntry = getSegmentEntryByCuid(cuid);
  if(!segmentEntry.isNull()) {
    return segmentEntry->segment;
  }
  PieceHandle piece = _pieceStorage->getMissingPiece();
  if(piece.isNull()) {
    PeerStatHandle myPeerStat = getPeerStat(cuid);
    if(myPeerStat.isNull()) {
      return 0;
    }
    SegmentEntryHandle slowSegmentEntry = findSlowerSegmentEntry(myPeerStat);
    if(slowSegmentEntry.get()) {
      logger->info(MSG_SEGMENT_FORWARDING,
		   slowSegmentEntry->cuid,
		   slowSegmentEntry->segment->getIndex(),
		   cuid);
      PeerStatHandle slowPeerStat = getPeerStat(slowSegmentEntry->cuid);
      slowPeerStat->requestIdle();
      cancelSegment(slowSegmentEntry->cuid);
      return checkoutSegment(cuid, slowSegmentEntry->segment->getPiece());
    } else {
      return 0;
    }
  } else {
    return checkoutSegment(cuid, piece);
  }
}

SegmentHandle SegmentMan::getSegment(int32_t cuid, int32_t index) {
  if(index < 0 || _downloadContext->getNumPieces() <= index) {
    return 0;
  }
  return checkoutSegment(cuid, _pieceStorage->getMissingPiece(index));
}

void SegmentMan::cancelSegment(int32_t cuid) {
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    if((*itr)->cuid == cuid) {
      _pieceStorage->cancelPiece((*itr)->segment->getPiece());
      usedSegmentEntries.erase(itr);
      break;
    }
  }
}

bool SegmentMan::completeSegment(int32_t cuid, const SegmentHandle& segment) {
  _pieceStorage->completePiece(segment->getPiece());
  _pieceStorage->advertisePiece(cuid, segment->getPiece()->getIndex());
  SegmentEntries::iterator itr = getSegmentEntryIteratorByCuid(cuid);
  if(itr == usedSegmentEntries.end()) {
    return false;
  } else {
    usedSegmentEntries.erase(itr);
    return true;
  }
}

bool SegmentMan::hasSegment(int32_t index) const {
  return _pieceStorage->hasPiece(index);
}

int64_t SegmentMan::getDownloadLength() const {
  if(_pieceStorage.isNull()) {
    return 0;
  } else {
    return _pieceStorage->getCompletedLength();
  }
}

void SegmentMan::registerPeerStat(const PeerStatHandle& peerStat) {
  PeerStatHandle temp = getPeerStat(peerStat->getCuid());
  if(!temp.get()) {
    peerStats.push_back(peerStat);
  }
}

PeerStatHandle SegmentMan::getPeerStat(int32_t cuid) const
{
  for(PeerStats::const_iterator itr = peerStats.begin(); itr != peerStats.end(); ++itr) {
    const PeerStatHandle& peerStat = *itr;
    if(peerStat->getCuid() == cuid) {
      return peerStat;
    }
  }
  return 0;
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

void SegmentMan::markAllPiecesDone()
{
  _pieceStorage->markAllPiecesDone();
}

void SegmentMan::markPieceDone(int64_t length)
{
  // TODO implement this function later
  /*
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
  */
}

#ifdef ENABLE_MESSAGE_DIGEST
void SegmentMan::validatePieceHash(const SegmentHandle& segment,
				   const string& expectedPieceHash)
{
  string actualPieceHash =
    MessageDigestHelper::digest("sha1",
				_pieceStorage->getDiskAdaptor(),
				segment->getPosition(),
				segment->getLength());
  if(actualPieceHash == expectedPieceHash) {
    logger->info(MSG_GOOD_CHUNK_CHECKSUM, actualPieceHash.c_str());
  } else {
    _pieceStorage->markPieceMissing(segment->getIndex());
    logger->info(EX_INVALID_CHUNK_CHECKSUM,
		 segment->getIndex(),
		 Util::llitos(segment->getPosition(), true).c_str(),
		 expectedPieceHash.c_str(),
		 actualPieceHash.c_str());
  }
}

/*
bool SegmentMan::isChunkChecksumValidationReady(const ChunkChecksumHandle& chunkChecksum) const {
  return false;
  // TODO fix this
  return !chunkChecksum.isNull() && !_downloadContext.isNull() && totalSize > 0 &&
    chunkChecksum->getEstimatedDataLength() >= totalSize;
}
*/
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_MESSAGE_DIGEST
  /*
void SegmentMan::tryChunkChecksumValidation(const SegmentHandle& segment, const ChunkChecksumHandle& chunkChecksum)
{
  // TODO implement this function later
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
  */
#endif // ENABLE_MESSAGE_DIGEST

SegmentEntryHandle SegmentMan::getSegmentEntryByIndex(int32_t index)
{
  for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->segment->getIndex() == index) {
      return segmentEntry;
    }
  }
  return 0;
}
  
SegmentEntryHandle SegmentMan::getSegmentEntryByCuid(int32_t cuid)
{
  for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == cuid) {
      return segmentEntry;
    }
  }
  return 0;    
}

SegmentEntries::iterator SegmentMan::getSegmentEntryIteratorByCuid(int32_t cuid)
{
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == cuid) {
      return itr;
    }
  }
  return usedSegmentEntries.end();    
}
