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
#include "Util.h"
#include "message.h"
#include "prefs.h"
#include "PiecedSegment.h"
#include "GrowSegment.h"
#include "LogFactory.h"
#include "Logger.h"
#include "PieceStorage.h"
#include "PeerStat.h"
#include "Option.h"
#include "DownloadContext.h"
#include "Piece.h"

namespace aria2 {

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

uint64_t SegmentMan::getTotalLength() const
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
  unsigned int speed = peerStat->getAvgDownloadSpeed()*0.8;
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
    unsigned int pSpeed = p->calculateDownloadSpeed(); 
    if(pSpeed < speed) {
      speed = pSpeed;
      slowSegmentEntry = segmentEntry;
    }
  }
  return slowSegmentEntry;
}

Segments SegmentMan::getInFlightSegment(int32_t cuid)
{
  Segments temp;
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == cuid) {
      temp.push_back(segmentEntry->segment);
    }
  }
  return temp;
}

SegmentHandle SegmentMan::getSegment(int32_t cuid) {
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

SegmentHandle SegmentMan::getSegment(int32_t cuid, size_t index) {
  if(_downloadContext->getNumPieces() <= index) {
    return 0;
  }
  return checkoutSegment(cuid, _pieceStorage->getMissingPiece(index));
}

void SegmentMan::cancelSegment(int32_t cuid) {
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end();) {
    if((*itr)->cuid == cuid) {
      _pieceStorage->cancelPiece((*itr)->segment->getPiece());
      itr = usedSegmentEntries.erase(itr);
    } else {
      ++itr;
    }
  }
}

class FindSegmentEntry {
private:
  SegmentHandle _segment;
public:
  FindSegmentEntry(const SegmentHandle& segment):_segment(segment) {}

  bool operator()(const SegmentEntryHandle& segmentEntry) const
  {
    return segmentEntry->segment->getIndex() == _segment->getIndex();
  }
};

bool SegmentMan::completeSegment(int32_t cuid, const SegmentHandle& segment) {
  _pieceStorage->completePiece(segment->getPiece());
  _pieceStorage->advertisePiece(cuid, segment->getPiece()->getIndex());
  SegmentEntries::iterator itr = std::find_if(usedSegmentEntries.begin(),
					      usedSegmentEntries.end(),
					      FindSegmentEntry(segment));
  if(itr == usedSegmentEntries.end()) {
    return false;
  } else {
    usedSegmentEntries.erase(itr);
    return true;
  }
}

bool SegmentMan::hasSegment(size_t index) const {
  return _pieceStorage->hasPiece(index);
}

uint64_t SegmentMan::getDownloadLength() const {
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
  for(std::deque<SharedHandle<PeerStat> >::const_iterator itr = peerStats.begin(); itr != peerStats.end(); ++itr) {
    const PeerStatHandle& peerStat = *itr;
    if(peerStat->getCuid() == cuid) {
      return peerStat;
    }
  }
  return 0;
}

unsigned int SegmentMan::calculateDownloadSpeed() const {
  unsigned int speed = 0;
  for(std::deque<SharedHandle<PeerStat> >::const_iterator itr = peerStats.begin(); itr != peerStats.end(); itr++) {
    const PeerStatHandle& peerStat = *itr;
    if(peerStat->getStatus() == PeerStat::ACTIVE) {
      speed += peerStat->calculateDownloadSpeed();
    }
  }
  return speed;
}

size_t SegmentMan::countFreePieceFrom(size_t index) const
{
  size_t numPieces = _downloadContext->getNumPieces();
  for(size_t i = index; i < numPieces; ++i) {
    if(_pieceStorage->hasPiece(i) || _pieceStorage->isPieceUsed(i)) {
      return i-index;
    }
  }
  return _downloadContext->getNumPieces()-index;
}

} // namespace aria2
