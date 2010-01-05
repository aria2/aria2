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
#include "SegmentMan.h"

#include <cassert>
#include <algorithm>
#include <numeric>

#include "util.h"
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
#include "FileEntry.h"

namespace aria2 {

SegmentEntry::SegmentEntry(cuid_t cuid, const SegmentHandle& segment):
  cuid(cuid), segment(segment) {}

SegmentEntry::~SegmentEntry() {}

SegmentMan::SegmentMan(const Option* option,
                       const SharedHandle<DownloadContext>& downloadContext,
                       const PieceStorageHandle& pieceStorage):
  _option(option),
  logger(LogFactory::getInstance()),
  _downloadContext(downloadContext),
  _pieceStorage(pieceStorage),
  _lastPeerStatDlspdMapUpdated(0),
  _cachedDlspd(0),
  _ignoreBitfield(downloadContext->getPieceLength(),
                  downloadContext->getTotalLength())
{
  _ignoreBitfield.enableFilter();
}

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

void SegmentMan::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  _downloadContext = downloadContext;
}

SegmentHandle SegmentMan::checkoutSegment(cuid_t cuid,
                                          const PieceHandle& piece)
{
  if(piece.isNull()) {
    return SharedHandle<Segment>();
  }
  logger->debug("Attach segment#%d to CUID#%d.", piece->getIndex(), cuid);

  SegmentHandle segment;
  if(piece->getLength() == 0) {
    segment.reset(new GrowSegment(piece));
  } else {
    segment.reset(new PiecedSegment(_downloadContext->getPieceLength(), piece));
  }
  SegmentEntryHandle entry(new SegmentEntry(cuid, segment));
  usedSegmentEntries.push_back(entry);

  logger->debug("index=%d, length=%d, segmentLength=%d, writtenLength=%d",
                segment->getIndex(),
                segment->getLength(),
                segment->getSegmentLength(),
                segment->getWrittenLength());
  if(piece->getLength() > 0) {
    std::map<size_t, size_t>::iterator positr =
      _segmentWrittenLengthMemo.find(segment->getIndex());
    if(positr != _segmentWrittenLengthMemo.end()) {
      const size_t writtenLength = (*positr).second;
      logger->debug("writtenLength(in memo)=%d, writtenLength=%d",
                    writtenLength, segment->getWrittenLength());
      //  If the difference between cached writtenLength and segment's
      //  writtenLength is less than one block, we assume that these
      //  missing bytes are already downloaded.
      if(segment->getWrittenLength() < writtenLength &&
         writtenLength-segment->getWrittenLength() < piece->getBlockLength()) {
        segment->updateWrittenLength(writtenLength-segment->getWrittenLength());
      }
    }
  }
  return segment;
}

void SegmentMan::getInFlightSegment(std::deque<SharedHandle<Segment> >& segments,
                                    cuid_t cuid)
{
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == cuid) {
      segments.push_back(segmentEntry->segment);
    }
  }
}

SegmentHandle SegmentMan::getSegment(cuid_t cuid) {
  PieceHandle piece =
    _pieceStorage->getSparseMissingUnusedPiece
    (_ignoreBitfield.getFilterBitfield(),_ignoreBitfield.getBitfieldLength());
  return checkoutSegment(cuid, piece);
}

void SegmentMan::getSegment(std::deque<SharedHandle<Segment> >& segments,
                            cuid_t cuid,
                            const SharedHandle<FileEntry>& fileEntry,
                            size_t maxSegments)
{
  BitfieldMan filter(_ignoreBitfield);
  filter.enableFilter();
  filter.addNotFilter(fileEntry->getOffset(), fileEntry->getLength());
  std::deque<SharedHandle<Segment> > pending;
  while(segments.size() < maxSegments) {
    SharedHandle<Segment> segment =
      checkoutSegment(cuid,
                      _pieceStorage->getSparseMissingUnusedPiece
                      (filter.getFilterBitfield(), filter.getBitfieldLength()));
    if(segment.isNull()) {
      break;
    }
    if(segment->getPositionToWrite() < fileEntry->getOffset() ||
       fileEntry->getLastOffset() <= segment->getPositionToWrite()) {
      pending.push_back(segment);
    } else {
      segments.push_back(segment);
    }
  }
  for(std::deque<SharedHandle<Segment> >::const_iterator i = pending.begin();
      i != pending.end(); ++i) {
    cancelSegment(cuid, *i);
  }
}

SegmentHandle SegmentMan::getSegment(cuid_t cuid, size_t index) {
  if(_downloadContext->getNumPieces() <= index) {
    return SharedHandle<Segment>();
  }
  return checkoutSegment(cuid, _pieceStorage->getMissingPiece(index));
}

void SegmentMan::cancelSegment(const SharedHandle<Segment>& segment)
{
  _pieceStorage->cancelPiece(segment->getPiece());
  _segmentWrittenLengthMemo[segment->getIndex()] = segment->getWrittenLength();
  logger->debug("Memorized segment index=%u, writtenLength=%u",
                segment->getIndex(), segment->getWrittenLength());
}

void SegmentMan::cancelSegment(cuid_t cuid) {
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end();) {
    if((*itr)->cuid == cuid) {
      cancelSegment((*itr)->segment);
      itr = usedSegmentEntries.erase(itr);
    } else {
      ++itr;
    }
  }
}

void SegmentMan::cancelSegment
(cuid_t cuid, const SharedHandle<Segment>& segment)
{
  for(SegmentEntries::iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end();) {
    if((*itr)->cuid == cuid && (*itr)->segment == segment) {
      cancelSegment((*itr)->segment);
      itr = usedSegmentEntries.erase(itr);
      break;
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

bool SegmentMan::completeSegment(cuid_t cuid, const SegmentHandle& segment) {
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

void SegmentMan::registerPeerStat(const SharedHandle<PeerStat>& peerStat)
{
  for(std::deque<SharedHandle<PeerStat> >::iterator i = peerStats.begin();
      i != peerStats.end(); ++i) {
    if((*i)->getStatus() == PeerStat::IDLE) {
      *i = peerStat;
      return;
    }
  }
  peerStats.push_back(peerStat);
}

class PeerStatHostProtoEqual {
private:
  const SharedHandle<PeerStat>& _peerStat;
public:
  PeerStatHostProtoEqual(const SharedHandle<PeerStat>& peerStat):
    _peerStat(peerStat) {}

  bool operator()(const SharedHandle<PeerStat>& p) const
  {
    return _peerStat->getHostname() == p->getHostname() &&
      _peerStat->getProtocol() == p->getProtocol();
  }
};

void SegmentMan::updateFastestPeerStat(const SharedHandle<PeerStat>& peerStat)
{
  std::deque<SharedHandle<PeerStat> >::iterator i =
    std::find_if(_fastestPeerStats.begin(), _fastestPeerStats.end(),
                 PeerStatHostProtoEqual(peerStat));
  if(i == _fastestPeerStats.end()) {
    _fastestPeerStats.push_back(peerStat);
  } else if((*i)->getAvgDownloadSpeed() < peerStat->getAvgDownloadSpeed()) {
    // *i's SessionDownloadLength must be added to peerStat
    peerStat->addSessionDownloadLength((*i)->getSessionDownloadLength());
    *i = peerStat;
  } else {
    // peerStat's SessionDownloadLength must be added to *i
    (*i)->addSessionDownloadLength(peerStat->getSessionDownloadLength());
  }
}

unsigned int SegmentMan::calculateDownloadSpeed()
{
  unsigned int speed = 0;
  if(_lastPeerStatDlspdMapUpdated.elapsedInMillis(250)) {
    _lastPeerStatDlspdMapUpdated.reset();
    _peerStatDlspdMap.clear();
    for(std::deque<SharedHandle<PeerStat> >::const_iterator i =
          peerStats.begin(); i != peerStats.end(); ++i) {
      if((*i)->getStatus() == PeerStat::ACTIVE) {
        unsigned int s = (*i)->calculateDownloadSpeed();
        _peerStatDlspdMap[(*i)->getCuid()] = s;
        speed += s;
      }
    }
    _cachedDlspd = speed;
  } else {
    speed = _cachedDlspd;
  }
  return speed;
}

void SegmentMan::updateDownloadSpeedFor(const SharedHandle<PeerStat>& pstat)
{
  unsigned int newspd = pstat->calculateDownloadSpeed();
  unsigned int oldSpd = _peerStatDlspdMap[pstat->getCuid()];
  if(_cachedDlspd > oldSpd) {
    _cachedDlspd -= oldSpd;
    _cachedDlspd += newspd;
  } else {
    _cachedDlspd = newspd;
  }
  _peerStatDlspdMap[pstat->getCuid()] = newspd;
}

class PeerStatDownloadLengthOperator {
public:
  uint64_t operator()(uint64_t total, const SharedHandle<PeerStat>& ps)
  {
    return ps->getSessionDownloadLength()+total;
  }
};

uint64_t SegmentMan::calculateSessionDownloadLength() const
{
  return std::accumulate(_fastestPeerStats.begin(), _fastestPeerStats.end(),
                         0LL, PeerStatDownloadLengthOperator());
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

void SegmentMan::ignoreSegmentFor(const SharedHandle<FileEntry>& fileEntry)
{
  _ignoreBitfield.addFilter(fileEntry->getOffset(), fileEntry->getLength());
}

void SegmentMan::recognizeSegmentFor(const SharedHandle<FileEntry>& fileEntry)
{
  _ignoreBitfield.removeFilter(fileEntry->getOffset(), fileEntry->getLength());
}

bool SegmentMan::allSegmentsIgnored() const
{
  return _ignoreBitfield.isAllFilterBitSet();
}

} // namespace aria2
