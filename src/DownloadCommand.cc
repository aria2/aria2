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
#include "DownloadCommand.h"

#include <cassert>

#include "Request.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "PeerStat.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "SegmentMan.h"
#include "Segment.h"
#include "Logger.h"
#include "ChecksumCheckIntegrityEntry.h"
#include "PieceStorage.h"
#include "CheckIntegrityCommand.h"
#include "DiskAdaptor.h"
#include "DownloadContext.h"
#include "Option.h"
#include "util.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "StringFormat.h"
#include "RequestGroupMan.h"
#include "wallclock.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "SinkStreamFilter.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

static const size_t BUFSIZE = 16*1024;

DownloadCommand::DownloadCommand(cuid_t cuid,
                                 const SharedHandle<Request>& req,
                                 const SharedHandle<FileEntry>& fileEntry,
                                 RequestGroup* requestGroup,
                                 DownloadEngine* e,
                                 const SocketHandle& s):
  AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
  buf_(new unsigned char[BUFSIZE]),
  startupIdleTime_(10),
  lowestDownloadSpeedLimit_(0)
#ifdef ENABLE_MESSAGE_DIGEST
  , pieceHashValidationEnabled_(false)
#endif // ENABLE_MESSAGE_DIGEST
{
#ifdef ENABLE_MESSAGE_DIGEST
  {
    if(getOption()->getAsBool(PREF_REALTIME_CHUNK_CHECKSUM)) {
      const std::string& algo = getDownloadContext()->getPieceHashAlgo();
      if(MessageDigestContext::supports(algo)) {
        messageDigestContext_.reset(new MessageDigestContext());
        messageDigestContext_->trySetAlgo(algo);
        messageDigestContext_->digestInit();
        
        pieceHashValidationEnabled_ = true;
      }
    }
  }
#endif // ENABLE_MESSAGE_DIGEST

  peerStat_ = req->initPeerStat();
  peerStat_->downloadStart();
  getSegmentMan()->registerPeerStat(peerStat_);

  streamFilter_.reset(new SinkStreamFilter(pieceHashValidationEnabled_));
  streamFilter_->init();
  sinkFilterOnly_ = true;
}

DownloadCommand::~DownloadCommand() {
  peerStat_->downloadStop();
  getSegmentMan()->updateFastestPeerStat(peerStat_);
  delete [] buf_;
}

bool DownloadCommand::executeInternal() {
  if(getDownloadEngine()->getRequestGroupMan()->doesOverallDownloadSpeedExceed()
     || getRequestGroup()->doesDownloadSpeedExceed()) {
    getDownloadEngine()->addCommand(this);
    disableReadCheckSocket();
    return false;
  }
  setReadCheckSocket(getSocket());

  const SharedHandle<DiskAdaptor>& diskAdaptor =
    getPieceStorage()->getDiskAdaptor();
  SharedHandle<Segment> segment = getSegments().front();
  size_t bufSize;
  if(sinkFilterOnly_) {
    if(segment->getLength() > 0 ) {
      if(static_cast<uint64_t>(segment->getPosition()+segment->getLength()) <=
         static_cast<uint64_t>(getFileEntry()->getLastOffset())) {
        bufSize = std::min(segment->getLength()-segment->getWrittenLength(),
                           BUFSIZE);
      } else {
        bufSize =
          std::min
          (static_cast<size_t>
           (getFileEntry()->getLastOffset()-segment->getPositionToWrite()),
           BUFSIZE);
      }
    } else {
      bufSize = BUFSIZE;
    }
    getSocket()->readData(buf_, bufSize);
    streamFilter_->transform(diskAdaptor, segment, buf_, bufSize);
  } else {
    // It is possible that segment is completed but we have some bytes
    // of stream to read. For example, chunked encoding has "0"+CRLF
    // after data. After we read data(at this moment segment is
    // completed), we need another 3bytes(or more if it has trailers).
    bufSize = BUFSIZE;
    getSocket()->peekData(buf_, bufSize);
    streamFilter_->transform(diskAdaptor, segment, buf_, bufSize);
    bufSize = streamFilter_->getBytesProcessed();
    getSocket()->readData(buf_, bufSize);
  }
  peerStat_->updateDownloadLength(bufSize);
  getSegmentMan()->updateDownloadSpeedFor(peerStat_);
  bool segmentPartComplete = false;
  // Note that GrowSegment::complete() always returns false.
  if(sinkFilterOnly_) {
    if(segment->complete() ||
       segment->getPositionToWrite() == getFileEntry()->getLastOffset()) {
      segmentPartComplete = true;
    } else if(segment->getLength() == 0 && bufSize == 0 &&
              !getSocket()->wantRead() && !getSocket()->wantWrite()) {
      segmentPartComplete = true;
    }
  } else {
    off_t loff = getFileEntry()->gtoloff(segment->getPositionToWrite());
    if(getFileEntry()->getLength() > 0 && !sinkFilterOnly_ &&
       ((loff == getRequestEndOffset() && streamFilter_->finished())
        || loff < getRequestEndOffset()) &&
       (segment->complete() ||
        segment->getPositionToWrite() == getFileEntry()->getLastOffset())) {
      // In this case, StreamFilter other than *SinkStreamFilter is
      // used and Content-Length is known.  We check
      // streamFilter_->finished() only if the requested end offset
      // equals to written position in file local offset; in other
      // words, data in the requested ranage is all received.  If
      // requested end offset is greater than this segment, then
      // streamFilter_ is not finished in this segment.
      segmentPartComplete = true;
    } else if(streamFilter_->finished()) {
      segmentPartComplete = true;
    }
  }

  if(!segmentPartComplete && bufSize == 0 &&
     !getSocket()->wantRead() && !getSocket()->wantWrite()) {
    throw DL_RETRY_EX(EX_GOT_EOF);
  }

  if(segmentPartComplete) {
    if(segment->complete() || segment->getLength() == 0) {
      // If segment->getLength() == 0, the server doesn't provide
      // content length, but the client detected that download
      // completed.
      if(getLogger()->info()) {
        getLogger()->info(MSG_SEGMENT_DOWNLOAD_COMPLETED,
                          util::itos(getCuid()).c_str());
      }
#ifdef ENABLE_MESSAGE_DIGEST

      {
        const std::string& expectedPieceHash =
          getDownloadContext()->getPieceHash(segment->getIndex());
        if(pieceHashValidationEnabled_ && !expectedPieceHash.empty()) {
          if(
#ifdef ENABLE_BITTORRENT
             (!getPieceStorage()->isEndGame() ||
              !getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)) &&
#endif // ENABLE_BITTORRENT
             segment->isHashCalculated()) {
            if(getLogger()->debug()) {
              getLogger()->debug
                ("Hash is available! index=%lu",
                 static_cast<unsigned long>(segment->getIndex()));
            }
            validatePieceHash
              (segment, expectedPieceHash, segment->getHashString());
          } else {
            messageDigestContext_->digestReset();
            validatePieceHash
              (segment, expectedPieceHash,
               MessageDigestHelper::digest
               (messageDigestContext_.get(),
                getPieceStorage()->getDiskAdaptor(),
                segment->getPosition(),
                segment->getLength()));
          }
        } else {
          getSegmentMan()->completeSegment(getCuid(), segment);
        }
      }

#else // !ENABLE_MESSAGE_DIGEST
      getSegmentMan()->completeSegment(getCuid(), segment);
#endif // !ENABLE_MESSAGE_DIGEST
    } else {
      // If segment is not canceled here, in the next pipelining
      // request, aria2 requests bad range
      // [FileEntry->getLastOffset(), FileEntry->getLastOffset())
      getSegmentMan()->cancelSegment(getCuid(), segment);
    }
    checkLowestDownloadSpeed();
    // this unit is going to download another segment.
    return prepareForNextSegment();
  } else {
    checkLowestDownloadSpeed();
    setWriteCheckSocketIf(getSocket(), getSocket()->wantWrite());
    getDownloadEngine()->addCommand(this);
    return false;
  }
}

void DownloadCommand::checkLowestDownloadSpeed() const
{
  // calculate downloading speed
  if(peerStat_->getDownloadStartTime().difference(global::wallclock) >=
     startupIdleTime_) {
    unsigned int nowSpeed = peerStat_->calculateDownloadSpeed();
    if(lowestDownloadSpeedLimit_ > 0 && nowSpeed <= lowestDownloadSpeedLimit_) {
      throw DL_ABORT_EX2(StringFormat(EX_TOO_SLOW_DOWNLOAD_SPEED,
                                      nowSpeed,
                                      lowestDownloadSpeedLimit_,
                                      getRequest()->getHost().c_str()).str(),
                         downloadresultcode::TOO_SLOW_DOWNLOAD_SPEED);
    }
  }
}

bool DownloadCommand::prepareForNextSegment() {
  if(getRequestGroup()->downloadFinished()) {
    // Remove in-flight request here.
    getFileEntry()->poolRequest(getRequest());
    // If this is a single file download, and file size becomes known
    // just after downloading, set total length to FileEntry object
    // here.
    if(getDownloadContext()->getFileEntries().size() == 1) {
      if(getFileEntry()->getLength() == 0) {
        getFileEntry()->setLength(getPieceStorage()->getCompletedLength());
      }
    }
#ifdef ENABLE_MESSAGE_DIGEST
    if(getDownloadContext()->getPieceHashAlgo().empty()) {
      SharedHandle<CheckIntegrityEntry> entry
        (new ChecksumCheckIntegrityEntry(getRequestGroup()));
      if(entry->isValidationReady()) {
        entry->initValidator();
        entry->cutTrailingGarbage();
        getDownloadEngine()->getCheckIntegrityMan()->pushEntry(entry);
      }
    }
    // Following 2lines are needed for DownloadEngine to detect
    // completed RequestGroups without 1sec delay.
    getDownloadEngine()->setNoWait(true);
    getDownloadEngine()->setRefreshInterval(0);
#endif // ENABLE_MESSAGE_DIGEST
    return true;
  } else {
    // The number of segments should be 1 in order to pass through the next
    // segment.
    if(getSegments().size() == 1) {
      SharedHandle<Segment> tempSegment = getSegments().front();
      if(!tempSegment->complete()) {
        return prepareForRetry(0);
      }
      if(getRequestEndOffset() ==
         getFileEntry()->gtoloff
         (tempSegment->getPosition()+tempSegment->getLength())) {
        return prepareForRetry(0);
      }
      SharedHandle<Segment> nextSegment = getSegmentMan()->getSegmentWithIndex
        (getCuid(), tempSegment->getIndex()+1);
      if(nextSegment.isNull()) {
        nextSegment = getSegmentMan()->getCleanSegmentIfOwnerIsIdle
        (getCuid(), tempSegment->getIndex()+1);
      }
      if(nextSegment.isNull() || nextSegment->getWrittenLength() > 0) {
        // If nextSegment->getWrittenLength() > 0, current socket must
        // be closed because writing incoming data at
        // nextSegment->getWrittenLength() corrupts file.
        return prepareForRetry(0);
      } else {
        getDownloadEngine()->addCommand(this);
        return false;
      }
    } else {
      return prepareForRetry(0);
    }
  }
}

#ifdef ENABLE_MESSAGE_DIGEST

void DownloadCommand::validatePieceHash(const SharedHandle<Segment>& segment,
                                        const std::string& expectedPieceHash,
                                        const std::string& actualPieceHash)
{
  if(actualPieceHash == expectedPieceHash) {
    getLogger()->info(MSG_GOOD_CHUNK_CHECKSUM, actualPieceHash.c_str());
    getSegmentMan()->completeSegment(getCuid(), segment);
  } else {
    getLogger()->info(EX_INVALID_CHUNK_CHECKSUM,
                      segment->getIndex(),
                      util::itos(segment->getPosition(), true).c_str(),
                      expectedPieceHash.c_str(),
                      actualPieceHash.c_str());
    segment->clear();
    getSegmentMan()->cancelSegment(getCuid());
    throw DL_RETRY_EX
      (StringFormat("Invalid checksum index=%d", segment->getIndex()).str());
  }
}

void DownloadCommand::installStreamFilter
(const SharedHandle<StreamFilter>& streamFilter)
{
  if(streamFilter.isNull()) {
    return;
  }
  streamFilter->installDelegate(streamFilter_);
  streamFilter_ = streamFilter;
  sinkFilterOnly_ =
    util::endsWith(streamFilter_->getName(), SinkStreamFilter::NAME);
}

#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
