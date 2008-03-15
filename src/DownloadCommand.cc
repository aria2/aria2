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
#include "DownloadCommand.h"
#include "Request.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "PeerStat.h"
#include "TransferEncoding.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "SegmentMan.h"
#include "Segment.h"
#include "Logger.h"
#include "ChecksumCheckIntegrityEntry.h"
#include "PieceStorage.h"
#include "CheckIntegrityCommand.h"
#include "DiskAdaptor.h"
#include "CUIDCounter.h"
#include "DownloadContext.h"
#include "Option.h"
#include "Util.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <cassert>

namespace aria2 {

DownloadCommand::DownloadCommand(int cuid,
				 const RequestHandle& req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, requestGroup, e, s),
  peerStat(0),
#ifdef ENABLE_MESSAGE_DIGEST
  _messageDigestContext(0),
#endif // ENABLE_MESSAGE_DIGEST
  transferDecoder(0)
{
#ifdef ENABLE_MESSAGE_DIGEST
  {
    std::string algo = _requestGroup->getDownloadContext()->getPieceHashAlgo();
    if(MessageDigestContext::supports(algo)) {
      _messageDigestContext = new MessageDigestContext();
      _messageDigestContext->trySetAlgo(algo);
      _messageDigestContext->digestInit();
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  peerStat = _requestGroup->getSegmentMan()->getPeerStat(cuid);
  if(peerStat.isNull()) {
    peerStat = new PeerStat(cuid);
    _requestGroup->getSegmentMan()->registerPeerStat(peerStat);
  }
  peerStat->downloadStart();
}

DownloadCommand::~DownloadCommand() {
  assert(peerStat.get());
  peerStat->downloadStop();
#ifdef ENABLE_MESSAGE_DIGEST
  delete _messageDigestContext;
#endif // ENABLE_MESSAGE_DIGEST
}

bool DownloadCommand::executeInternal() {
  if(maxDownloadSpeedLimit > 0 &&
     maxDownloadSpeedLimit < _requestGroup->getSegmentMan()->calculateDownloadSpeed()) {
    e->commands.push_back(this);
    disableReadCheckSocket();
    return false;
  }
  setReadCheckSocket(socket);
  SegmentHandle segment = _segments.front();

  size_t BUFSIZE = 16*1024;
  unsigned char buf[BUFSIZE];
  size_t bufSize;
  if(segment->getLength() > 0 && segment->getLength()-segment->getWrittenLength() < BUFSIZE) {
    bufSize = segment->getLength()-segment->getWrittenLength();
  } else {
    bufSize = BUFSIZE;
  }
  socket->readData(buf, bufSize);

  if(transferDecoder.isNull()) {
    _requestGroup->getPieceStorage()->getDiskAdaptor()->writeData(buf, bufSize,
								  segment->getPositionToWrite());
    //logger->debug("bufSize = %d, posToWrite = %lld", bufSize, segment->getPositionToWrite());
    segment->updateWrittenLength(bufSize);
    //logger->debug("overflow length = %d, next posToWrite = %lld", segment->getOverflowLength(), segment->getPositionToWrite());
    //logger->debug("%s", Util::toHex(segment->getPiece()->getBitfield(),
    //segment->getPiece()->getBitfieldLength()).c_str());
    //segment->writtenLength += bufSize;
    peerStat->updateDownloadLength(bufSize);
  } else {
    size_t infbufSize = 16*1024;
    unsigned char infbuf[infbufSize];
    transferDecoder->inflate(infbuf, infbufSize, buf, bufSize);
    _requestGroup->getPieceStorage()->getDiskAdaptor()->writeData(infbuf, infbufSize,
								  segment->getPositionToWrite());
    segment->updateWrittenLength(infbufSize);
    //segment->writtenLength += infbufSize;
    peerStat->updateDownloadLength(infbufSize);
  }
  if(_requestGroup->getTotalLength() != 0 && bufSize == 0) {
    throw new DlRetryEx(EX_GOT_EOF);
  }
  if((!transferDecoder.isNull() && transferDecoder->finished())
     || (transferDecoder.isNull() && segment->complete())
     || bufSize == 0) {
    if(!transferDecoder.isNull()) transferDecoder->end();
    logger->info(MSG_SEGMENT_DOWNLOAD_COMPLETED, cuid);
    validatePieceHash(segment);
    checkLowestDownloadSpeed();
    // this unit is going to download another segment.
    return prepareForNextSegment();
  } else {
    checkLowestDownloadSpeed();
    e->commands.push_back(this);
    return false;
  }
}

void DownloadCommand::checkLowestDownloadSpeed() const
{
  // calculate downloading speed
  if(peerStat->getDownloadStartTime().elapsed(startupIdleTime)) {
    unsigned int nowSpeed = peerStat->calculateDownloadSpeed();
    if(lowestDownloadSpeedLimit > 0 &&  nowSpeed <= lowestDownloadSpeedLimit) {
      throw new DlAbortEx(EX_TOO_SLOW_DOWNLOAD_SPEED,
			  nowSpeed,
			  lowestDownloadSpeedLimit,
			  req->getHost().c_str());
    }
  }
}

bool DownloadCommand::prepareForNextSegment() {
  if(_requestGroup->downloadFinished()) {
#ifdef ENABLE_MESSAGE_DIGEST
    CheckIntegrityEntryHandle entry = new ChecksumCheckIntegrityEntry(_requestGroup);
    if(entry->isValidationReady()) {
      entry->initValidator();
      CheckIntegrityCommand* command =
	new CheckIntegrityCommand(CUIDCounterSingletonHolder::instance()->newID(), _requestGroup, e, entry);
      e->commands.push_back(command);
    }
#endif // ENABLE_MESSAGE_DIGEST
    return true;
  } else {
    SegmentHandle tempSegment = _segments.front();
    SegmentHandle nextSegment =
      _requestGroup->getSegmentMan()->getSegment(cuid,
						 tempSegment->getIndex()+1);
    if(!nextSegment.isNull() && nextSegment->getWrittenLength() == 0) {
      e->commands.push_back(this);
      return false;
    } else {
      return prepareForRetry(0);
    }
  }
}

void DownloadCommand::validatePieceHash(const SegmentHandle& segment)
{
#ifdef ENABLE_MESSAGE_DIGEST
  std::string expectedPieceHash =
    _requestGroup->getDownloadContext()->getPieceHash(segment->getIndex());
  if(_messageDigestContext &&
     e->option->get(PREF_REALTIME_CHUNK_CHECKSUM) == V_TRUE &&
     !expectedPieceHash.empty()) {
    _messageDigestContext->digestReset();
    std::string actualPieceHash =
      MessageDigestHelper::digest(_messageDigestContext,
				  _requestGroup->getPieceStorage()->getDiskAdaptor(),
				  segment->getPosition(),
				  segment->getLength());
    if(actualPieceHash == expectedPieceHash) {
      logger->info(MSG_GOOD_CHUNK_CHECKSUM, actualPieceHash.c_str());
      _requestGroup->getSegmentMan()->completeSegment(cuid, segment);
    } else {
      logger->info(EX_INVALID_CHUNK_CHECKSUM,
		   segment->getIndex(),
		   Util::itos(segment->getPosition(), true).c_str(),
		   expectedPieceHash.c_str(),
		   actualPieceHash.c_str());
      segment->clear();
      _requestGroup->getSegmentMan()->cancelSegment(cuid);
      throw new DlRetryEx("Invalid checksum index=%d", segment->getIndex());
    }
  } else
#endif // ENABLE_MESSAGE_DIGEST
    {
      _requestGroup->getSegmentMan()->completeSegment(cuid, segment);
    }
}

void DownloadCommand::setTransferDecoder(const TransferEncodingHandle& transferDecoder)
{
  this->transferDecoder = transferDecoder;
}

} // namespace aria2
