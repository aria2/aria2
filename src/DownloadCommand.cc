/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#define STARTUP_IDLE_TIME 10

DownloadCommand::DownloadCommand(int cuid, Request* req, DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, e, s), lastSize(0) {
  PeerStatHandle peerStat = PeerStatHandle(new PeerStat(cuid));
  peerStat->downloadStart();
  this->e->segmentMan->registerPeerStat(peerStat);
}

DownloadCommand::~DownloadCommand() {
  PeerStatHandle peerStat = e->segmentMan->getPeerStat(cuid);
  assert(peerStat.get());
  peerStat->downloadStop();
}

bool DownloadCommand::executeInternal(Segment& segment) {
  int maxSpeedLimit = e->option->getAsInt(PREF_MAX_SPEED_LIMIT);
  if(maxSpeedLimit > 0 &&
     maxSpeedLimit < e->segmentMan->calculateDownloadSpeed()) {
    usleep(1);
    e->commands.push_back(this);
    return false;
  }
  TransferEncoding* te = NULL;
  if(transferEncoding.size()) {
    te = getTransferEncoding(transferEncoding);
    assert(te != NULL);
  }
  int bufSize = 4096;
  char buf[bufSize];
  socket->readData(buf, bufSize);
  PeerStatHandle peerStat = e->segmentMan->getPeerStat(cuid);
  assert(peerStat.get());
  if(te != NULL) {
    int infbufSize = 4096;
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
  if(/*sw.elapsed(1) >= 1 && */peerStat->getDownloadStartTime().elapsed(STARTUP_IDLE_TIME)) {
    int lowestLimit = e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT);
    int nowSpeed = peerStat->calculateDownloadSpeed();
    if(lowestLimit > 0 &&  nowSpeed <= lowestLimit) {
      throw new DlAbortEx("CUID#%d - Too slow Downloading speed: %d <= %d(B/s)",
			  cuid,
			  nowSpeed,
			  lowestLimit);
    }
    //sw.reset();
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
