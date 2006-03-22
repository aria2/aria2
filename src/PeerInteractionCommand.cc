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

PeerInteractionCommand::PeerInteractionCommand(int cuid, Peer* peer,
					       TorrentDownloadEngine* e,
					       Socket* s, int sequence)
  :PeerAbstractCommand(cuid, peer, e, s), sequence(sequence) {
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    setReadCheckSocket(NULL);
    setWriteCheckSocket(socket);
  }
  peerConnection = new PeerConnection(cuid, socket, e->option, e->logger,
				      peer, e->torrentMan);
  requestSlotMan = new RequestSlotMan(cuid, &pendingMessages, peerConnection,
				      e->torrentMan, e->logger);
  piece = Piece::nullPiece;
  keepAliveCheckPoint.tv_sec = 0;
  keepAliveCheckPoint.tv_usec = 0;
}

PeerInteractionCommand::~PeerInteractionCommand() {
  delete peerConnection;
  delete requestSlotMan;
  e->torrentMan->unadvertisePiece(cuid);
}

bool PeerInteractionCommand::executeInternal() {
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    socket->setBlockingMode();
    setReadCheckSocket(socket);
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
    if(e->torrentMan->getDownloadedSize() > 0) {
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
    if(e->torrentMan->getDownloadedSize() > 0) {
      peerConnection->sendBitfield();
    }
    sequence = WIRED;    
    break;
  }
  case WIRED:
    syncPiece();
    decideChoking();
    for(int i = 0; i < 10; i++) {
      if(!socket->isReadable(0)) {
	break;
      }
      receiveMessage();
    }
    requestSlotMan->deleteTimedoutRequestSlot(piece);
    requestSlotMan->deleteCompletedRequestSlot(piece);
    sendInterest();
    sendMessages();
    break;
  }
  if(pendingMessages.size() > 0) {
    setWriteCheckSocket(socket);
  }
  e->commands.push(this);
  return false;
}

void PeerInteractionCommand::syncPiece() {
  if(Piece::isNull(piece)) {
    return;
  }
  e->torrentMan->syncPiece(piece);
}

void PeerInteractionCommand::decideChoking() {
  if(peer->shouldChoke()) {
    if(!peer->amChocking) {
      PendingMessage pendingMessage(PeerMessage::CHOKE, peerConnection);
      pendingMessages.push_back(pendingMessage);
    }
  } else if(peer->amChocking && peer->peerInterested) {
    PendingMessage pendingMessage(PeerMessage::UNCHOKE, peerConnection);
    pendingMessages.push_back(pendingMessage);
  } else if(!peer->peerInterested) {
    PendingMessage pendingMessage(PeerMessage::CHOKE, peerConnection);
    pendingMessages.push_back(pendingMessage);
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
      peer->peerChoking = true;
      requestSlotMan->deleteAllRequestSlot(piece);
      break;
    case PeerMessage::UNCHOKE:
      peer->peerChoking = false;
      break;
    case PeerMessage::INTERESTED:
      peer->peerInterested = true;
      break;
    case PeerMessage::NOT_INTERESTED:
      peer->peerInterested = false;
      break;
    case PeerMessage::HAVE:
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
	pendingMessages.push_back(pendingMessage);
	e->torrentMan->addUploadedSize(message->getLength());
	e->torrentMan->addDeltaUpload(message->getLength());
      }
      break;
    case PeerMessage::CANCEL:
      deletePendingMessage(message);
      break;
    case PeerMessage::PIECE: {
      RequestSlot slot = requestSlotMan->getCorrespoindingRequestSlot(message);
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
	requestSlotMan->deleteRequestSlot(slot);
	e->torrentMan->updatePiece(piece);
	e->logger->debug("CUID#%d - setting piece bit index=%d", cuid,
			 slot.getBlockIndex());
	e->torrentMan->addDeltaDownload(message->getBlockLength());
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

void PeerInteractionCommand::deletePendingMessage(PeerMessage* cancelMessage) {
  for(PendingMessages::iterator itr = pendingMessages.begin();
      itr != pendingMessages.end();) {
    PendingMessage& pendingMessage = *itr;
    if(pendingMessage.getPeerMessageId() == PeerMessage::PIECE &&
       pendingMessage.getIndex() == cancelMessage->getIndex() &&
       pendingMessage.getBegin() == cancelMessage->getBegin() &&
       pendingMessage.getLength() == cancelMessage->getLength() &&
       !pendingMessage.isInProgress()) {
      e->logger->debug("CUID#%d - deleting pending piece message because cancel message received. index=%d, begin=%d, length=%d",
		       cuid,
		       pendingMessage.getIndex(),
		       pendingMessage.getBegin(),
		       pendingMessage.getLength());
      itr = pendingMessages.erase(itr);
    } else {
      itr++;
    }
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
  Piece piece = e->torrentMan->getMissingPiece(peer);
  if(Piece::isNull(piece)) {
    e->logger->debug("CUID#%d - try to send not-interested", cuid);
    PendingMessage pendingMessage(PeerMessage::NOT_INTERESTED, peerConnection);
    pendingMessages.push_back(pendingMessage);
  } else {
    e->logger->debug("CUID#%d - try to send interested", cuid);
    PendingMessage pendingMessage(PeerMessage::INTERESTED, peerConnection);
    pendingMessages.push_back(pendingMessage);
  }
  return piece;
}

void PeerInteractionCommand::sendInterest() {
  if(Piece::isNull(piece)) {
    // retrive new piece from TorrentMan
    piece = getNewPieceAndSendInterest();
  } else if(peer->peerChoking) {
    // TODO separate method is better
    requestSlotMan->deleteAllRequestSlot(piece);
    e->torrentMan->cancelPiece(piece);
    piece = Piece::nullPiece;
  } else if(piece.pieceComplete()) {
    piece = getNewPieceAndSendInterest();
  }
}

void PeerInteractionCommand::createRequestPendingMessage(int blockIndex) {
  PendingMessage pendingMessage =
    PendingMessage::createRequestMessage(piece.getIndex(),
					 blockIndex*piece.getBlockLength(),
					 piece.getBlockLength(blockIndex),
					 peerConnection);
  pendingMessages.push_back(pendingMessage);
  RequestSlot requestSlot(piece.getIndex(),
			  blockIndex*piece.getBlockLength(),
			  piece.getBlockLength(blockIndex),
			  blockIndex);
  requestSlotMan->addRequestSlot(requestSlot);
}

void PeerInteractionCommand::sendMessages() {
  if(!Piece::isNull(piece) && !peer->peerChoking) {
    if(e->torrentMan->isEndGame()) {
      BlockIndexes missingBlockIndexes = piece.getAllMissingBlockIndexes();
      if(requestSlotMan->isEmpty()) {
	for(PieceIndexes::const_iterator itr = missingBlockIndexes.begin();
	    itr != missingBlockIndexes.end(); itr++) {
	  createRequestPendingMessage(*itr);
	}
      }
    } else {
      for(int i = requestSlotMan->countRequestSlot(); i <= 5; i++) {
	int blockIndex = piece.getMissingUnusedBlockIndex();
	if(blockIndex == -1) {
	  if(requestSlotMan->isEmpty()) {
	    piece = Piece::nullPiece;
	  }
	  break;
	}
	e->torrentMan->updatePiece(piece);
	createRequestPendingMessage(blockIndex);
      }
    }
  }

  for(PendingMessages::iterator itr = pendingMessages.begin(); itr != pendingMessages.end();) {
    if(itr->processMessage()) {
      itr = pendingMessages.erase(itr);
    } else {
      //setWriteCheckSocket(socket);
      break;
    }
  }
}

void PeerInteractionCommand::onAbort(Exception* ex) {
  requestSlotMan->deleteAllRequestSlot(piece);
  e->torrentMan->cancelPiece(piece);
  PeerAbstractCommand::onAbort(ex);
}

void PeerInteractionCommand::keepAlive() {
  if(keepAliveCheckPoint.tv_sec == 0 && keepAliveCheckPoint.tv_usec == 0) {
    gettimeofday(&keepAliveCheckPoint, NULL);
  } else {
    struct timeval now;
    gettimeofday(&now, NULL);
    if(Util::difftv(now, keepAliveCheckPoint) >= 120*1000000) {
      if(pendingMessages.empty()) {
	peerConnection->sendKeepAlive();
      }
      keepAliveCheckPoint = now;
    }
  }
}

void PeerInteractionCommand::beforeSocketCheck() {
  if(sequence == WIRED) {
    e->torrentMan->unadvertisePiece(cuid);

    PieceIndexes indexes = e->torrentMan->getAdvertisedPieceIndexes(cuid);
    if(indexes.size() >= 20) {
      PendingMessage pendingMessage(PeerMessage::BITFIELD, peerConnection);
      pendingMessages.push_back(pendingMessage);
    } else {
      if(pendingMessages.size() == 0) {
	for(PieceIndexes::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
	  peerConnection->sendHave(*itr);
	}
      } else {
	for(PieceIndexes::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
	  PendingMessage pendingMessage = PendingMessage::createHaveMessage(*itr, peerConnection);
	  pendingMessages.push_back(pendingMessage);
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
