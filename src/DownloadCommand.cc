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
#include "Decoder.h"
#include "RequestGroupMan.h"
#include "wallclock.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

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
  SharedHandle<Segment> segment = getSegments().front();

  size_t bufSize;
  if(segment->getLength() > 0) {
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
  // It is possible that segment is completed but we have some bytes
  // of stream to read. For example, chunked encoding has "0"+CRLF
  // after data. After we read data(at this moment segment is
  // completed), we need another 3bytes(or more if it has extension).
  if(bufSize == 0 &&
     ((!transferEncodingDecoder_.isNull() &&
       !transferEncodingDecoder_->finished()) ||
      (!contentEncodingDecoder_.isNull() &&
       !contentEncodingDecoder_->finished()))) {
    bufSize = 1;
  }
  getSocket()->readData(buf_, bufSize);

  const SharedHandle<DiskAdaptor>& diskAdaptor =
    getPieceStorage()->getDiskAdaptor();

  const unsigned char* bufFinal;
  size_t bufSizeFinal;

  std::string decoded;
  if(transferEncodingDecoder_.isNull()) {
    bufFinal = buf_;
    bufSizeFinal = bufSize;
  } else {
    decoded = transferEncodingDecoder_->decode(buf_, bufSize);

    bufFinal = reinterpret_cast<const unsigned char*>(decoded.c_str());
    bufSizeFinal = decoded.size();
  }

  if(contentEncodingDecoder_.isNull()) {
    diskAdaptor->writeData(bufFinal, bufSizeFinal,
                           segment->getPositionToWrite());
  } else {
    std::string out = contentEncodingDecoder_->decode(bufFinal, bufSizeFinal);
    diskAdaptor->writeData(reinterpret_cast<const unsigned char*>(out.data()),
                           out.size(),
                           segment->getPositionToWrite());
    bufSizeFinal = out.size();
  }

#ifdef ENABLE_MESSAGE_DIGEST

  if(pieceHashValidationEnabled_) {
    segment->updateHash(segment->getWrittenLength(), bufFinal, bufSizeFinal);
  }

#endif // ENABLE_MESSAGE_DIGEST
  if(bufSizeFinal > 0) {
    segment->updateWrittenLength(bufSizeFinal);
  }
  peerStat_->updateDownloadLength(bufSize);
  getSegmentMan()->updateDownloadSpeedFor(peerStat_);
  bool segmentPartComplete = false;
  // Note that GrowSegment::complete() always returns false.
  if(transferEncodingDecoder_.isNull() && contentEncodingDecoder_.isNull()) {
    if(segment->complete() ||
       segment->getPositionToWrite() == getFileEntry()->getLastOffset()) {
      segmentPartComplete = true;
    } else if(segment->getLength() == 0 && bufSize == 0 &&
              !getSocket()->wantRead() && !getSocket()->wantWrite()) {
      segmentPartComplete = true;
    }
  } else {
    off_t loff = getFileEntry()->gtoloff(segment->getPositionToWrite());
    if(!transferEncodingDecoder_.isNull() &&
       ((loff == getRequestEndOffset() && transferEncodingDecoder_->finished())
        || loff < getRequestEndOffset()) &&
       (segment->complete() ||
        segment->getPositionToWrite() == getFileEntry()->getLastOffset())) {
      // In this case, transferEncodingDecoder is used and
      // Content-Length is known.  We check
      // transferEncodingDecoder_->finished() only if the requested
      // end offset equals to written position in file local offset;
      // in other words, data in the requested ranage is all received.
      // If requested end offset is greater than this segment, then
      // transferEncodingDecoder_ is not finished in this segment.
      segmentPartComplete = true;
    } else if((transferEncodingDecoder_.isNull() ||
               transferEncodingDecoder_->finished()) &&
              (contentEncodingDecoder_.isNull() ||
               contentEncodingDecoder_->finished())) {
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
          if(segment->isHashCalculated()) {
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
      SharedHandle<Segment> nextSegment = getSegmentMan()->getSegment
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

#endif // ENABLE_MESSAGE_DIGEST

void DownloadCommand::setTransferEncodingDecoder
(const SharedHandle<Decoder>& decoder)
{
  this->transferEncodingDecoder_ = decoder;
}

void DownloadCommand::setContentEncodingDecoder
(const SharedHandle<Decoder>& decoder)
{
  contentEncodingDecoder_ = decoder;
}

} // namespace aria2
