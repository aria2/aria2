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
#include "PeerInteractionCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "message.h"
#include "prefs.h"
#include <algorithm>

PeerInteractionCommand::PeerInteractionCommand(int cuid, Peer* peer,
					       TorrentDownloadEngine* e,
					       const Socket* s, int sequence)
  :PeerAbstractCommand(cuid, peer, e, s), sequence(sequence) {
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    setReadCheckSocket(NULL);
    setWriteCheckSocket(socket);
    setTimeout(e->option->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));
  }
  peerInteraction = new PeerInteraction(cuid, socket, e->option,
					e->torrentMan, this->peer);
  peerInteraction->setUploadLimit(e->option->getAsInt(PREF_UPLOAD_LIMIT));
  setUploadLimit(e->option->getAsInt(PREF_UPLOAD_LIMIT));
  keepAliveCheckPoint.tv_sec = 0;
  keepAliveCheckPoint.tv_usec = 0;
  chokeCheckPoint.tv_sec = 0;
  chokeCheckPoint.tv_usec = 0;
  freqCheckPoint.tv_sec = 0;
  freqCheckPoint.tv_usec = 0;
  chokeUnchokeCount = 0;
  haveCount = 0;
  keepAliveCount = 0;
  e->torrentMan->addActivePeer(this->peer);
}

PeerInteractionCommand::~PeerInteractionCommand() {
  delete peerInteraction;
  e->torrentMan->unadvertisePiece(cuid);
  e->torrentMan->deleteActivePeer(this->peer);
}

bool PeerInteractionCommand::executeInternal() {
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    socket->setBlockingMode();
    setReadCheckSocket(socket);
    setTimeout(e->option->getAsInt(PREF_TIMEOUT));
  }
  setWriteCheckSocket(NULL);
  setUploadLimitCheck(false);

  switch(sequence) {
  case INITIATOR_SEND_HANDSHAKE:
    peerInteraction->sendHandshake();
    sequence = INITIATOR_WAIT_HANDSHAKE;
    break;
  case INITIATOR_WAIT_HANDSHAKE: {
    if(peerInteraction->countMessageInQueue() > 0) {
      peerInteraction->sendMessages(e->getUploadSpeed());
      if(peerInteraction->countMessageInQueue() > 0) {
	break;
      }
    }
    HandshakeMessage* handshakeMessage = peerInteraction->receiveHandshake();
    if(handshakeMessage == NULL) {
      break;
    }
    peer->setPeerId(handshakeMessage->peerId);
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 handshakeMessage->toString().c_str());
    delete handshakeMessage;
    peerInteraction->sendBitfield();
    peerInteraction->sendAllowedFast();
    sequence = WIRED;
    break;
  }
  case RECEIVER_WAIT_HANDSHAKE: {
    HandshakeMessage* handshakeMessage = peerInteraction->receiveHandshake();
    if(handshakeMessage == NULL) {
      break;
    }
    peer->setPeerId(handshakeMessage->peerId);
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 handshakeMessage->toString().c_str());
    delete handshakeMessage;
    peerInteraction->sendHandshake();
    peerInteraction->sendBitfield();
    peerInteraction->sendAllowedFast();
    sequence = WIRED;    
    break;
  }
  case WIRED:
    peerInteraction->syncPiece();
    decideChoking();
    receiveMessages();
    detectMessageFlooding();
    //checkLongTimePeerChoking();

    peerInteraction->deleteTimeoutRequestSlot();
    peerInteraction->deleteCompletedRequestSlot();
    peerInteraction->addRequests();
    peerInteraction->sendMessages(e->getUploadSpeed());
    break;
  }
  if(peerInteraction->countMessageInQueue() > 0) {
    if(peerInteraction->isSendingMessageInProgress()) {
      setWriteCheckSocket(socket);
    } else {
      setUploadLimitCheck(true);
    }
  }
  e->commands.push_back(this);
  return false;
}

void PeerInteractionCommand::detectMessageFlooding() {
  struct timeval now;
  gettimeofday(&now, NULL);
  if(freqCheckPoint.tv_sec == 0 && freqCheckPoint.tv_usec == 0) {
    freqCheckPoint = now;
  } else {
    int elapsed = Util::difftvsec(now, freqCheckPoint);
    if(elapsed >= 5) {
      if(chokeUnchokeCount*1.0/elapsed >= 0.4
	 //|| haveCount*1.0/elapsed >= 20.0
	 || keepAliveCount*1.0/elapsed >= 1.0) {
	throw new DlAbortEx("Flooding detected.");
      } else {
	chokeUnchokeCount = 0;
	haveCount = 0;
	keepAliveCount = 0;
	freqCheckPoint = now;
      }
    }
  }
}

void PeerInteractionCommand::checkLongTimePeerChoking() {
  if(e->torrentMan->downloadComplete()) {
    return;
  }    
  struct timeval now;
  gettimeofday(&now, NULL);
  if(chokeCheckPoint.tv_sec == 0 && chokeCheckPoint.tv_usec == 0) {
    if(peer->amInterested && peer->peerChoking) {
      chokeCheckPoint = now;
    }
  } else {
    if(peer->amInterested && peer->peerChoking) {
      if(Util::difftvsec(now, chokeCheckPoint) >= MAX_PEER_CHOKING_INTERVAL) {
	throw new DlAbortEx("Too long choking.");
      }
    } else {
      chokeCheckPoint.tv_sec = 0;
      chokeCheckPoint.tv_usec = 0;
    }
  }
}

void PeerInteractionCommand::decideChoking() {
  if(peer->shouldBeChoking()) {
    if(!peer->amChoking) {
      peerInteraction->addMessage(peerInteraction->createChokeMessage());
    }
  } else {
    if(peer->amChoking) {
      peerInteraction->addMessage(peerInteraction->createUnchokeMessage());
    }
  }
}

void PeerInteractionCommand::receiveMessages() {
  for(int i = 0; i < 50; i++) {
    PeerMessage* message = peerInteraction->receiveMessage();
    if(message == NULL) {
      return;
    }
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 message->toString().c_str());
    // to detect flooding
    switch(message->getId()) {
    case KeepAliveMessage::ID:
      keepAliveCount++;
      break;
    case ChokeMessage::ID:
      if(!peer->peerChoking) {
	chokeUnchokeCount++;
      }
      break;
    case UnchokeMessage::ID:
      if(peer->peerChoking) {
	chokeUnchokeCount++;
      }
      break;
    case HaveMessage::ID:
      haveCount++;
      break;
    }
    try {
      message->receivedAction();
      delete message;
    } catch(Exception* ex) {
      delete message;
      throw;
    }
  }
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(int wait) {
  if(e->torrentMan->isPeerAvailable()) {
    Peer* peer = e->torrentMan->getPeer();
    int newCuid = e->torrentMan->getNewCuid();
    peer->cuid = newCuid;
    PeerInitiateConnectionCommand* command = new PeerInitiateConnectionCommand(newCuid, peer, e);
    e->commands.push_back(command);
  }
  return true;
}

bool PeerInteractionCommand::prepareForRetry(int wait) {
  e->commands.push_back(this);
  return false;
}

void PeerInteractionCommand::onAbort(Exception* ex) {
  peerInteraction->abortAllPieces();
  PeerAbstractCommand::onAbort(ex);
}

void PeerInteractionCommand::keepAlive() {
  if(keepAliveCheckPoint.tv_sec == 0 && keepAliveCheckPoint.tv_usec == 0) {
    gettimeofday(&keepAliveCheckPoint, NULL);
  } else {
    struct timeval now;
    gettimeofday(&now, NULL);
    if(Util::difftvsec(now, keepAliveCheckPoint) >= 120) {
      if(peerInteraction->countMessageInQueue() == 0) {
	peerInteraction->addMessage(peerInteraction->createKeepAliveMessage());
	peerInteraction->sendMessages(e->getUploadSpeed());
      }
      keepAliveCheckPoint = now;
    }
  }
}

void PeerInteractionCommand::beforeSocketCheck() {
  if(sequence == WIRED) {
    e->torrentMan->unadvertisePiece(cuid);
    detectMessageFlooding();
    //checkLongTimePeerChoking();
    PieceIndexes indexes = e->torrentMan->getAdvertisedPieceIndexes(cuid);
    if(indexes.size() >= 20) {
      if(peer->isFastExtensionEnabled()) {
	if(e->torrentMan->hasAllPieces()) {
	  peerInteraction->addMessage(peerInteraction->createHaveAllMessage());
	} else {
	  peerInteraction->addMessage(peerInteraction->createBitfieldMessage());
	}
      } else {
	peerInteraction->addMessage(peerInteraction->createBitfieldMessage());
      }
    } else {
      for(PieceIndexes::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
	peerInteraction->addMessage(peerInteraction->createHaveMessage(*itr));
      }
    }
    keepAlive();
  }
}
