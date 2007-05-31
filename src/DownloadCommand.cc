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
#include "Util.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "HttpInitiateConnectionCommand.h"
#include "InitiateConnectionCommandFactory.h"
#include "message.h"
#include "prefs.h"
#include "ChecksumCommand.h"
#include <sys/time.h>

DownloadCommand::DownloadCommand(int cuid,
				 const RequestHandle req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, requestGroup, e, s),
  peerStat(0),
  transferDecoder(0)
{
  peerStat = _requestGroup->getSegmentMan()->getPeerStat(cuid);
  if(!peerStat.get()) {
    peerStat = new PeerStat(cuid);
    _requestGroup->getSegmentMan()->registerPeerStat(peerStat);
  }
  peerStat->downloadStart();
}

DownloadCommand::~DownloadCommand() {
  assert(peerStat.get());
  peerStat->downloadStop();
}

bool DownloadCommand::executeInternal() {
  // TODO we need to specify the sum of all segmentMan's download speed here.
  if(maxDownloadSpeedLimit > 0 &&
     maxDownloadSpeedLimit < _requestGroup->getSegmentMan()->calculateDownloadSpeed()) {
    usleep(1);
    e->commands.push_back(this);
    return false;
  }
  int32_t bufSize = 16*1024;
  char buf[bufSize];
  socket->readData(buf, bufSize);

  if(transferDecoder.isNull()) {
    _requestGroup->getSegmentMan()->diskWriter->writeData(buf, bufSize,
							  segment->getPositionToWrite());
    segment->writtenLength += bufSize;
    peerStat->updateDownloadLength(bufSize);
  } else {
    int32_t infbufSize = 16*1024;
    char infbuf[infbufSize];
    transferDecoder->inflate(infbuf, infbufSize, buf, bufSize);
    _requestGroup->getSegmentMan()->diskWriter->writeData(infbuf, infbufSize,
							  segment->getPositionToWrite());
    segment->writtenLength += infbufSize;
    peerStat->updateDownloadLength(infbufSize);
  }
  // calculate downloading speed
  if(peerStat->getDownloadStartTime().elapsed(startupIdleTime)) {
    int32_t nowSpeed = peerStat->calculateDownloadSpeed();
    if(lowestDownloadSpeedLimit > 0 &&  nowSpeed <= lowestDownloadSpeedLimit) {
      throw new DlAbortEx("CUID#%d - Too slow Downloading speed: %d <= %d(B/s)",
			  cuid,
			  nowSpeed,
			  lowestDownloadSpeedLimit);
    }
  }
  if(_requestGroup->getSegmentMan()->totalSize != 0 && bufSize == 0) {
    throw new DlRetryEx(EX_GOT_EOF);
  }
  if(!transferDecoder.isNull() && transferDecoder->finished()
     || transferDecoder.isNull() && segment->complete()
     || bufSize == 0) {
    if(!transferDecoder.isNull()) transferDecoder->end();
    logger->info(MSG_DOWNLOAD_COMPLETED, cuid);
    _requestGroup->getSegmentMan()->completeSegment(cuid, segment);
#ifdef ENABLE_MESSAGE_DIGEST
    if(e->option->get(PREF_REALTIME_CHUNK_CHECKSUM) == V_TRUE) {
      _requestGroup->getSegmentMan()->tryChunkChecksumValidation(segment);
    }
#endif // ENABLE_MESSAGE_DIGEST
    // this unit is going to download another segment.
    return prepareForNextSegment();
  } else {
    e->commands.push_back(this);
    return false;
  }
  
}

bool DownloadCommand::prepareForNextSegment() {
  if(_requestGroup->getSegmentMan()->finished()) {
    if(!_requestGroup->getChecksum().isNull() &&
       !_requestGroup->getChecksum()->isEmpty()) {
      ChecksumCommand* command = new ChecksumCommand(cuid, _requestGroup, e);
      e->commands.push_back(command);
    }
    return true;
  } else {
    // Merge segment with next segment, if segment.index+1 == nextSegment.index
    SegmentHandle tempSegment = segment;
    while(1) {
      SegmentHandle nextSegment =
	_requestGroup->getSegmentMan()->getSegment(cuid,
						   tempSegment->index+1);
      if(nextSegment.isNull()) {
	break;
      } else {
	if(nextSegment->writtenLength > 0) {
	  return prepareForRetry(0);
	}
	nextSegment->writtenLength =
	  tempSegment->writtenLength-tempSegment->length;
	if(nextSegment->complete()) {
	  _requestGroup->getSegmentMan()->completeSegment(cuid, nextSegment);
	  tempSegment = nextSegment;
	} else {
	  segment = nextSegment;
	  e->commands.push_back(this);
	  return false;
	}
      }
    }
    return prepareForRetry(0);
  }
}
