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
#include <sys/time.h>

DownloadCommand::DownloadCommand(int cuid,
				 const RequestHandle req,
				 DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, e, s), lastSize(0), peerStat(0) {
  peerStat = this->e->segmentMan->getPeerStat(cuid);
  if(!peerStat.get()) {
    peerStat = new PeerStat(cuid);
    this->e->segmentMan->registerPeerStat(peerStat);
  }
  peerStat->downloadStart();
}

DownloadCommand::~DownloadCommand() {
  assert(peerStat.get());
  peerStat->downloadStop();
}

bool DownloadCommand::executeInternal(Segment& segment) {
  if(maxDownloadSpeedLimit > 0 &&
     maxDownloadSpeedLimit < e->segmentMan->calculateDownloadSpeed()) {
    usleep(1);
    e->commands.push_back(this);
    return false;
  }
  TransferEncoding* te = NULL;
  if(transferEncoding.size()) {
    te = getTransferEncoding(transferEncoding);
    assert(te != NULL);
  }
  int bufSize = 16*1024;//4096;
  char buf[bufSize];
  socket->readData(buf, bufSize);
  if(te != NULL) {
    int infbufSize = 16*1024;//4096;
    char infbuf[infbufSize];
    te->inflate(infbuf, infbufSize, buf, bufSize);
    e->segmentMan->diskWriter->writeData(infbuf, infbufSize,
					 segment.getPosition()+segment.writtenLength);
    segment.writtenLength += infbufSize;
    peerStat->updateDownloadLength(infbufSize);
  } else {
    e->segmentMan->diskWriter->writeData(buf, bufSize,
					 segment.getPosition()+segment.writtenLength);
    segment.writtenLength += bufSize;
    peerStat->updateDownloadLength(bufSize);
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
  if(e->segmentMan->totalSize != 0 && bufSize == 0) {
    throw new DlRetryEx(EX_GOT_EOF);
  }
  if(te != NULL && te->finished()
     || te == NULL && segment.complete()
     || bufSize == 0) {
    if(te != NULL) te->end();
    logger->info(MSG_DOWNLOAD_COMPLETED, cuid);
    e->segmentMan->completeSegment(cuid, segment);
    if(e->option->get(PREF_REALTIME_CHUNK_CHECKSUM) == V_TRUE) {
      e->segmentMan->tryChunkChecksumValidation(segment);
    }
    // this unit is going to download another segment.
    return prepareForNextSegment(segment);
  } else {
    e->segmentMan->updateSegment(cuid, segment);
    e->commands.push_back(this);
    return false;
  }
  
}

bool DownloadCommand::prepareForNextSegment(const Segment& currentSegment) {
  if(e->segmentMan->finished()) {
    return true;
  } else {
    // Merge segment with next segment, if segment.index+1 == nextSegment.index
    Segment tempSegment = currentSegment;
    while(1) {
      Segment nextSegment;
      if(e->segmentMan->getSegment(nextSegment, cuid, tempSegment.index+1)) {
	if(nextSegment.writtenLength > 0) {
	  return prepareForRetry(0);
	}
	nextSegment.writtenLength = tempSegment.writtenLength-tempSegment.length;
	if(nextSegment.complete()) {
	  e->segmentMan->completeSegment(cuid, nextSegment);
	  tempSegment = nextSegment;
	} else {
	  e->segmentMan->updateSegment(cuid, nextSegment);
	  e->commands.push_back(this);
	  return false;
	}
      } else {
	break;
      }
    }
    return prepareForRetry(0);
  }
}
