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
#include "SendMessageQueue.h"
#include "LogFactory.h"
#include "DlAbortEx.h"
#include "KeepAliveMessage.h"
#include "PeerMessageUtil.h"
#include <netinet/in.h>

SendMessageQueue::SendMessageQueue(int cuid,
				   const Socket* socket,
				   const Option* op,
				   TorrentMan* torrentMan,
				   Peer* peer)
  :cuid(cuid),
   uploadLimit(0),
   torrentMan(torrentMan),
   peer(peer),
   piece(Piece::nullPiece) {
  peerConnection = new PeerConnection(cuid, socket, op, peer, this->torrentMan);
  logger = LogFactory::getInstance();
}

SendMessageQueue::~SendMessageQueue() {
  delete peerConnection;
  for_each(messageQueue.begin(), messageQueue.end(), Deleter());
}

void SendMessageQueue::send(int uploadSpeed) {
  int size = messageQueue.size();
  for(int i = 0; i < size; i++) {
    PeerMessage* msg = messageQueue.front();
    messageQueue.pop_front();
    if(uploadLimit != 0 && uploadLimit*1024 <= uploadSpeed &&
       msg->getId() == PieceMessage::ID && !msg->isInProgress()) {
      messageQueue.push_back(msg);
    } else {
      try {
	msg->send();
      } catch(Exception* ex) {
	delete msg;
	throw;
      }
      if(msg->isInProgress()) {
	messageQueue.push_front(msg);
	break;
      } else {
	delete msg;
      }
    }
  }
}

void SendMessageQueue::addMessage(PeerMessage* peerMessage) {
  messageQueue.push_back(peerMessage);
  if(peerMessage->getId() == RequestMessage::ID) {
    RequestMessage* requestMessage = (RequestMessage*)peerMessage;
    RequestSlot requestSlot(requestMessage->getIndex(),
			    requestMessage->getBegin(),
			    requestMessage->getLength(),
			    requestMessage->getBlockIndex());
    requestSlots.push_back(requestSlot);
  }
}

void SendMessageQueue::deletePieceMessageInQueue(const CancelMessage* cancelMessage) {
  for(MessageQueue::iterator itr = messageQueue.begin();
      itr != messageQueue.end();) {
    if((*itr)->getId() == PieceMessage::ID) {
      PieceMessage* pieceMessage = (PieceMessage*)*itr;
      if(pieceMessage->getIndex() == cancelMessage->getIndex() &&
	 pieceMessage->getBegin() == cancelMessage->getBegin() &&
	 pieceMessage->getBlockLength() == cancelMessage->getLength() &&
	 !pieceMessage->isInProgress()) {
	logger->debug("CUID#%d - deleting piece message in queue because cancel message received. index=%d, begin=%d, length=%d",
		      cuid,
		      cancelMessage->getIndex(),
		      cancelMessage->getBegin(),
		      cancelMessage->getLength());
	delete pieceMessage;
	itr = messageQueue.erase(itr);
      } else {
	itr++;
      }
    } else {
      itr++;
    }
  }
}

void SendMessageQueue::deleteRequestMessageInQueue() {
  for(MessageQueue::iterator itr = messageQueue.begin();
      itr != messageQueue.end();) {
    if((*itr)->getId() == RequestMessage::ID) {
      delete *itr;
      itr = messageQueue.erase(itr);
    } else {
      itr++;
    }
  }      
}

void SendMessageQueue::deleteRequestSlot(const RequestSlot& requestSlot) {
  // TODO use STL algorithm
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    if(*itr == requestSlot) {
      requestSlots.erase(itr);
      break;
    }
  }
}

void SendMessageQueue::deleteAllRequestSlot(Piece& piece) {
  if(!Piece::isNull(piece)) {
    for(RequestSlots::const_iterator itr = requestSlots.begin();
	itr != requestSlots.end(); itr++) {
      if(itr->getIndex() == piece.getIndex()) {
	piece.cancelBlock(itr->getBlockIndex());
      }
    }
    torrentMan->updatePiece(piece);
  }
  requestSlots.clear();
}

void SendMessageQueue::deleteTimeoutRequestSlot() {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    if(itr->isTimeout(REQUEST_TIME_OUT)) {
      logger->debug("CUID#%d - deleting requestslot blockIndex=%d because of time out", cuid,
		    itr->getBlockIndex());
      if(!Piece::isNull(piece)) {
	//addMessage(createCancelMessage(itr->getIndex(), itr->getBegin(), itr->getLength()));
	piece.cancelBlock(itr->getBlockIndex());
      }
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }
  torrentMan->updatePiece(piece);
}

void SendMessageQueue::deleteCompletedRequestSlot() {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    if(Piece::isNull(piece) || piece.hasBlock(itr->getBlockIndex()) ||
       torrentMan->hasPiece(piece.getIndex())) {
      logger->debug("CUID#%d - deleting requestslot blockIndex=%d because the block is already acquired.", cuid,
		    itr->getBlockIndex());
      addMessage(createCancelMessage(itr->getIndex(), itr->getBegin(), itr->getLength()));
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }
}

RequestSlot SendMessageQueue::getCorrespondingRequestSlot(const PieceMessage* pieceMessage) const {
  for(RequestSlots::const_iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    const RequestSlot& slot = *itr;
    if(slot.getIndex() == pieceMessage->getIndex() &&
       slot.getBegin() == pieceMessage->getBegin() &&
       slot.getLength() == pieceMessage->getBlockLength()) {
      return slot;
    }
  }
  return RequestSlot::nullSlot;
}

void SendMessageQueue::cancelAllRequest() {
  cancelAllRequest(Piece::nullPiece);
}

void SendMessageQueue::cancelAllRequest(Piece& piece) {
  deleteRequestMessageInQueue();
  deleteAllRequestSlot(piece);
}

int SendMessageQueue::countMessageInQueue() const {
  return messageQueue.size();
}

int SendMessageQueue::countRequestSlot() const {
  return requestSlots.size();
}

HandshakeMessage* SendMessageQueue::receiveHandshake() {
  char msg[HANDSHAKE_MESSAGE_LENGTH];
  int msgLength = 0;
  if(!peerConnection->receiveHandshake(msg, msgLength)) {
    return NULL;
  }
  HandshakeMessage* handshakeMessage = createHandshakeMessage(msg, msgLength);
  try {
    handshakeMessage->check();
  } catch(Exception* e) {
    delete handshakeMessage;
    throw;
  }
  return handshakeMessage;
}

HandshakeMessage* SendMessageQueue::createHandshakeMessage(const char* msg, int msgLength) {
  HandshakeMessage* message = PeerMessageUtil::createHandshakeMessage(msg, msgLength);
  message->setSendMessageQueue(this);
  return message;
}

PeerMessage* SendMessageQueue::receiveMessage() {
  char msg[MAX_PAYLOAD_LEN];
  int msgLength = 0;
  if(!peerConnection->receiveMessage(msg, msgLength)) {
    return NULL;
  }
  PeerMessage* peerMessage = createPeerMessage(msg, msgLength);
  try {
    peerMessage->check();
  } catch(Exception* e) {
    delete peerMessage;
    throw;
  }
  return peerMessage;
}

PeerMessage* SendMessageQueue::createPeerMessage(const char* msg, int msgLength) {
  PeerMessage* peerMessage;
  if(msgLength == 0) {
    // keep-alive
    peerMessage = new KeepAliveMessage();
  } else {
    int id = PeerMessageUtil::getId(msg);
    switch(id) {
    case ChokeMessage::ID:
      peerMessage = PeerMessageUtil::createChokeMessage(msg, msgLength);
      break;
    case UnchokeMessage::ID:
      peerMessage = PeerMessageUtil::createUnchokeMessage(msg, msgLength);
      break;
    case InterestedMessage::ID:
      peerMessage = PeerMessageUtil::createInterestedMessage(msg, msgLength);
      break;
    case NotInterestedMessage::ID:
      peerMessage = PeerMessageUtil::createNotInterestedMessage(msg, msgLength);
      break;
    case HaveMessage::ID:
      peerMessage = PeerMessageUtil::createHaveMessage(msg, msgLength);
      ((HaveMessage*)peerMessage)->setPieces(torrentMan->pieces);
      break;
    case BitfieldMessage::ID:
      peerMessage = PeerMessageUtil::createBitfieldMessage(msg, msgLength);
      ((BitfieldMessage*)peerMessage)->setPieces(torrentMan->pieces);
      break;
    case RequestMessage::ID:
      peerMessage = PeerMessageUtil::createRequestMessage(msg, msgLength);
      ((RequestMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((RequestMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((RequestMessage*)peerMessage)->getIndex()));
      break;
    case CancelMessage::ID:
      peerMessage = PeerMessageUtil::createCancelMessage(msg, msgLength);
      ((CancelMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((CancelMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((CancelMessage*)peerMessage)->getIndex()));
      break;
    case PieceMessage::ID:
      peerMessage = PeerMessageUtil::createPieceMessage(msg, msgLength);
      ((PieceMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((PieceMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((PieceMessage*)peerMessage)->getIndex()));
      break;
    case PortMessage::ID:
      peerMessage = PeerMessageUtil::createPortMessage(msg, msgLength);
      break;
    default:
      throw new DlAbortEx("invalid message id. id = %d", id);
    }
  }
  setPeerMessageCommonProperty(peerMessage);
  return peerMessage;
}


void SendMessageQueue::syncPiece() {
  if(Piece::isNull(piece)) {
    return;
  }
  torrentMan->syncPiece(piece);
}

Piece SendMessageQueue::getNewPieceAndSendInterest() {
  cancelAllRequest();
  Piece piece = torrentMan->getMissingPiece(peer);
  if(Piece::isNull(piece)) {
    logger->debug("CUID#%d - not interested in the peer", cuid);
    addMessage(createNotInterestedMessage());
  } else {
    logger->debug("CUID#%d - starting download for piece index=%d (%d/%d completed)",
		  cuid, piece.getIndex(), piece.countCompleteBlock(),
		  piece.countBlock());
    logger->debug("CUID#%d - interested in the peer", cuid);
    addMessage(createInterestedMessage());
  }
  return piece;
}

void SendMessageQueue::sendMessages(int currentUploadSpeed) {
  if(Piece::isNull(piece)) {
    // retrive new piece from TorrentMan
    piece = getNewPieceAndSendInterest();
  } else if(peer->peerChoking) {
    cancelAllRequest(piece);
    torrentMan->cancelPiece(piece);
    piece = Piece::nullPiece;
  } else if(piece.pieceComplete()) {
    piece = getNewPieceAndSendInterest();
  }
  if(!Piece::isNull(piece) && !peer->peerChoking) {
    if(torrentMan->isEndGame()) {
      BlockIndexes missingBlockIndexes = piece.getAllMissingBlockIndexes();
      if(countRequestSlot() == 0) {
	random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end());
	int count = 0;
	for(BlockIndexes::const_iterator itr = missingBlockIndexes.begin();
	    itr != missingBlockIndexes.end() && count < 6; itr++, count++) {
	  addMessage(createRequestMessage(*itr));
	}
      }
    } else {
      for(int i = countRequestSlot(); i < 6; i++) {
	int blockIndex = piece.getMissingUnusedBlockIndex();
	if(blockIndex == -1) {
	  break;
	}
	torrentMan->updatePiece(piece);
	addMessage(createRequestMessage(blockIndex));
      }
    }
  }
  send(currentUploadSpeed);
}

void SendMessageQueue::sendNow(PeerMessage* peerMessage) {
  // ignore inProgress state
  peerMessage->send();
  delete peerMessage;
}

void SendMessageQueue::trySendNow(PeerMessage* peerMessage) {
  if(countMessageInQueue() == 0) {
    sendNow(peerMessage);
  } else {
    addMessage(peerMessage);
  }
}

void SendMessageQueue::sendHandshake() {
  peerConnection->sendHandshake();
}

void SendMessageQueue::abortPiece() {
  cancelAllRequest(piece);
  torrentMan->cancelPiece(piece);
}

Piece& SendMessageQueue::getDownloadPiece() {
  if(Piece::isNull(piece)) {
    throw new DlAbortEx("current piece is null");
  }
  return piece;
}

void SendMessageQueue::setPeerMessageCommonProperty(PeerMessage* peerMessage) {
  peerMessage->setPeer(peer);
  peerMessage->setCuid(cuid);
  peerMessage->setSendMessageQueue(this);  
}

RequestMessage* SendMessageQueue::createRequestMessage(int blockIndex) {
  RequestMessage* msg = new RequestMessage();
  setPeerMessageCommonProperty(msg);
  msg->setIndex(piece.getIndex());
  msg->setBegin(blockIndex*piece.getBlockLength());
  msg->setLength(piece.getBlockLength(blockIndex));
  msg->setBlockIndex(blockIndex);
  return msg;
}

CancelMessage* SendMessageQueue::createCancelMessage(int index, int begin, int length) {
  CancelMessage* msg = new CancelMessage();
  setPeerMessageCommonProperty(msg);
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setLength(length);
  return msg;
}

PieceMessage* SendMessageQueue::createPieceMessage(int index, int begin, int length) {
  PieceMessage* msg = new PieceMessage();
  setPeerMessageCommonProperty(msg);
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setBlockLength(length);
  return msg;  
}

HaveMessage* SendMessageQueue::createHaveMessage(int index) {
  HaveMessage* msg = new HaveMessage();
  setPeerMessageCommonProperty(msg);
  msg->setIndex(index);
  return msg;
}

ChokeMessage* SendMessageQueue::createChokeMessage() {
  ChokeMessage* msg = new ChokeMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

UnchokeMessage* SendMessageQueue::createUnchokeMessage() {
  UnchokeMessage* msg = new UnchokeMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

InterestedMessage* SendMessageQueue::createInterestedMessage() {
  InterestedMessage* msg = new InterestedMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

NotInterestedMessage* SendMessageQueue::createNotInterestedMessage() {
  NotInterestedMessage* msg = new NotInterestedMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

BitfieldMessage* SendMessageQueue::createBitfieldMessage() {
  BitfieldMessage* msg = new BitfieldMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

KeepAliveMessage* SendMessageQueue::createKeepAliveMessage() {
  KeepAliveMessage* msg = new KeepAliveMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

