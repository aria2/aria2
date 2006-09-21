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
#include "Util.h"
#include "message.h"
#include "prefs.h"

PeerAbstractCommand::PeerAbstractCommand(int cuid, const PeerHandle& peer,
					 TorrentDownloadEngine* e,
					 const SocketHandle& s)
  :Command(cuid), e(e), socket(s), peer(peer),
  checkSocketIsReadable(false), checkSocketIsWritable(false),
  uploadLimitCheck(false), uploadLimit(0), noCheck(false) {
  setReadCheckSocket(socket);
  timeout = e->option->getAsInt(PREF_TIMEOUT);
  e->torrentMan->connections++;
}

PeerAbstractCommand::~PeerAbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
  e->torrentMan->connections--;
}

bool PeerAbstractCommand::execute() {
  if(e->torrentMan->isHalt()) {
    return true;
  }
  try {
    if(noCheck ||
       /*
       uploadLimitCheck && (uploadLimit == 0 ||
			    e->getUploadSpeed() <= uploadLimit*1024) ||
       */
       checkSocketIsReadable && readCheckTarget->isReadable(0) ||
       checkSocketIsWritable && writeCheckTarget->isWritable(0)) {
      checkPoint.reset();
    }
    if(checkPoint.elapsed(timeout)) {
      throw new DlRetryEx(EX_TIME_OUT);
    }
    return executeInternal();
  } catch(Exception* err) {
    logger->error(MSG_DOWNLOAD_ABORTED, err, cuid);
    logger->debug("CUID#%d - Peer %s:%d banned.",
		  cuid, peer->ipaddr.c_str(), peer->port);
    onAbort(err);
    delete err;
    return prepareForNextPeer(0);
  }
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
}

void PeerAbstractCommand::disableReadCheckSocket() {
  if(checkSocketIsReadable) {
    e->deleteSocketForReadCheck(readCheckTarget, this);
    checkSocketIsReadable = false;
    readCheckTarget = SocketHandle();
  }  
}

void PeerAbstractCommand::setReadCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableReadCheckSocket();
  } else {
    if(checkSocketIsReadable) {
      if(readCheckTarget != socket) {
	e->deleteSocketForReadCheck(readCheckTarget, this);
	e->addSocketForReadCheck(socket, this);
	readCheckTarget = socket;
      }
    } else {
      e->addSocketForReadCheck(socket, this);
      checkSocketIsReadable = true;
      readCheckTarget = socket;
    }
  }
}

void PeerAbstractCommand::disableWriteCheckSocket() {
  if(checkSocketIsWritable) {
    e->deleteSocketForWriteCheck(writeCheckTarget, this);
    checkSocketIsWritable = false;
    writeCheckTarget = SocketHandle();
  }
}

void PeerAbstractCommand::setWriteCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableWriteCheckSocket();
  } else {
    if(checkSocketIsWritable) {
      if(writeCheckTarget != socket) {
	e->deleteSocketForWriteCheck(writeCheckTarget, this);
	e->addSocketForWriteCheck(socket, this);
	writeCheckTarget = socket;
      }
    } else {
      e->addSocketForWriteCheck(socket, this);
      checkSocketIsWritable = true;
      writeCheckTarget = socket;
    }
  }
}

void PeerAbstractCommand::setUploadLimit(int uploadLimit) {
  this->uploadLimit = uploadLimit;
}

void PeerAbstractCommand::setUploadLimitCheck(bool check) {
  this->uploadLimitCheck = check;
}

void PeerAbstractCommand::setNoCheck(bool check) {
  this->noCheck = check;
}
