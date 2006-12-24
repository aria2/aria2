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
#include "PeerInteraction.h"
#include "LogFactory.h"
#include "DlAbortEx.h"
#include "KeepAliveMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "prefs.h"
#include "BtRegistry.h"
#include <netinet/in.h>

PeerInteraction::PeerInteraction(int cuid,
				 const PeerHandle& peer,
				 const SocketHandle& socket,
				 const Option* op,
				 const BtContextHandle& btContext)
  :cuid(cuid),
   option(op),
   btContext(btContext),
   peerStorage(PEER_STORAGE(btContext)),
   pieceStorage(PIECE_STORAGE(btContext)),
   btAnnounce(BT_ANNOUNCE(btContext)),
   peer(peer),
   quickReplied(false) {
  peerConnection = new PeerConnection(cuid, socket, op);
  peerMessageFactory = new PeerMessageFactory(cuid, btContext, this, peer);
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

void PeerInteraction::sendMessages() {
  MessageQueue tempQueue;
  int uploadLimit = option->getAsInt(PREF_MAX_UPLOAD_LIMIT);
  while(messageQueue.size() > 0) {
    PeerMessageHandle msg = messageQueue.front();
    messageQueue.pop_front();
    if(uploadLimit > 0) {
      TransferStat stat = peerStorage->calculateStat();
      if(uploadLimit < stat.uploadSpeed &&
	 msg->isUploading() && !msg->isInProgress()) {
	tempQueue.push_back(msg);
	continue;
      }
    }
    msg->send();
    if(msg->isInProgress()) {
      messageQueue.push_front(msg);
      break;
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
    PieceHandle& piece = *itr;
    if(!peer->isInFastSet(piece->getIndex())) {
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

void PeerInteraction::abortPiece(const PieceHandle& piece) {
  if(!piece.isNull()) {
    int size = messageQueue.size();
    for(int i = 0; i < size; i++) {
      messageQueue.at(i)->onAbortPiece(piece);
    }
    for(RequestSlots::iterator itr = requestSlots.begin();
	itr != requestSlots.end();) {
      if(itr->getIndex() == piece->getIndex()) {
	logger->debug("CUID#%d - Deleting request slot blockIndex=%d"
		      " because piece was canceled",
		      cuid,
		      itr->getBlockIndex());
	piece->cancelBlock(itr->getBlockIndex());
	itr = requestSlots.erase(itr);
      } else {
	itr++;
      }
    }
    pieceStorage->cancelPiece(piece);
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
      PieceHandle piece = getDownloadPiece(slot.getIndex());
      piece->cancelBlock(slot.getBlockIndex());
      itr = requestSlots.erase(itr);
      peer->snubbing = true;
    } else {
      PieceHandle piece = getDownloadPiece(slot.getIndex());
      if(piece->hasBlock(slot.getBlockIndex()) ||
	 pieceStorage->hasPiece(piece->getIndex())) {
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
    if(memcmp(btContext->getInfoHash(), &msg[28], INFO_HASH_LENGTH) == 0) {
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
  /*
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    pieceStorage->syncPiece(*itr);
  }
  */
}

void PeerInteraction::updatePiece() {
  /*
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    pieceStorage->updatePiece(*itr);
  }
  */
}

void PeerInteraction::getNewPieceAndSendInterest(int pieceNum) {
  if(pieces.empty() && !pieceStorage->hasMissingPiece(peer)) {
    if(peer->amInterested) {
      logger->debug("CUID#%d - Not interested in the peer", cuid);
      addMessage(peerMessageFactory->createNotInterestedMessage());
    }
  } else {
    if(peer->peerChoking) {
      onChoked();
      if(peer->isFastExtensionEnabled()) {
	while((int)pieces.size() < pieceNum) {
	  PieceHandle piece = pieceStorage->getMissingFastPiece(peer);
	  if(piece.isNull()) {
	    break;
	  } else {
	    pieces.push_back(piece);
	  }
	}
      }
    } else {
      while((int)pieces.size() < pieceNum) {
	PieceHandle piece = pieceStorage->getMissingPiece(peer);
	if(piece.isNull()) {
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
    PieceHandle& piece = *itr;
    if(piece->pieceComplete()) {
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
  if(pieceStorage->isEndGame()) {
    pieceNum = 1;
  } else {
    int blocks = DIV_FLOOR(btContext->getPieceLength(), BLOCK_LENGTH);
    pieceNum = DIV_FLOOR(MAX_PENDING_REQUEST, blocks);
  }
  getNewPieceAndSendInterest(pieceNum);
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    PieceHandle& piece = *itr;
    if(pieceStorage->isEndGame()) {
      BlockIndexes missingBlockIndexes = piece->getAllMissingBlockIndexes();
      random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end());
      int count = countRequestSlot();
      for(BlockIndexes::const_iterator bitr = missingBlockIndexes.begin();
	  bitr != missingBlockIndexes.end() && count < MAX_PENDING_REQUEST;
	  bitr++) {
	int blockIndex = *bitr;
	if(!isInRequestSlot(piece->getIndex(), blockIndex)) {
	  addMessage(peerMessageFactory->createRequestMessage(piece,
							      blockIndex));
	  count++;
	}
      }
    } else {
      while(countRequestSlot() < MAX_PENDING_REQUEST) {
	int blockIndex = piece->getMissingUnusedBlockIndex();
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
    peerMessageFactory->createHandshakeMessage(btContext->getInfoHash(),
					       (const char*)btContext->getPeerId());
  addMessage(handle);
  sendMessages();
}

void PeerInteraction::sendBitfield() {
  if(peer->isFastExtensionEnabled()) {
    if(pieceStorage->downloadFinished()) {
      addMessage(peerMessageFactory->createHaveAllMessage());
    } else if(pieceStorage->getCompletedLength() > 0) {
      addMessage(peerMessageFactory->createBitfieldMessage());
    } else {
      addMessage(peerMessageFactory->createHaveNoneMessage());
    }
  } else {
    if(pieceStorage->getCompletedLength() > 0) {
      addMessage(peerMessageFactory->createBitfieldMessage());
    }
  }
  sendMessages();
}

void PeerInteraction::sendAllowedFast() {
  if(peer->isFastExtensionEnabled()) {
    Integers fastSet = Util::computeFastSet(peer->ipaddr,
					    btContext->getInfoHash(),
					    btContext->getNumPieces(),
					    ALLOWED_FAST_SET_SIZE);
    for(Integers::const_iterator itr = fastSet.begin();
	itr != fastSet.end(); itr++) {
      addMessage(peerMessageFactory->createAllowedFastMessage(*itr));
    }
  }
}

PieceHandle PeerInteraction::getDownloadPiece(int index) {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    if((*itr)->getIndex() == index) {
      return *itr;
    }
  }
  throw new DlAbortEx("No such piece index=%d", index);
}

bool PeerInteraction::hasDownloadPiece(int index) const {
  for(Pieces::const_iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    if((*itr)->getIndex() == index) {
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
