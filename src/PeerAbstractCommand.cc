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
#include "PeerAbstractCommand.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include <sys/time.h>
#include "Util.h"
#include "message.h"
#include "prefs.h"

PeerAbstractCommand::PeerAbstractCommand(int cuid, Peer* peer, TorrentDownloadEngine* e, const Socket* s):
  Command(cuid), e(e), peer(peer), checkSocketIsReadable(false), checkSocketIsWritable(false) {

  if(s != NULL) {
    socket = new Socket(*s);
    setReadCheckSocket(socket);
  } else {
    socket = NULL;
  }
  this->checkPoint.tv_sec = 0;
  this->checkPoint.tv_usec = 0;
  timeout = e->option->getAsInt(PREF_TIMEOUT);
  e->torrentMan->connections++;
}

PeerAbstractCommand::~PeerAbstractCommand() {
  setReadCheckSocket(NULL);
  setWriteCheckSocket(NULL);
  if(socket != NULL) {
    delete(socket);
  }
  e->torrentMan->connections--;
}

void PeerAbstractCommand::updateCheckPoint() {
  gettimeofday(&checkPoint, NULL);
}

bool PeerAbstractCommand::isTimeoutDetected() {
  struct timeval now;
  gettimeofday(&now, NULL);
  if(checkPoint.tv_sec == 0 && checkPoint.tv_usec == 0) {
    checkPoint = now;
    return false;
  } else {
    long long int elapsed = Util::difftv(now, checkPoint);
    if(elapsed >= ((long long int)timeout)*1000000) {
      return true;
    } else {
      return false;
    }
  }
}

bool PeerAbstractCommand::execute() {
  try {
    beforeSocketCheck();
    if(checkSocketIsReadable && !readCheckTarget->isReadable(0)
       || checkSocketIsWritable && !writeCheckTarget->isWritable(0)) {
      if(isTimeoutDetected()) {
	// TODO
	checkPoint.tv_sec = 0;
	checkPoint.tv_usec = 0;
	throw new DlRetryEx(EX_TIME_OUT);
      }
      e->commands.push(this);
      return false;
    }
    updateCheckPoint();
    bool returnValue =  executeInternal();
    //e->torrentMan->updatePeer(peer);
    return returnValue;
  } catch(Exception* err) {
    e->logger->error(MSG_DOWNLOAD_ABORTED, err, cuid);
    onAbort(err);
    delete(err);
    return prepareForNextPeer(0);
  }
  /*catch(DlRetryEx* err) {
    e->logger->error(MSG_RESTARTING_DOWNLOAD, err, cuid);
    peer->tryCount++;
    bool isAbort = e->option->getAsInt(PREF_MAX_TRIES) != 0 &&
      peer->tryCount >= e->option->getAsInt(PREF_MAX_TRIES);
    int tryCount = peer->tryCount;
    if(isAbort) {
      onAbort(err);
    }
    delete(err);
    if(isAbort) {
      e->logger->error(MSG_MAX_TRY, cuid, tryCount);
      return true;
    } else {
      return prepareForRetry(e->option->getAsInt(PREF_RETRY_WAIT));
    }
  }
  */
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerAbstractCommand::prepareForNextPeer(int wait) {
  return true;
}

bool PeerAbstractCommand::prepareForRetry(int wait) {
  return true;
}

void PeerAbstractCommand::onAbort(Exception* ex) {
  if(peer->isSeeder()) {
    peer->error++;
  } else {
    peer->error += MAX_PEER_ERROR;
  }
  peer->tryCount = 0;
  peer->cuid = 0;
  peer->amChocking = true;
  peer->amInterested = false;
  peer->peerChoking = true;
  peer->peerInterested = false;
  e->logger->debug("CUID#%d - peer %s:%d banned.", cuid, peer->ipaddr.c_str(), peer->port);
}

void PeerAbstractCommand::setReadCheckSocket(Socket* socket) {
  if(socket == NULL) {
    if(checkSocketIsReadable) {
      e->deleteSocketForReadCheck(readCheckTarget);
      checkSocketIsReadable = false;
      readCheckTarget = NULL;
    }
  } else {
    if(checkSocketIsReadable) {
      if(readCheckTarget != socket) {
	e->deleteSocketForReadCheck(readCheckTarget);
	e->addSocketForReadCheck(socket);
	readCheckTarget = socket;
      }
    } else {
      e->addSocketForReadCheck(socket);
      checkSocketIsReadable = true;
      readCheckTarget = socket;
    }
  }
}

void PeerAbstractCommand::setWriteCheckSocket(Socket* socket) {
  if(socket == NULL) {
    if(checkSocketIsWritable) {
      e->deleteSocketForWriteCheck(writeCheckTarget);
      checkSocketIsWritable = false;
      writeCheckTarget = NULL;
    }
  } else {
    if(checkSocketIsWritable) {
      if(writeCheckTarget != socket) {
	e->deleteSocketForWriteCheck(writeCheckTarget);
	e->addSocketForWriteCheck(socket);
	writeCheckTarget = socket;
      }
    } else {
      e->addSocketForWriteCheck(socket);
      checkSocketIsWritable = true;
      writeCheckTarget = socket;
    }
  }
}
