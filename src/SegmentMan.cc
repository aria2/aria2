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
#include "wallclock.h"
#include "fmt.h"

namespace aria2 {

SegmentEntry::SegmentEntry(cuid_t cuid, const SharedHandle<Segment>& segment)
  : cuid(cuid), segment(segment)
{}

SegmentEntry::~SegmentEntry() {}

SegmentMan::SegmentMan
(const Option* option,
 const SharedHandle<DownloadContext>& downloadContext,
 const PieceStorageHandle& pieceStorage)
  : option_(option),
    downloadContext_(downloadContext),
    pieceStorage_(pieceStorage),
    lastPeerStatDlspdMapUpdated_(0),
    cachedDlspd_(0),
    ignoreBitfield_(downloadContext->getPieceLength(),
                    downloadContext->getTotalLength())
{
  ignoreBitfield_.enableFilter();
}

SegmentMan::~SegmentMan() {}

bool SegmentMan::downloadFinished() const
{
  if(!pieceStorage_) {
    return false;
  } else {
    return pieceStorage_->downloadFinished();
  }
}

void SegmentMan::init()
{
  // TODO Do we have to do something about DownloadContext and
  // PieceStorage here?
}

uint64_t SegmentMan::getTotalLength() const
{
  if(!pieceStorage_) {
    return 0;
  } else {
    return pieceStorage_->getTotalLength();
  }
}

void SegmentMan::setPieceStorage(const PieceStorageHandle& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void SegmentMan::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  downloadContext_ = downloadContext;
}

SharedHandle<Segment> SegmentMan::checkoutSegment
(cuid_t cuid, const SharedHandle<Piece>& piece)
{
  if(!piece) {
    return SharedHandle<Segment>();
  }
  A2_LOG_DEBUG(fmt("Attach segment#%lu to CUID#%lld.",
                   static_cast<unsigned long>(piece->getIndex()),
                   cuid));
  SharedHandle<Segment> segment;
  if(piece->getLength() == 0) {
    segment.reset(new GrowSegment(piece));
  } else {
    segment.reset(new PiecedSegment(downloadContext_->getPieceLength(), piece));
  }
  SegmentEntryHandle entry(new SegmentEntry(cuid, segment));
  usedSegmentEntries_.push_back(entry);
  A2_LOG_DEBUG(fmt("index=%lu, length=%lu, segmentLength=%lu,"
                   " writtenLength=%lu",
                   static_cast<unsigned long>(segment->getIndex()),
                   static_cast<unsigned long>(segment->getLength()),
                   static_cast<unsigned long>(segment->getSegmentLength()),
                   static_cast<unsigned long>(segment->getWrittenLength())));
  if(piece->getLength() > 0) {
    std::map<size_t, size_t>::iterator positr =
      segmentWrittenLengthMemo_.find(segment->getIndex());
    if(positr != segmentWrittenLengthMemo_.end()) {
      const size_t writtenLength = (*positr).second;
      A2_LOG_DEBUG(fmt("writtenLength(in memo)=%lu, writtenLength=%lu",
                       static_cast<unsigned long>(writtenLength),
                       static_cast<unsigned long>(segment->getWrittenLength()))
                   );
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

void SegmentMan::getInFlightSegment
(std::vector<SharedHandle<Segment> >& segments, cuid_t cuid)
{
  for(SegmentEntries::const_iterator itr = usedSegmentEntries_.begin(),
        eoi = usedSegmentEntries_.end(); itr != eoi; ++itr) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == cuid) {
      segments.push_back(segmentEntry->segment);
    }
  }
}

SharedHandle<Segment> SegmentMan::getSegment(cuid_t cuid, size_t minSplitSize)
{
  SharedHandle<Piece> piece =
    pieceStorage_->getSparseMissingUnusedPiece
    (minSplitSize,
     ignoreBitfield_.getFilterBitfield(), ignoreBitfield_.getBitfieldLength());
  return checkoutSegment(cuid, piece);
}

void SegmentMan::getSegment
(std::vector<SharedHandle<Segment> >& segments,
 cuid_t cuid,
 size_t minSplitSize,
 const SharedHandle<FileEntry>& fileEntry,
 size_t maxSegments)
{
  BitfieldMan filter(ignoreBitfield_);
  filter.enableFilter();
  filter.addNotFilter(fileEntry->getOffset(), fileEntry->getLength());
  std::vector<SharedHandle<Segment> > pending;
  while(segments.size() < maxSegments) {
    SharedHandle<Segment> segment =
      checkoutSegment(cuid,
                      pieceStorage_->getSparseMissingUnusedPiece
                      (minSplitSize,
                       filter.getFilterBitfield(), filter.getBitfieldLength()));
    if(!segment) {
      break;
    }
    if(segment->getPositionToWrite() < fileEntry->getOffset() ||
       fileEntry->getLastOffset() <= segment->getPositionToWrite()) {
      pending.push_back(segment);
    } else {
      segments.push_back(segment);
    }
  }
  for(std::vector<SharedHandle<Segment> >::const_iterator i = pending.begin(),
        eoi = pending.end(); i != eoi; ++i) {
    cancelSegment(cuid, *i);
  }
}

SharedHandle<Segment> SegmentMan::getSegmentWithIndex
(cuid_t cuid, size_t index) {
  if(index > 0 && downloadContext_->getNumPieces() <= index) {
    return SharedHandle<Segment>();
  }
  return checkoutSegment(cuid, pieceStorage_->getMissingPiece(index));
}

SharedHandle<Segment> SegmentMan::getCleanSegmentIfOwnerIsIdle
(cuid_t cuid, size_t index)
{
  if(index > 0 && downloadContext_->getNumPieces() <= index) {
    return SharedHandle<Segment>();
  }
  for(SegmentEntries::const_iterator itr = usedSegmentEntries_.begin(),
        eoi = usedSegmentEntries_.end(); itr != eoi; ++itr) {
    const SharedHandle<SegmentEntry>& segmentEntry = *itr;
    if(segmentEntry->segment->getIndex() == index) {
      if(segmentEntry->segment->getWrittenLength() > 0) {
        return SharedHandle<Segment>();
      }
      if(segmentEntry->cuid == cuid) {
        return segmentEntry->segment;
      }
      cuid_t owner = segmentEntry->cuid;
      SharedHandle<PeerStat> ps = getPeerStat(owner);
      if(!ps || ps->getStatus() == PeerStat::IDLE) {
        cancelSegment(owner);
        return getSegmentWithIndex(cuid, index);
      } else {
        return SharedHandle<Segment>();
      }
    }
  }
  return SharedHandle<Segment>();
}

void SegmentMan::cancelSegment(const SharedHandle<Segment>& segment)
{
  A2_LOG_DEBUG(fmt("Canceling segment#%lu",
                   static_cast<unsigned long>(segment->getIndex())));
  pieceStorage_->cancelPiece(segment->getPiece());
  segmentWrittenLengthMemo_[segment->getIndex()] = segment->getWrittenLength();
  A2_LOG_DEBUG(fmt("Memorized segment index=%lu, writtenLength=%lu",
                   static_cast<unsigned long>(segment->getIndex()),
                   static_cast<unsigned long>(segment->getWrittenLength())));
}

void SegmentMan::cancelSegment(cuid_t cuid) {
  for(SegmentEntries::iterator itr = usedSegmentEntries_.begin(),
        eoi = usedSegmentEntries_.end(); itr != eoi;) {
    if((*itr)->cuid == cuid) {
      cancelSegment((*itr)->segment);
      itr = usedSegmentEntries_.erase(itr);
      eoi = usedSegmentEntries_.end();
    } else {
      ++itr;
    }
  }
}

void SegmentMan::cancelSegment
(cuid_t cuid, const SharedHandle<Segment>& segment)
{
  for(SegmentEntries::iterator itr = usedSegmentEntries_.begin(),
        eoi = usedSegmentEntries_.end(); itr != eoi;) {
    if((*itr)->cuid == cuid && *(*itr)->segment == *segment) {
      cancelSegment((*itr)->segment);
      itr = usedSegmentEntries_.erase(itr);
      //eoi = usedSegmentEntries_.end();
      break;
    } else {
      ++itr;
    }
  }
}

void SegmentMan::cancelAllSegments()
{
  for(std::deque<SharedHandle<SegmentEntry> >::iterator itr =
        usedSegmentEntries_.begin(), eoi = usedSegmentEntries_.end();
      itr != eoi; ++itr) {
    cancelSegment((*itr)->segment);
  }
  usedSegmentEntries_.clear();
}

void SegmentMan::eraseSegmentWrittenLengthMemo()
{
  segmentWrittenLengthMemo_.clear();
}

namespace {
class FindSegmentEntry {
private:
  SharedHandle<Segment> segment_;
public:
  FindSegmentEntry(const SharedHandle<Segment>& segment):segment_(segment) {}

  bool operator()(const SegmentEntryHandle& segmentEntry) const
  {
    return segmentEntry->segment->getIndex() == segment_->getIndex();
  }
};
} // namespace

bool SegmentMan::completeSegment
(cuid_t cuid, const SharedHandle<Segment>& segment) {
  pieceStorage_->completePiece(segment->getPiece());
  pieceStorage_->advertisePiece(cuid, segment->getPiece()->getIndex());
  SegmentEntries::iterator itr = std::find_if(usedSegmentEntries_.begin(),
                                              usedSegmentEntries_.end(),
                                              FindSegmentEntry(segment));
  if(itr == usedSegmentEntries_.end()) {
    return false;
  } else {
    usedSegmentEntries_.erase(itr);
    return true;
  }
}

bool SegmentMan::hasSegment(size_t index) const {
  return pieceStorage_->hasPiece(index);
}

uint64_t SegmentMan::getDownloadLength() const {
  if(!pieceStorage_) {
    return 0;
  } else {
    return pieceStorage_->getCompletedLength();
  }
}

void SegmentMan::registerPeerStat(const SharedHandle<PeerStat>& peerStat)
{
  for(std::vector<SharedHandle<PeerStat> >::iterator i = peerStats_.begin(),
        eoi = peerStats_.end(); i != eoi; ++i) {
    if((*i)->getStatus() == PeerStat::IDLE) {
      *i = peerStat;
      return;
    }
  }
  peerStats_.push_back(peerStat);
}

SharedHandle<PeerStat> SegmentMan::getPeerStat(cuid_t cuid) const
{
  for(std::vector<SharedHandle<PeerStat> >::const_iterator i =
        peerStats_.begin(), eoi = peerStats_.end(); i != eoi; ++i) {
    if((*i)->getCuid() == cuid) {
      return *i;
    }
  }
  return SharedHandle<PeerStat>();
}

namespace {
class PeerStatHostProtoEqual {
private:
  const SharedHandle<PeerStat>& peerStat_;
public:
  PeerStatHostProtoEqual(const SharedHandle<PeerStat>& peerStat):
    peerStat_(peerStat) {}

  bool operator()(const SharedHandle<PeerStat>& p) const
  {
    return peerStat_->getHostname() == p->getHostname() &&
      peerStat_->getProtocol() == p->getProtocol();
  }
};
} // namespace

void SegmentMan::updateFastestPeerStat(const SharedHandle<PeerStat>& peerStat)
{
  std::vector<SharedHandle<PeerStat> >::iterator i =
    std::find_if(fastestPeerStats_.begin(), fastestPeerStats_.end(),
                 PeerStatHostProtoEqual(peerStat));
  if(i == fastestPeerStats_.end()) {
    fastestPeerStats_.push_back(peerStat);
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
  if(lastPeerStatDlspdMapUpdated_.differenceInMillis(global::wallclock) >= 250){
    lastPeerStatDlspdMapUpdated_ = global::wallclock;
    peerStatDlspdMap_.clear();
    for(std::vector<SharedHandle<PeerStat> >::const_iterator i =
          peerStats_.begin(), eoi = peerStats_.end(); i != eoi; ++i) {
      if((*i)->getStatus() == PeerStat::ACTIVE) {
        unsigned int s = (*i)->calculateDownloadSpeed();
        peerStatDlspdMap_[(*i)->getCuid()] = s;
        speed += s;
      }
    }
    cachedDlspd_ = speed;
  } else {
    speed = cachedDlspd_;
  }
  return speed;
}

void SegmentMan::updateDownloadSpeedFor(const SharedHandle<PeerStat>& pstat)
{
  unsigned int newspd = pstat->calculateDownloadSpeed();
  unsigned int oldSpd = peerStatDlspdMap_[pstat->getCuid()];
  if(cachedDlspd_ > oldSpd) {
    cachedDlspd_ -= oldSpd;
    cachedDlspd_ += newspd;
  } else {
    cachedDlspd_ = newspd;
  }
  peerStatDlspdMap_[pstat->getCuid()] = newspd;
}

namespace {
class PeerStatDownloadLengthOperator {
public:
  uint64_t operator()(uint64_t total, const SharedHandle<PeerStat>& ps)
  {
    return ps->getSessionDownloadLength()+total;
  }
};
} // namespace

uint64_t SegmentMan::calculateSessionDownloadLength() const
{
  return std::accumulate(fastestPeerStats_.begin(), fastestPeerStats_.end(),
                         0LL, PeerStatDownloadLengthOperator());
}

size_t SegmentMan::countFreePieceFrom(size_t index) const
{
  size_t numPieces = downloadContext_->getNumPieces();
  for(size_t i = index; i < numPieces; ++i) {
    if(pieceStorage_->hasPiece(i) || pieceStorage_->isPieceUsed(i)) {
      return i-index;
    }
  }
  return downloadContext_->getNumPieces()-index;
}

void SegmentMan::ignoreSegmentFor(const SharedHandle<FileEntry>& fileEntry)
{
  A2_LOG_DEBUG(fmt("ignoring segment for path=%s, offset=%s, length=%s",
                   fileEntry->getPath().c_str(),
                   util::itos(fileEntry->getOffset()).c_str(),
                   util::uitos(fileEntry->getLength()).c_str()));
  ignoreBitfield_.addFilter(fileEntry->getOffset(), fileEntry->getLength());
}

void SegmentMan::recognizeSegmentFor(const SharedHandle<FileEntry>& fileEntry)
{
  ignoreBitfield_.removeFilter(fileEntry->getOffset(), fileEntry->getLength());
}

bool SegmentMan::allSegmentsIgnored() const
{
  return ignoreBitfield_.isAllFilterBitSet();
}

} // namespace aria2
