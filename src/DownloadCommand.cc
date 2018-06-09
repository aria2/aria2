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
#include "LogFactory.h"
#include "ChecksumCheckIntegrityEntry.h"
#include "PieceStorage.h"
#include "CheckIntegrityCommand.h"
#include "DiskAdaptor.h"
#include "DownloadContext.h"
#include "Option.h"
#include "util.h"
#include "SocketCore.h"
#include "message.h"
#include "prefs.h"
#include "fmt.h"
#include "RequestGroupMan.h"
#include "wallclock.h"
#include "SinkStreamFilter.h"
#include "FileEntry.h"
#include "SocketRecvBuffer.h"
#include "Piece.h"
#include "WrDiskCacheEntry.h"
#include "DownloadFailureException.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

DownloadCommand::DownloadCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e, const std::shared_ptr<SocketCore>& s,
    const std::shared_ptr<SocketRecvBuffer>& socketRecvBuffer)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, s,
                      socketRecvBuffer),
      startupIdleTime_(10),
      lowestDownloadSpeedLimit_(0),
      pieceHashValidationEnabled_(false)
{
  {
    if (getOption()->getAsBool(PREF_REALTIME_CHUNK_CHECKSUM)) {
      const std::string& algo = getDownloadContext()->getPieceHashType();
      if (MessageDigest::supports(algo)) {
        messageDigest_ = MessageDigest::create(algo);
        pieceHashValidationEnabled_ = true;
      }
    }
  }

  peerStat_ = req->initPeerStat();
  peerStat_->downloadStart();
  getSegmentMan()->registerPeerStat(peerStat_);

  streamFilter_ = make_unique<SinkStreamFilter>(
      getPieceStorage()->getWrDiskCache(), pieceHashValidationEnabled_);
  streamFilter_->init();
  sinkFilterOnly_ = true;
  checkSocketRecvBuffer();
}

DownloadCommand::~DownloadCommand()
{
  peerStat_->downloadStop();
  getSegmentMan()->updateFastestPeerStat(peerStat_);
}

namespace {
void flushWrDiskCacheEntry(WrDiskCache* wrDiskCache,
                           const std::shared_ptr<Segment>& segment)
{
  const std::shared_ptr<Piece>& piece = segment->getPiece();
  if (piece->getWrDiskCacheEntry()) {
    piece->flushWrCache(wrDiskCache);
    if (piece->getWrDiskCacheEntry()->getError() !=
        WrDiskCacheEntry::CACHE_ERR_SUCCESS) {
      segment->clear(wrDiskCache);
      throw DOWNLOAD_FAILURE_EXCEPTION2(
          fmt("Write disk cache flush failure index=%lu",
              static_cast<unsigned long>(piece->getIndex())),
          piece->getWrDiskCacheEntry()->getErrorCode());
    }
  }
}
} // namespace

bool DownloadCommand::executeInternal()
{
  if (getDownloadEngine()
          ->getRequestGroupMan()
          ->doesOverallDownloadSpeedExceed() ||
      getRequestGroup()->doesDownloadSpeedExceed()) {
    addCommandSelf();
    disableReadCheckSocket();
    disableWriteCheckSocket();
    return false;
  }
  setReadCheckSocket(getSocket());

  const std::shared_ptr<DiskAdaptor>& diskAdaptor =
      getPieceStorage()->getDiskAdaptor();
  std::shared_ptr<Segment> segment = getSegments().front();
  bool eof = false;
  if (getSocketRecvBuffer()->bufferEmpty()) {
    // Only read from socket when buffer is empty.  Imagine that When
    // segment length is *short* and we are using HTTP pilelining.  We
    // issued 2 requests in pipeline. When reading first response
    // header, we may read its response body and 2nd response header
    // and 2nd response body in buffer if they are small enough to fit
    // in buffer. And then server may sends EOF.  In this case, we
    // read data from socket here, we will get EOF and leaves 2nd
    // response unprocessed.  To prevent this, we don't read from
    // socket when buffer is not empty.
    eof = getSocketRecvBuffer()->recv() == 0 && !getSocket()->wantRead() &&
          !getSocket()->wantWrite();
  }
  if (!eof) {
    size_t bufSize;
    if (sinkFilterOnly_) {
      if (segment->getLength() > 0) {
        if (segment->getPosition() + segment->getLength() <=
            getFileEntry()->getLastOffset()) {
          bufSize = std::min(static_cast<size_t>(segment->getLength() -
                                                 segment->getWrittenLength()),
                             getSocketRecvBuffer()->getBufferLength());
        }
        else {
          bufSize =
              std::min(static_cast<size_t>(getFileEntry()->getLastOffset() -
                                           segment->getPositionToWrite()),
                       getSocketRecvBuffer()->getBufferLength());
        }
      }
      else {
        bufSize = getSocketRecvBuffer()->getBufferLength();
      }
      streamFilter_->transform(diskAdaptor, segment,
                               getSocketRecvBuffer()->getBuffer(), bufSize);
    }
    else {
      // It is possible that segment is completed but we have some bytes
      // of stream to read. For example, chunked encoding has "0"+CRLF
      // after data. After we read data(at this moment segment is
      // completed), we need another 3bytes(or more if it has trailers).
      streamFilter_->transform(diskAdaptor, segment,
                               getSocketRecvBuffer()->getBuffer(),
                               getSocketRecvBuffer()->getBufferLength());
      bufSize = streamFilter_->getBytesProcessed();
    }
    getSocketRecvBuffer()->drain(bufSize);
    peerStat_->updateDownload(bufSize);
    getDownloadContext()->updateDownload(bufSize);
  }
  bool segmentPartComplete = false;
  // Note that GrowSegment::complete() always returns false.
  if (sinkFilterOnly_) {
    if (segment->complete() ||
        (getFileEntry()->getLength() != 0 &&
         segment->getPositionToWrite() == getFileEntry()->getLastOffset())) {
      segmentPartComplete = true;
    }
    else if (segment->getLength() == 0 && eof) {
      segmentPartComplete = true;
    }
  }
  else {
    int64_t loff = getFileEntry()->gtoloff(segment->getPositionToWrite());
    if (getFileEntry()->getLength() > 0 && !sinkFilterOnly_ &&
        ((loff == getRequestEndOffset() && streamFilter_->finished()) ||
         loff < getRequestEndOffset()) &&
        (segment->complete() ||
         segment->getPositionToWrite() == getFileEntry()->getLastOffset())) {
      // In this case, StreamFilter other than *SinkStreamFilter is
      // used and Content-Length is known.  We check
      // streamFilter_->finished() only if the requested end offset
      // equals to written position in file local offset; in other
      // words, data in the requested range is all received.  If
      // requested end offset is greater than this segment, then
      // streamFilter_ is not finished in this segment.
      segmentPartComplete = true;
    }
    else if (streamFilter_->finished()) {
      segmentPartComplete = true;
    }
  }

  if (!segmentPartComplete && eof) {
    throw DL_RETRY_EX(EX_GOT_EOF);
  }

  if (segmentPartComplete) {
    if (segment->complete() || segment->getLength() == 0) {
      // If segment->getLength() == 0, the server doesn't provide
      // content length, but the client detected that download
      // completed.
      A2_LOG_INFO(fmt(MSG_SEGMENT_DOWNLOAD_COMPLETED, getCuid()));

      {
        const std::string& expectedPieceHash =
            getDownloadContext()->getPieceHash(segment->getIndex());
        if (pieceHashValidationEnabled_ && !expectedPieceHash.empty()) {
          if (
#ifdef ENABLE_BITTORRENT
              // TODO Is this necessary?
              (!getPieceStorage()->isEndGame() ||
               !getDownloadContext()->hasAttribute(CTX_ATTR_BT)) &&
#endif // ENABLE_BITTORRENT
              segment->isHashCalculated()) {
            A2_LOG_DEBUG(fmt("Hash is available! index=%lu",
                             static_cast<unsigned long>(segment->getIndex())));
            validatePieceHash(segment, expectedPieceHash, segment->getDigest());
          }
          else {
            try {
              std::string actualHash =
                  segment->getPiece()->getDigestWithWrCache(
                      segment->getSegmentLength(), diskAdaptor);
              validatePieceHash(segment, expectedPieceHash, actualHash);
            }
            catch (RecoverableException& e) {
              segment->clear(getPieceStorage()->getWrDiskCache());
              getSegmentMan()->cancelSegment(getCuid());
              throw;
            }
          }
        }
        else {
          completeSegment(getCuid(), segment);
        }
      }
    }
    else {
      // If segment is not canceled here, in the next pipelining
      // request, aria2 requests bad range
      // [FileEntry->getLastOffset(), FileEntry->getLastOffset())
      getSegmentMan()->cancelSegment(getCuid(), segment);
    }
    checkLowestDownloadSpeed();
    // this unit is going to download another segment.
    return prepareForNextSegment();
  }
  else {
    checkLowestDownloadSpeed();
    setWriteCheckSocketIf(getSocket(), shouldEnableWriteCheck());
    checkSocketRecvBuffer();
    addCommandSelf();
    return false;
  }
}

bool DownloadCommand::shouldEnableWriteCheck()
{
  return getSocket()->wantWrite();
}

void DownloadCommand::checkLowestDownloadSpeed() const
{
  if (lowestDownloadSpeedLimit_ > 0 &&
      peerStat_->getDownloadStartTime().difference(global::wallclock()) >=
          startupIdleTime_) {
    int nowSpeed = peerStat_->calculateDownloadSpeed();
    if (nowSpeed <= lowestDownloadSpeedLimit_) {
      throw DL_ABORT_EX2(fmt(EX_TOO_SLOW_DOWNLOAD_SPEED, nowSpeed,
                             lowestDownloadSpeedLimit_,
                             getRequest()->getHost().c_str()),
                         error_code::TOO_SLOW_DOWNLOAD_SPEED);
    }
  }
}

bool DownloadCommand::prepareForNextSegment()
{
  if (getRequestGroup()->downloadFinished()) {
    // Remove in-flight request here.
    getFileEntry()->poolRequest(getRequest());
    // If this is a single file download, and file size becomes known
    // just after downloading, set total length to FileEntry object
    // here.
    if (getDownloadContext()->getFileEntries().size() == 1) {
      if (getFileEntry()->getLength() == 0) {
        getFileEntry()->setLength(getPieceStorage()->getCompletedLength());
      }
    }
    if (getDownloadContext()->getPieceHashType().empty()) {
      auto entry = make_unique<ChecksumCheckIntegrityEntry>(getRequestGroup());
      if (entry->isValidationReady()) {
        entry->initValidator();
        entry->cutTrailingGarbage();
        getDownloadEngine()->getCheckIntegrityMan()->pushEntry(
            std::move(entry));
      }
    }
    // Following 2lines are needed for DownloadEngine to detect
    // completed RequestGroups without 1sec delay.
    getDownloadEngine()->setNoWait(true);
    getDownloadEngine()->setRefreshInterval(std::chrono::milliseconds(0));
    return true;
  }
  else {
    // The number of segments should be 1 in order to pass through the next
    // segment.
    if (getSegments().size() == 1) {
      std::shared_ptr<Segment> tempSegment = getSegments().front();
      if (!tempSegment->complete()) {
        return prepareForRetry(0);
      }
      if (getRequestEndOffset() ==
          getFileEntry()->gtoloff(tempSegment->getPosition() +
                                  tempSegment->getLength())) {
        return prepareForRetry(0);
      }
      std::shared_ptr<Segment> nextSegment =
          getSegmentMan()->getSegmentWithIndex(getCuid(),
                                               tempSegment->getIndex() + 1);
      if (!nextSegment) {
        nextSegment = getSegmentMan()->getCleanSegmentIfOwnerIsIdle(
            getCuid(), tempSegment->getIndex() + 1);
      }
      if (!nextSegment || nextSegment->getWrittenLength() > 0) {
        // If nextSegment->getWrittenLength() > 0, current socket must
        // be closed because writing incoming data at
        // nextSegment->getWrittenLength() corrupts file.
        return prepareForRetry(0);
      }
      else {
        checkSocketRecvBuffer();
        addCommandSelf();
        return false;
      }
    }
    else {
      return prepareForRetry(0);
    }
  }
}

void DownloadCommand::validatePieceHash(const std::shared_ptr<Segment>& segment,
                                        const std::string& expectedHash,
                                        const std::string& actualHash)
{
  if (actualHash == expectedHash) {
    A2_LOG_INFO(fmt(MSG_GOOD_CHUNK_CHECKSUM, util::toHex(actualHash).c_str()));
    completeSegment(getCuid(), segment);
  }
  else {
    A2_LOG_INFO(fmt(EX_INVALID_CHUNK_CHECKSUM,
                    static_cast<unsigned long>(segment->getIndex()),
                    segment->getPosition(), util::toHex(expectedHash).c_str(),
                    util::toHex(actualHash).c_str()));
    segment->clear(getPieceStorage()->getWrDiskCache());
    getSegmentMan()->cancelSegment(getCuid());
    throw DL_RETRY_EX(fmt("Invalid checksum index=%lu",
                          static_cast<unsigned long>(segment->getIndex())));
  }
}

void DownloadCommand::completeSegment(cuid_t cuid,
                                      const std::shared_ptr<Segment>& segment)
{
  flushWrDiskCacheEntry(getPieceStorage()->getWrDiskCache(), segment);
  getSegmentMan()->completeSegment(cuid, segment);
}

void DownloadCommand::installStreamFilter(
    std::unique_ptr<StreamFilter> streamFilter)
{
  if (!streamFilter) {
    return;
  }
  streamFilter->installDelegate(std::move(streamFilter_));
  streamFilter_ = std::move(streamFilter);
  const std::string& name = streamFilter_->getName();
  sinkFilterOnly_ = util::endsWith(name, SinkStreamFilter::NAME);
}

// We need to override noCheck() to return true in order to measure
// download speed to check lowest speed.
bool DownloadCommand::noCheck() const { return lowestDownloadSpeedLimit_ > 0; }

} // namespace aria2
