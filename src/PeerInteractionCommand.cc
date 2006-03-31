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
  peerConnection = new PeerConnection(cuid, socket, e->option, e->logger,
				      peer, e->torrentMan);
  sendMessageQueue = new SendMessageQueue(cuid, peerConnection, e->torrentMan,
					  e->logger);
  piece = Piece::nullPiece;
  keepAliveCheckPoint.tv_sec = 0;
  keepAliveCheckPoint.tv_usec = 0;
  chokeCheckPoint.tv_sec = 0;
  chokeCheckPoint.tv_usec = 0;
  freqCheckPoint.tv_sec = 0;
  freqCheckPoint.tv_usec = 0;
  chokeUnchokeCount = 0;
  haveCount = 0;
}

PeerInteractionCommand::~PeerInteractionCommand() {
  delete peerConnection;
  delete sendMessageQueue;
  e->torrentMan->unadvertisePiece(cuid);
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
    peerConnection->sendHandshake();
    sequence = INITIATOR_WAIT_HANDSHAKE;
    break;
  case INITIATOR_WAIT_HANDSHAKE: {
    HandshakeMessage* handshakeMessage = peerConnection->receiveHandshake();
    if(handshakeMessage == NULL) {
      break;
    }
    peer->setPeerId(handshakeMessage->peerId);
    e->logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		    peer->ipaddr.c_str(), peer->port,
		    handshakeMessage->toString().c_str());
    delete handshakeMessage;
    if(e->torrentMan->getDownloadLength() > 0) {
      peerConnection->sendBitfield();
    }
    sequence = WIRED;
    break;
  }
  case RECEIVER_WAIT_HANDSHAKE: {
    HandshakeMessage* handshakeMessage = peerConnection->receiveHandshake();
    if(handshakeMessage == NULL) {
      break;
    }
    peer->setPeerId(handshakeMessage->peerId);
    e->logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		    peer->ipaddr.c_str(), peer->port,
		    handshakeMessage->toString().c_str());
    delete handshakeMessage;
    peerConnection->sendHandshake();
    if(e->torrentMan->getDownloadLength() > 0) {
      peerConnection->sendBitfield();
    }
    sequence = WIRED;    
    break;
  }
  case WIRED:
    detectMessageFlooding();
    checkLongTimePeerChoking();
    checkInactiveConnection();
    syncPiece();
    decideChoking();
    for(int i = 0; i < 10; i++) {
      if(!socket->isReadable(0)) {
	break;
      }
      receiveMessage();
    }
    sendMessageQueue->deleteTimeoutRequestSlot(piece);
    sendMessageQueue->deleteCompletedRequestSlot(piece);
    sendInterest();
    sendMessages();
    break;
  }
  if(sendMessageQueue->countPendingMessage() > 0) {
    setWriteCheckSocket(socket);
  }
  e->commands.push(this);
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
	 || haveCount*1.0/(Util::difftv(now, freqCheckPoint)/1000000) >= 20.0) {
	throw new DlAbortEx("flooding detected.");
      } else {
	chokeUnchokeCount = 0;
	haveCount = 0;
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
      if(Util::difftv(now, chokeCheckPoint) >= 3*60*1000000) {
	throw new DlAbortEx("too long choking");
      }
    } else {
      chokeCheckPoint.tv_sec = 0;
      chokeCheckPoint.tv_usec = 0;
    }
  }
}

void PeerInteractionCommand::syncPiece() {
  if(Piece::isNull(piece)) {
    return;
  }
  e->torrentMan->syncPiece(piece);
}

void PeerInteractionCommand::decideChoking() {
  if(e->torrentMan->downloadComplete()) {
    if(peer->amChocking && peer->peerInterested) {
      PendingMessage pendingMessage(PeerMessage::UNCHOKE, peerConnection);
      sendMessageQueue->addPendingMessage(pendingMessage);
    }
    return;
  }
  if(peer->shouldChoke()) {
    if(!peer->amChocking) {
      PendingMessage pendingMessage(PeerMessage::CHOKE, peerConnection);
      sendMessageQueue->addPendingMessage(pendingMessage);
    }
  } else if(peer->amChocking && peer->peerInterested) {
    PendingMessage pendingMessage(PeerMessage::UNCHOKE, peerConnection);
    sendMessageQueue->addPendingMessage(pendingMessage);
  } else if(!peer->peerInterested) {
    PendingMessage pendingMessage(PeerMessage::CHOKE, peerConnection);
    sendMessageQueue->addPendingMessage(pendingMessage);
  }
}

void PeerInteractionCommand::receiveMessage() {
  PeerMessage* message = peerConnection->receiveMessage();
  if(message == NULL) {
    return;
  }
  e->logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		  peer->ipaddr.c_str(), peer->port,
		  message->toString().c_str());
  try {
    switch(message->getId()) {
    case PeerMessage::KEEP_ALIVE:
      break;
    case PeerMessage::CHOKE:
      if(!peer->peerChoking) {
	chokeUnchokeCount++;
      }
      peer->peerChoking = true;
      break;
    case PeerMessage::UNCHOKE:
      if(peer->peerChoking) {
	chokeUnchokeCount++;
      }
      peer->peerChoking = false;
      break;
    case PeerMessage::INTERESTED:
      peer->peerInterested = true;
      break;
    case PeerMessage::NOT_INTERESTED:
      peer->peerInterested = false;
      break;
    case PeerMessage::HAVE:
      haveCount++;
      peer->updateBitfield(message->getIndex(), 1);
      break;
    case PeerMessage::BITFIELD:
      peer->setBitfield(message->getBitfield(), message->getBitfieldLength());
      break;
    case PeerMessage::REQUEST:
      if(e->torrentMan->hasPiece(message->getIndex())) {
	PendingMessage pendingMessage
	  = PendingMessage::createPieceMessage(message->getIndex(),
					       message->getBegin(),
					       message->getLength(),
					       e->torrentMan->pieceLength,
					       peerConnection);
	sendMessageQueue->addPendingMessage(pendingMessage);
	e->torrentMan->addUploadLength(message->getLength());
	e->torrentMan->addDeltaUpload(message->getLength());
      }
      break;
    case PeerMessage::CANCEL:
      sendMessageQueue->deletePendingPieceMessage(message);
      break;
    case PeerMessage::PIECE: {
      RequestSlot slot = sendMessageQueue->getCorrespoindingRequestSlot(message);
      peer->addPeerUpload(message->getBlockLength());
      if(!Piece::isNull(piece) && !RequestSlot::isNull(slot)) {
	long long int offset =
	  ((long long int)message->getIndex())*e->torrentMan->pieceLength+message->getBegin();
	e->logger->debug("CUID#%d - write block length = %d, offset=%lld",
			 cuid, message->getBlockLength(), offset);      
	e->torrentMan->diskWriter->writeData(message->getBlock(),
					     message->getBlockLength(),
					     offset);
	piece.completeBlock(slot.getBlockIndex());
	sendMessageQueue->deleteRequestSlot(slot);
	e->torrentMan->updatePiece(piece);
	e->logger->debug("CUID#%d - setting piece bit index=%d", cuid,
			 slot.getBlockIndex());
	e->torrentMan->addDeltaDownloadLength(message->getBlockLength());
	if(piece.pieceComplete()) {
	  if(checkPieceHash(piece)) {
	    onGotNewPiece();
	  } else {
	    onGotWrongPiece();
	  }
	}
      }
      break;
    }
    }
    delete message;
  } catch(Exception* ex) {
    delete message;
    throw;
  }
}

void PeerInteractionCommand::onGotNewPiece() {
  e->logger->info(MSG_GOT_NEW_PIECE, cuid, piece.getIndex());
  e->torrentMan->completePiece(piece);
  e->torrentMan->advertisePiece(cuid, piece.getIndex());
  piece = Piece::nullPiece;
}

void PeerInteractionCommand::onGotWrongPiece() {
  e->logger->error(MSG_GOT_WRONG_PIECE, cuid, piece.getIndex());
  erasePieceOnDisk(piece);
  piece.clearAllBlock();
  e->torrentMan->updatePiece(piece);
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(int wait) {
  if(e->torrentMan->isPeerAvailable()) {
    Peer* peer = e->torrentMan->getPeer();
    int newCuid = e->torrentMan->getNewCuid();
    peer->cuid = newCuid;
    PeerInitiateConnectionCommand* command = new PeerInitiateConnectionCommand(newCuid, peer, e);
    e->commands.push(command);
  }
  return true;
}

bool PeerInteractionCommand::prepareForRetry(int wait) {
  e->commands.push(this);
  return false;
}

Piece PeerInteractionCommand::getNewPieceAndSendInterest() {
  sendMessageQueue->cancelAllRequest();
  Piece piece = e->torrentMan->getMissingPiece(peer);
  if(Piece::isNull(piece)) {
    e->logger->debug("CUID#%d - try to send not-interested", cuid);
    PendingMessage pendingMessage(PeerMessage::NOT_INTERESTED, peerConnection);
    sendMessageQueue->addPendingMessage(pendingMessage);
  } else {
    e->logger->debug("CUID#%d - starting download for piece #%d", cuid, piece.getIndex());
    e->logger->debug("CUID#%d - try to send interested", cuid);
    PendingMessage pendingMessage(PeerMessage::INTERESTED, peerConnection);
    sendMessageQueue->addPendingMessage(pendingMessage);
  }
  return piece;
}

void PeerInteractionCommand::sendInterest() {
  if(Piece::isNull(piece)) {
    // retrive new piece from TorrentMan
    piece = getNewPieceAndSendInterest();
  } else if(peer->peerChoking) {
    sendMessageQueue->cancelAllRequest(piece);
    e->torrentMan->cancelPiece(piece);
    piece = Piece::nullPiece;
  } else if(piece.pieceComplete()) {
    piece = getNewPieceAndSendInterest();
  }
}

void PeerInteractionCommand::createRequestPendingMessage(int blockIndex) {
  PendingMessage pendingMessage =
    PendingMessage::createRequestMessage(piece, blockIndex, peerConnection);
  sendMessageQueue->addPendingMessage(pendingMessage);
}

void PeerInteractionCommand::sendMessages() {
  if(!Piece::isNull(piece) && !peer->peerChoking) {
    if(e->torrentMan->isEndGame()) {
      BlockIndexes missingBlockIndexes = piece.getAllMissingBlockIndexes();
      if(sendMessageQueue->countRequestSlot() == 0) {
	random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end());
	int count = 0;
	for(PieceIndexes::const_iterator itr = missingBlockIndexes.begin();
	    itr != missingBlockIndexes.end() && count < 6; itr++, count++) {
	  createRequestPendingMessage(*itr);
	}
      }
    } else {
      for(int i = sendMessageQueue->countRequestSlot(); i < 6; i++) {
	int blockIndex = piece.getMissingUnusedBlockIndex();
	if(blockIndex == -1) {
	  break;
	}
	e->torrentMan->updatePiece(piece);
	createRequestPendingMessage(blockIndex);
      }
    }
  }

  sendMessageQueue->send();
}

void PeerInteractionCommand::onAbort(Exception* ex) {
  sendMessageQueue->cancelAllRequest(piece);
  e->torrentMan->cancelPiece(piece);
  PeerAbstractCommand::onAbort(ex);
}

void PeerInteractionCommand::keepAlive() {
  if(keepAliveCheckPoint.tv_sec == 0 && keepAliveCheckPoint.tv_usec == 0) {
    gettimeofday(&keepAliveCheckPoint, NULL);
  } else {
    struct timeval now;
    gettimeofday(&now, NULL);
    if(Util::difftv(now, keepAliveCheckPoint) >= (long long int)120*1000000) {
      if(sendMessageQueue->countPendingMessage() == 0) {
	peerConnection->sendKeepAlive();
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
      PendingMessage pendingMessage(PeerMessage::BITFIELD, peerConnection);
      sendMessageQueue->addPendingMessage(pendingMessage);
    } else {
      if(sendMessageQueue->countPendingMessage() == 0) {
	for(PieceIndexes::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
	  peerConnection->sendHave(*itr);
	}
      } else {
	for(PieceIndexes::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
	  PendingMessage pendingMessage = PendingMessage::createHaveMessage(*itr, peerConnection);
	  sendMessageQueue->addPendingMessage(pendingMessage);
	}
      }
    }
    keepAlive();
  }
}

bool PeerInteractionCommand::checkPieceHash(const Piece& piece) {
  long long int offset = ((long long int)piece.getIndex())*e->torrentMan->pieceLength;
  return e->torrentMan->diskWriter->sha1Sum(offset, piece.getLength()) ==
    e->torrentMan->getPieceHash(piece.getIndex());
}

void PeerInteractionCommand::erasePieceOnDisk(const Piece& piece) {
  int BUFSIZE = 4096;
  char buf[BUFSIZE];
  memset(buf, 0, BUFSIZE);
  long long int offset = ((long long int)piece.getIndex())*e->torrentMan->pieceLength;
  for(int i = 0; i < piece.getLength()/BUFSIZE; i++) {
    e->torrentMan->diskWriter->writeData(buf, BUFSIZE, offset);
    offset += BUFSIZE;
  }
  int r = piece.getLength()%BUFSIZE;
  if(r > 0) {
    e->torrentMan->diskWriter->writeData(buf, r, offset);
  }
}
