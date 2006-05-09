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
  sendMessageQueue = new SendMessageQueue(cuid, socket, e->option,
					  e->torrentMan, this->peer);
  sendMessageQueue->setUploadLimit(e->option->getAsInt(PREF_UPLOAD_LIMIT));
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
  delete sendMessageQueue;
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

  switch(sequence) {
  case INITIATOR_SEND_HANDSHAKE:
    sendMessageQueue->sendHandshake();
    sequence = INITIATOR_WAIT_HANDSHAKE;
    break;
  case INITIATOR_WAIT_HANDSHAKE: {
    HandshakeMessage* handshakeMessage = sendMessageQueue->receiveHandshake();
    if(handshakeMessage == NULL) {
      break;
    }
    peer->setPeerId(handshakeMessage->peerId);
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 handshakeMessage->toString().c_str());
    delete handshakeMessage;
    if(e->torrentMan->getDownloadLength() > 0) {
      sendMessageQueue->sendNow(sendMessageQueue->createBitfieldMessage());
    }
    sequence = WIRED;
    break;
  }
  case RECEIVER_WAIT_HANDSHAKE: {
    HandshakeMessage* handshakeMessage = sendMessageQueue->receiveHandshake();
    if(handshakeMessage == NULL) {
      break;
    }
    peer->setPeerId(handshakeMessage->peerId);
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 handshakeMessage->toString().c_str());
    delete handshakeMessage;
    sendMessageQueue->sendHandshake();
    if(e->torrentMan->getDownloadLength() > 0) {
      sendMessageQueue->sendNow(sendMessageQueue->createBitfieldMessage());
    }
    sequence = WIRED;    
    break;
  }
  case WIRED:
    detectMessageFlooding();
    checkLongTimePeerChoking();
    checkInactiveConnection();
    sendMessageQueue->syncPiece();
    decideChoking();
    for(int i = 0; i < 10; i++) {
      if(!socket->isReadable(0)) {
	break;
      }
      receiveMessage();
    }
    sendMessageQueue->deleteTimeoutRequestSlot();
    sendMessageQueue->deleteCompletedRequestSlot();
    sendMessageQueue->sendMessages(e->getUploadSpeed());
    break;
  }
  if(sendMessageQueue->countMessageInQueue() > 0) {
    setWriteCheckSocket(socket);
  }
  e->commands.push_back(this);
  return false;
}

void PeerInteractionCommand::checkInactiveConnection() {
  if((!peer->amInterested && !peer->peerInterested &&
     e->torrentMan->connections >= MAX_PEER_LIST_SIZE) ||
     (!peer->amInterested && e->torrentMan->connections >= MAX_PEER_LIST_SIZE &&
      e->torrentMan->isEndGame())) {
    throw new DlAbortEx("marked as inactive connection.");
  }
  
}

void PeerInteractionCommand::detectMessageFlooding() {
  struct timeval now;
  gettimeofday(&now, NULL);
  if(freqCheckPoint.tv_sec == 0 && freqCheckPoint.tv_usec == 0) {
    freqCheckPoint = now;
  } else {
    if(Util::difftv(now, freqCheckPoint) >= 5*1000000) {
      if(chokeUnchokeCount*1.0/(Util::difftv(now, freqCheckPoint)/1000000) >= 0.4
	 || haveCount*1.0/(Util::difftv(now, freqCheckPoint)/1000000) >= 20.0
	 || keepAliveCount*1.0/(Util::difftv(now, freqCheckPoint)/1000000) >= 1.0) {
	throw new DlAbortEx("flooding detected.");
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
      if(Util::difftv(now, chokeCheckPoint) >= MAX_PEER_CHOKING_INTERVAL*1000000) {
	throw new DlAbortEx("too long choking");
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
      sendMessageQueue->addMessage(sendMessageQueue->createChokeMessage());
    }
  } else {
    if(peer->amChoking) {
      sendMessageQueue->addMessage(sendMessageQueue->createUnchokeMessage());
    }
  }
}

void PeerInteractionCommand::receiveMessage() {
  PeerMessage* message = sendMessageQueue->receiveMessage();
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
  sendMessageQueue->abortPiece();
  PeerAbstractCommand::onAbort(ex);
}

void PeerInteractionCommand::keepAlive() {
  if(keepAliveCheckPoint.tv_sec == 0 && keepAliveCheckPoint.tv_usec == 0) {
    gettimeofday(&keepAliveCheckPoint, NULL);
  } else {
    struct timeval now;
    gettimeofday(&now, NULL);
    if(Util::difftv(now, keepAliveCheckPoint) >= (long long int)120*1000000) {
      if(sendMessageQueue->countMessageInQueue() == 0) {
	sendMessageQueue->sendNow(sendMessageQueue->createKeepAliveMessage());
      }
      keepAliveCheckPoint = now;
    }
  }
}

void PeerInteractionCommand::beforeSocketCheck() {
  if(sequence == WIRED) {
    e->torrentMan->unadvertisePiece(cuid);
    detectMessageFlooding();
    checkLongTimePeerChoking();
    checkInactiveConnection();

    PieceIndexes indexes = e->torrentMan->getAdvertisedPieceIndexes(cuid);
    if(indexes.size() >= 20) {
      sendMessageQueue->trySendNow(sendMessageQueue->createBitfieldMessage());
    } else {
      for(PieceIndexes::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
	sendMessageQueue->trySendNow(sendMessageQueue->createHaveMessage(*itr));
      }
    }
    keepAlive();
  }
}
