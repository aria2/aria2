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
#include "PeerInteraction.h"
#include "LogFactory.h"
#include "DlAbortEx.h"
#include "KeepAliveMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include <netinet/in.h>

PeerInteraction::PeerInteraction(int cuid,
				 const PeerHandle& peer,
				 const SocketHandle& socket,
				 const Option* op,
				 TorrentMan* torrentMan)
  :cuid(cuid),
   uploadLimit(0),
   torrentMan(torrentMan),
   peer(peer),
   quickReplied(false) {
  peerConnection = new PeerConnection(cuid, socket, op);
  peerMessageFactory = new PeerMessageFactory(cuid, this, peer);
  logger = LogFactory::getInstance();
}

PeerInteraction::~PeerInteraction() {
  delete peerConnection;
  delete peerMessageFactory;
}

bool PeerInteraction::isSendingMessageInProgress() const {
  if(messageQueue.size() > 0) {
    const PeerMessageHandle& peerMessage = messageQueue.front();
    if(peerMessage->isInProgress()) {
      return true;
    }
  }
  return false;
}

void PeerInteraction::sendMessages(int uploadSpeed) {
  MessageQueue tempQueue;
  while(messageQueue.size() > 0) {
    PeerMessageHandle msg = messageQueue.front();
    messageQueue.pop_front();
    if(uploadLimit != 0 && uploadLimit*1024 <= uploadSpeed &&
       msg->isUploading() && !msg->isInProgress()) {
      tempQueue.push_back(msg);
    } else {
      msg->send();
      if(msg->isInProgress()) {
	messageQueue.push_front(msg);
	break;
      }
    }
  }
  copy(tempQueue.begin(), tempQueue.end(), back_inserter(messageQueue));
}

void PeerInteraction::addMessage(const PeerMessageHandle& peerMessage) {
  peerMessage->onPush();
  messageQueue.push_back(peerMessage);
}

void PeerInteraction::addRequestSlot(const RequestSlot& requestSlot) {
  requestSlots.push_back(requestSlot);
}

void PeerInteraction::rejectAllPieceMessageInQueue() {
  int size = messageQueue.size();
  for(int i = 0; i < size; i++) {
    messageQueue.at(i)->onChoked();
  }
}

void PeerInteraction::rejectPieceMessageInQueue(int index, int begin, int length) {
  int size = messageQueue.size();
  for(int i = 0; i < size; i++) {
    messageQueue.at(i)->onCanceled(index, begin, length);
  }
}

void PeerInteraction::onChoked() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end();) {
    Piece& piece = *itr;
    if(!peer->isInFastSet(piece.getIndex())) {
      abortPiece(piece);
      itr = pieces.erase(itr);
    } else {
      itr++;
    }
  }
}

void PeerInteraction::abortAllPieces() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end();) {
    abortPiece(*itr);
    itr = pieces.erase(itr);
  }
}

void PeerInteraction::abortPiece(Piece& piece) {
  if(!Piece::isNull(piece)) {
    int size = messageQueue.size();
    for(int i = 0; i < size; i++) {
      messageQueue.at(i)->onAbortPiece(piece);
    }
    for(RequestSlots::iterator itr = requestSlots.begin();
	itr != requestSlots.end();) {
      if(itr->getIndex() == piece.getIndex()) {
	logger->debug("CUID#%d - Deleting request slot blockIndex=%d"
		      " because piece was canceled",
		      cuid,
		      itr->getBlockIndex());
	piece.cancelBlock(itr->getBlockIndex());
	itr = requestSlots.erase(itr);
      } else {
	itr++;
      }
    }
    torrentMan->cancelPiece(piece);
  }
}


void PeerInteraction::deleteRequestSlot(const RequestSlot& requestSlot) {
  RequestSlots::iterator itr = find(requestSlots.begin(), requestSlots.end(),
				    requestSlot);
  if(itr != requestSlots.end()) {
    requestSlots.erase(itr);
  }
}

void PeerInteraction::checkRequestSlot() {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    RequestSlot& slot = *itr;
    if(slot.isTimeout(REQUEST_TIME_OUT)) {
      logger->debug("CUID#%d - Deleting request slot blockIndex=%d"
		    " because of time out",
		    cuid,
		    slot.getBlockIndex());
      Piece& piece = getDownloadPiece(slot.getIndex());
      piece.cancelBlock(slot.getBlockIndex());
      itr = requestSlots.erase(itr);
      peer->snubbing = true;
    } else {
      Piece piece = getDownloadPiece(slot.getIndex());
      if(piece.hasBlock(slot.getBlockIndex()) ||
	 torrentMan->hasPiece(piece.getIndex())) {
	logger->debug("CUID#%d - Deleting request slot blockIndex=%d because"
		      " the block has been acquired.", cuid,
		      slot.getBlockIndex());
	addMessage(peerMessageFactory->createCancelMessage(slot.getIndex(),
							   slot.getBegin(),
							   slot.getLength()));
	itr = requestSlots.erase(itr); 
      } else {
	itr++;
      }
    }
  }
  updatePiece();
}

bool PeerInteraction::isInRequestSlot(int index, int blockIndex) const {
  for(RequestSlots::const_iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    const RequestSlot& slot = *itr;
    if(slot.getIndex() == index && slot.getBlockIndex() == blockIndex) {
      return true;
    }
  }
  return false;
}

RequestSlot PeerInteraction::getCorrespondingRequestSlot(int index,
							 int begin,
							 int length) const {
  for(RequestSlots::const_iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    const RequestSlot& slot = *itr;
    if(slot.getIndex() == index &&
       slot.getBegin() == begin &&
       slot.getLength() == length) {
      return slot;
    }
  }
  return RequestSlot::nullSlot;
}

int PeerInteraction::countMessageInQueue() const {
  return messageQueue.size();
}

int PeerInteraction::countRequestSlot() const {
  return requestSlots.size();
}

PeerMessageHandle PeerInteraction::receiveHandshake(bool quickReply) {
  char msg[HANDSHAKE_MESSAGE_LENGTH];
  int msgLength = HANDSHAKE_MESSAGE_LENGTH;
  bool retval = peerConnection->receiveHandshake(msg, msgLength);
  // To handle tracker's NAT-checking feature
  if(!quickReplied && quickReply && msgLength >= 48) {
    quickReplied = true;
    // check info_hash
    if(memcmp(torrentMan->getInfoHash(), &msg[28], INFO_HASH_LENGTH) == 0) {
      sendHandshake();
    }
  }
  if(!retval) {
    return NULL;
  }
  PeerMessageHandle handshakeMessage(peerMessageFactory->createHandshakeMessage(msg, msgLength));
  handshakeMessage->check();
  if(((HandshakeMessage*)handshakeMessage.get())->isFastExtensionSupported()) {
    peer->setFastExtensionEnabled(true);
    logger->info("CUID#%d - Fast extension enabled.", cuid);
  }
  return handshakeMessage;
}

PeerMessageHandle PeerInteraction::receiveMessage() {
  char msg[MAX_PAYLOAD_LEN];
  int msgLength = 0;
  if(!peerConnection->receiveMessage(msg, msgLength)) {
    return NULL;
  }
  PeerMessageHandle peerMessage =
    peerMessageFactory->createPeerMessage(msg, msgLength);
  peerMessage->check();
  return peerMessage;
}

void PeerInteraction::syncPiece() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    torrentMan->syncPiece(*itr);
  }
}

void PeerInteraction::updatePiece() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    torrentMan->updatePiece(*itr);
  }
}

void PeerInteraction::getNewPieceAndSendInterest(int pieceNum) {
  if(pieces.empty() && !torrentMan->hasMissingPiece(peer)) {
    if(peer->amInterested) {
      logger->debug("CUID#%d - Not interested in the peer", cuid);
      addMessage(peerMessageFactory->createNotInterestedMessage());
    }
  } else {
    if(peer->peerChoking) {
      onChoked();
      if(peer->isFastExtensionEnabled()) {
	while((int)pieces.size() < pieceNum) {
	  Piece piece = torrentMan->getMissingFastPiece(peer);
	  if(Piece::isNull(piece)) {
	    break;
	  } else {
	    pieces.push_back(piece);
	  }
	}
      }
    } else {
      while((int)pieces.size() < pieceNum) {
	Piece piece = torrentMan->getMissingPiece(peer);
	if(Piece::isNull(piece)) {
	  break;
	} else {
	  pieces.push_back(piece);
	}
      }
    }
    if(!peer->amInterested) {
      logger->debug("CUID#%d - Interested in the peer", cuid);
      addMessage(peerMessageFactory->createInterestedMessage());
    }
  }
}

void PeerInteraction::addRequests() {
  // Abort downloading of completed piece.
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end();) {
    Piece& piece = *itr;
    if(piece.pieceComplete()) {
      abortPiece(piece);
      itr = pieces.erase(itr);
    } else {
      itr++;
    }
  }
  int MAX_PENDING_REQUEST;
  if(peer->getLatency() < 500) {
    MAX_PENDING_REQUEST = 24;
  } else if(peer->getLatency() < 1500) {
    MAX_PENDING_REQUEST = 12;
  } else {
    MAX_PENDING_REQUEST = 6;
  }
  int pieceNum;
  if(torrentMan->isEndGame()) {
    pieceNum = 1;
  } else {
    int blocks = DIV_FLOOR(torrentMan->pieceLength, BLOCK_LENGTH);
    pieceNum = DIV_FLOOR(MAX_PENDING_REQUEST, blocks);
  }
  getNewPieceAndSendInterest(pieceNum);
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    Piece& piece = *itr;
    if(torrentMan->isEndGame()) {
      BlockIndexes missingBlockIndexes = piece.getAllMissingBlockIndexes();
      random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end());
      int count = countRequestSlot();
      for(BlockIndexes::const_iterator bitr = missingBlockIndexes.begin();
	  bitr != missingBlockIndexes.end() && count < MAX_PENDING_REQUEST;
	  bitr++) {
	int blockIndex = *bitr;
	if(!isInRequestSlot(piece.getIndex(), blockIndex)) {
	  addMessage(peerMessageFactory->createRequestMessage(piece,
							      blockIndex));
	  count++;
	}
      }
    } else {
      while(countRequestSlot() < MAX_PENDING_REQUEST) {
	int blockIndex = piece.getMissingUnusedBlockIndex();
	if(blockIndex == -1) {
	  break;
	}
	addMessage(peerMessageFactory->createRequestMessage(piece,
							    blockIndex));
      }
    }
    if(countRequestSlot() >= MAX_PENDING_REQUEST) {
      break;
    }
  }
  updatePiece();
}

void PeerInteraction::sendHandshake() {
  PeerMessageHandle handle =
    peerMessageFactory->createHandshakeMessage(torrentMan->getInfoHash(),
					       torrentMan->peerId.c_str());
  addMessage(handle);
  sendMessages(0);
}

void PeerInteraction::sendBitfield() {
  if(peer->isFastExtensionEnabled()) {
    if(torrentMan->hasAllPieces()) {
      addMessage(peerMessageFactory->createHaveAllMessage());
    } else if(torrentMan->getDownloadLength() > 0) {
      addMessage(peerMessageFactory->createBitfieldMessage());
    } else {
      addMessage(peerMessageFactory->createHaveNoneMessage());
    }
  } else {
    if(torrentMan->getDownloadLength() > 0) {
      addMessage(peerMessageFactory->createBitfieldMessage());
    }
  }
  sendMessages(0);
}

void PeerInteraction::sendAllowedFast() {
  if(peer->isFastExtensionEnabled()) {
    Integers fastSet = Util::computeFastSet(peer->ipaddr, torrentMan->getInfoHash(),
				   torrentMan->pieces, ALLOWED_FAST_SET_SIZE);
    for(Integers::const_iterator itr = fastSet.begin();
	itr != fastSet.end(); itr++) {
      addMessage(peerMessageFactory->createAllowedFastMessage(*itr));
    }
  }
}

Piece& PeerInteraction::getDownloadPiece(int index) {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    if(itr->getIndex() == index) {
      return *itr;
    }
  }
  throw new DlAbortEx("No such piece index=%d", index);
}

bool PeerInteraction::hasDownloadPiece(int index) const {
  for(Pieces::const_iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    if(itr->getIndex() == index) {
      return true;
    }
  }
  return false;
}

bool PeerInteraction::isInFastSet(int index) const {
  return find(fastSet.begin(), fastSet.end(), index) != fastSet.end();
}

void PeerInteraction::addFastSetIndex(int index) {
  if(!isInFastSet(index)) {
    fastSet.push_back(index);
  }
}
