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
#include "Util.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "StringFormat.h"
#include "Decoder.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

DownloadCommand::DownloadCommand(int cuid,
				 const RequestHandle& req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, requestGroup, e, s)
#ifdef ENABLE_MESSAGE_DIGEST
  , _pieceHashValidationEnabled(false)
#endif // ENABLE_MESSAGE_DIGEST
{
#ifdef ENABLE_MESSAGE_DIGEST
  {
    if(e->option->getAsBool(PREF_REALTIME_CHUNK_CHECKSUM)) {
      std::string algo = _requestGroup->getDownloadContext()->getPieceHashAlgo();
      if(MessageDigestContext::supports(algo)) {
	_messageDigestContext.reset(new MessageDigestContext());
	_messageDigestContext->trySetAlgo(algo);
	_messageDigestContext->digestInit();
	
	_pieceHashValidationEnabled = true;
      }
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  peerStat = _requestGroup->getSegmentMan()->getPeerStat(cuid);
  if(peerStat.isNull()) {
    peerStat.reset(new PeerStat(cuid, req->getHost(), req->getProtocol()));
    _requestGroup->getSegmentMan()->registerPeerStat(peerStat);
  }
  peerStat->downloadStart();
}

DownloadCommand::~DownloadCommand() {
  assert(peerStat.get());
  peerStat->downloadStop();
}

bool DownloadCommand::executeInternal() {
  if(maxDownloadSpeedLimit > 0 &&
     maxDownloadSpeedLimit <
     _requestGroup->calculateStat().getDownloadSpeed()) {
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

  const SharedHandle<DiskAdaptor>& diskAdaptor =
    _requestGroup->getPieceStorage()->getDiskAdaptor();

  const unsigned char* bufFinal;
  size_t bufSizeFinal;

  std::string decoded;
  if(_transferEncodingDecoder.isNull()) {
    bufFinal = buf;
    bufSizeFinal = bufSize;
  } else {
    decoded = _transferEncodingDecoder->decode(buf, bufSize);

    bufFinal = reinterpret_cast<const unsigned char*>(decoded.c_str());
    bufSizeFinal = decoded.size();
  }

  if(_contentEncodingDecoder.isNull()) {
    diskAdaptor->writeData(bufFinal, bufSizeFinal,
			   segment->getPositionToWrite());
  } else {
    std::string out = _contentEncodingDecoder->decode(bufFinal, bufSizeFinal);
    diskAdaptor->writeData(reinterpret_cast<const unsigned char*>(out.data()),
			   out.size(),
			   segment->getPositionToWrite());
    bufSizeFinal = out.size();
  }

#ifdef ENABLE_MESSAGE_DIGEST

  if(_pieceHashValidationEnabled) {
    segment->updateHash(segment->getWrittenLength(), bufFinal, bufSizeFinal);
  }

#endif // ENABLE_MESSAGE_DIGEST

  segment->updateWrittenLength(bufSizeFinal);
  
  peerStat->updateDownloadLength(bufSize);

  if(_requestGroup->getTotalLength() != 0 && bufSize == 0 &&
     !socket->wantRead() && !socket->wantWrite()) {
    throw DlRetryEx(EX_GOT_EOF);
  }
  bool segmentComplete = false;
  if(_transferEncodingDecoder.isNull() && _contentEncodingDecoder.isNull()) {
    if(segment->complete()) {
      segmentComplete = true;
    } else if(segment->getLength() == 0 && bufSize == 0 &&
	      !socket->wantRead() && !socket->wantWrite()) {
      segmentComplete = true;
    }
  } else if(!_transferEncodingDecoder.isNull() &&
	    !_contentEncodingDecoder.isNull()) {
    if(_transferEncodingDecoder->finished() &&
       _contentEncodingDecoder->finished()) {
      segmentComplete = true;
    }
  } else if(!_transferEncodingDecoder.isNull() &&
	    _contentEncodingDecoder.isNull()) {
    if(_transferEncodingDecoder->finished()) {
      segmentComplete = true;
    }
  } else if(_transferEncodingDecoder.isNull() &&
	    !_contentEncodingDecoder.isNull()) {
    if(_contentEncodingDecoder->finished()) {
      segmentComplete = true;
    }
  }

  if(segmentComplete) {
    logger->info(MSG_SEGMENT_DOWNLOAD_COMPLETED, cuid);
#ifdef ENABLE_MESSAGE_DIGEST

    {
      std::string expectedPieceHash =
	_requestGroup->getDownloadContext()->getPieceHash(segment->getIndex());
      if(_pieceHashValidationEnabled && !expectedPieceHash.empty()) {
	if(segment->isHashCalculated()) {
	  logger->debug("Hash is available! index=%lu",
			static_cast<unsigned long>(segment->getIndex()));
	  validatePieceHash(segment, expectedPieceHash, segment->getHashString());
	} else {
	  _messageDigestContext->digestReset();
	  validatePieceHash(segment, expectedPieceHash,
			    MessageDigestHelper::digest
			    (_messageDigestContext.get(),
			     _requestGroup->getPieceStorage()->getDiskAdaptor(),
			     segment->getPosition(),
			     segment->getLength()));
	}
      } else {
	_requestGroup->getSegmentMan()->completeSegment(cuid, segment);
      }
    }

#else // !ENABLE_MESSAGE_DIGEST

    _requestGroup->getSegmentMan()->completeSegment(cuid, segment);

#endif // !ENABLE_MESSAGE_DIGEST

    checkLowestDownloadSpeed();
    // this unit is going to download another segment.
    return prepareForNextSegment();
  } else {
    checkLowestDownloadSpeed();
    setWriteCheckSocketIf(socket, socket->wantWrite());
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
      throw DlAbortEx(StringFormat(EX_TOO_SLOW_DOWNLOAD_SPEED,
				   nowSpeed,
				   lowestDownloadSpeedLimit,
				   req->getHost().c_str()).str(), DownloadResult::TOO_SLOW_DOWNLOAD_SPEED);
    }
  }
}

bool DownloadCommand::prepareForNextSegment() {
  if(_requestGroup->downloadFinished()) {
#ifdef ENABLE_MESSAGE_DIGEST
    CheckIntegrityEntryHandle entry(new ChecksumCheckIntegrityEntry(_requestGroup));
    if(entry->isValidationReady()) {
      entry->initValidator();
      // TODO do we need cuttrailinggarbage here?
      e->_checkIntegrityMan->pushEntry(entry);
    }
    e->setNoWait(true);
    e->setRefreshInterval(0);
#endif // ENABLE_MESSAGE_DIGEST
    return true;
  } else {
    // The number of segments should be 1 in order to pass through the next
    // segment.
    if(_segments.size() == 1) {
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
    throw DlRetryEx
      (StringFormat("Invalid checksum index=%d", segment->getIndex()).str());
  }
}

#endif // ENABLE_MESSAGE_DIGEST

void DownloadCommand::setTransferEncodingDecoder
(const SharedHandle<Decoder>& decoder)
{
  this->_transferEncodingDecoder = decoder;
}

void DownloadCommand::setContentEncodingDecoder
(const SharedHandle<Decoder>& decoder)
{
  _contentEncodingDecoder = decoder;
}

} // namespace aria2
