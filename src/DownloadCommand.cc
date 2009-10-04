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
#include "RequestGroupMan.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

static const size_t BUFSIZE = 16*1024;

DownloadCommand::DownloadCommand(int cuid,
				 const RequestHandle& req,
				 const SharedHandle<FileEntry>& fileEntry,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
  _buf(new unsigned char[BUFSIZE])
#ifdef ENABLE_MESSAGE_DIGEST
  , _pieceHashValidationEnabled(false)
#endif // ENABLE_MESSAGE_DIGEST
{
#ifdef ENABLE_MESSAGE_DIGEST
  {
    if(getOption()->getAsBool(PREF_REALTIME_CHUNK_CHECKSUM)) {
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

  peerStat = req->initPeerStat();
  peerStat->downloadStart();
  _requestGroup->getSegmentMan()->registerPeerStat(peerStat);
}

DownloadCommand::~DownloadCommand() {
  peerStat->downloadStop();
  _requestGroup->getSegmentMan()->updateFastestPeerStat(peerStat);
  delete [] _buf;
}

bool DownloadCommand::executeInternal() {
  if(e->_requestGroupMan->doesOverallDownloadSpeedExceed() ||
     _requestGroup->doesDownloadSpeedExceed()) {
    e->commands.push_back(this);
    disableReadCheckSocket();
    return false;
  }
  setReadCheckSocket(socket);
  SegmentHandle segment = _segments.front();

  size_t bufSize;
  if(segment->getLength() > 0) {
    if(static_cast<uint64_t>(segment->getPosition()+segment->getLength()) <=
       static_cast<uint64_t>(_fileEntry->getLastOffset())) {
      bufSize = std::min(segment->getLength()-segment->getWrittenLength(),
			 BUFSIZE);
    } else {
      bufSize = std::min(static_cast<size_t>(_fileEntry->getLastOffset()-segment->getPositionToWrite()), BUFSIZE);
    }
  } else {
    bufSize = BUFSIZE;
  }
  socket->readData(_buf, bufSize);

  const SharedHandle<DiskAdaptor>& diskAdaptor =
    _requestGroup->getPieceStorage()->getDiskAdaptor();

  const unsigned char* bufFinal;
  size_t bufSizeFinal;

  std::string decoded;
  if(_transferEncodingDecoder.isNull()) {
    bufFinal = _buf;
    bufSizeFinal = bufSize;
  } else {
    decoded = _transferEncodingDecoder->decode(_buf, bufSize);

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
  if(bufSizeFinal > 0) {
    segment->updateWrittenLength(bufSizeFinal);
  }

  peerStat->updateDownloadLength(bufSize);

  _requestGroup->getSegmentMan()->updateDownloadSpeedFor(peerStat);

  bool segmentPartComplete = false;
  // Note that GrowSegment::complete() always returns false.
  if(_transferEncodingDecoder.isNull() && _contentEncodingDecoder.isNull()) {
    if(segment->complete() ||
       segment->getPositionToWrite() == _fileEntry->getLastOffset()) {
      segmentPartComplete = true;
    } else if(segment->getLength() == 0 && bufSize == 0 &&
	      !socket->wantRead() && !socket->wantWrite()) {
      segmentPartComplete = true;
    }
  } else if(!_transferEncodingDecoder.isNull() &&
	    (segment->complete() || segment->getPositionToWrite() == _fileEntry->getLastOffset())) {
    segmentPartComplete = true;
  } else if((_transferEncodingDecoder.isNull() ||
	     _transferEncodingDecoder->finished()) &&
	    (_contentEncodingDecoder.isNull() ||
	     _contentEncodingDecoder->finished())) {
    segmentPartComplete = true;
  }

  if(!segmentPartComplete && bufSize == 0 &&
     !socket->wantRead() && !socket->wantWrite()) {
    throw DL_RETRY_EX(EX_GOT_EOF);
  }

  if(segmentPartComplete) {
    if(segment->complete() || segment->getLength() == 0) {
      // If segment->getLength() == 0, the server doesn't provide
      // content length, but the client detected that download
      // completed.
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
    } else {
      // If segment is not cacnel here, in the next pipelining
      // request, aria2 requests bad range
      // [FileEntry->getLastOffset(), FileEntry->getLastOffset())
      _requestGroup->getSegmentMan()->cancelSegment(cuid, segment);
    }
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
      throw DL_ABORT_EX2(StringFormat(EX_TOO_SLOW_DOWNLOAD_SPEED,
				      nowSpeed,
				      lowestDownloadSpeedLimit,
				      req->getHost().c_str()).str(),
			 downloadresultcode::TOO_SLOW_DOWNLOAD_SPEED);
    }
  }
}

bool DownloadCommand::prepareForNextSegment() {
  if(_requestGroup->downloadFinished()) {
    // If this is a single file download, and file size becomes known
    // just after downloading, set total length to FileEntry object
    // here.
    if(_requestGroup->getDownloadContext()->getFileEntries().size() == 1) {
      const SharedHandle<FileEntry>& fileEntry =
	_requestGroup->getDownloadContext()->getFirstFileEntry();
      if(fileEntry->getLength() == 0) {
	fileEntry->setLength
	  (_requestGroup->getPieceStorage()->getCompletedLength());
      }
    }
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
      if(!tempSegment->complete()) {
	return prepareForRetry(0);
      }
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
    throw DL_RETRY_EX
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
