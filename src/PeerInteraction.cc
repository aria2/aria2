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
				 const Socket* socket,
				 const Option* op,
				 TorrentMan* torrentMan,
				 Peer* peer)
  :cuid(cuid),
   uploadLimit(0),
   torrentMan(torrentMan),
   peer(peer) {
  peerConnection = new PeerConnection(cuid, socket, op);
  logger = LogFactory::getInstance();
}

PeerInteraction::~PeerInteraction() {
  delete peerConnection;
  for_each(messageQueue.begin(), messageQueue.end(), Deleter());
}

class MsgPushBack {
private:
  MessageQueue* messageQueue;
public:
  MsgPushBack(MessageQueue* messageQueue):messageQueue(messageQueue) {}

  void operator()(PeerMessage* msg) {
    messageQueue->push_back(msg);
  }
};

bool PeerInteraction::isSendingMessageInProgress() const {
  if(messageQueue.size() > 0) {
    PeerMessage* peerMessage = messageQueue.front();
    if(peerMessage->isInProgress()) {
      return true;
    }
  }
  return false;
}

void PeerInteraction::sendMessages(int uploadSpeed) {
  MessageQueue tempQueue;
  while(messageQueue.size() > 0) {
    PeerMessage* msg = messageQueue.front();
    messageQueue.pop_front();
    if(uploadLimit != 0 && uploadLimit*1024 <= uploadSpeed &&
       msg->getId() == PieceMessage::ID && !msg->isInProgress()) {
      //!((PieceMessage*)msg)->isPendingCountMax()) {
      //((PieceMessage*)msg)->incrementPendingCount();
      tempQueue.push_back(msg);
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
  for_each(tempQueue.begin(), tempQueue.end(), MsgPushBack(&messageQueue));
}

void PeerInteraction::addMessage(PeerMessage* peerMessage) {
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

void PeerInteraction::rejectAllPieceMessageInQueue() {
  MessageQueue tempQueue;
  for(MessageQueue::iterator itr = messageQueue.begin();
      itr != messageQueue.end();) {
    // Don't delete piece message which is in the allowed fast set.
    if((*itr)->getId() == PieceMessage::ID && !(*itr)->isInProgress()
       && !isInFastSet(((PieceMessage*)*itr)->getIndex())) {
      PieceMessage* pieceMessage = (PieceMessage*)*itr;
      logger->debug("CUID#%d - Reject piece message in queue because"
		    " peer has been choked. index=%d, begin=%d, length=%d",
		    cuid,
		    pieceMessage->getIndex(),
		    pieceMessage->getBegin(),
		    pieceMessage->getBlockLength());
      if(peer->isFastExtensionEnabled()) {
	tempQueue.push_back(createRejectMessage(pieceMessage->getIndex(),
						pieceMessage->getBegin(),
						pieceMessage->getBlockLength()));
      }
      delete pieceMessage;
      itr = messageQueue.erase(itr);
    } else {
      itr++;
    }
  }
  for_each(tempQueue.begin(), tempQueue.end(), MsgPushBack(&messageQueue));
}

void PeerInteraction::rejectPieceMessageInQueue(int index, int begin, int length) {
  MessageQueue tempQueue;
  for(MessageQueue::iterator itr = messageQueue.begin();
      itr != messageQueue.end();) {
    if((*itr)->getId() == PieceMessage::ID && !(*itr)->isInProgress()) {
      PieceMessage* pieceMessage = (PieceMessage*)*itr;
      if(pieceMessage->getIndex() == index &&
	 pieceMessage->getBegin() == begin &&
	 pieceMessage->getBlockLength() == length) {
	logger->debug("CUID#%d - Reject piece message in queue because cancel"
		      " message received. index=%d, begin=%d, length=%d",
		      cuid, index, begin, length);
	delete pieceMessage;
	itr = messageQueue.erase(itr);
	if(peer->isFastExtensionEnabled()) {
	  tempQueue.push_back(createRejectMessage(index, begin, length));
	}
      } else {
	itr++;
      }
    } else {
      itr++;
    }
  }
  for_each(tempQueue.begin(), tempQueue.end(), MsgPushBack(&messageQueue));
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
    for(MessageQueue::iterator itr = messageQueue.begin();
	itr != messageQueue.end();) {
      if((*itr)->getId() == RequestMessage::ID &&
	!(*itr)->isInProgress() &&
	 ((RequestMessage*)*itr)->getIndex() == piece.getIndex()) {
	delete *itr;
	itr = messageQueue.erase(itr);
      } else {
	itr++;
      }
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

void PeerInteraction::deleteTimeoutRequestSlot() {
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
    } else {
      itr++;
    }
  }
  updatePiece();
}

void PeerInteraction::deleteCompletedRequestSlot() {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    RequestSlot& slot = *itr;
    Piece piece = getDownloadPiece(slot.getIndex());
    if(piece.hasBlock(slot.getBlockIndex()) ||
       torrentMan->hasPiece(piece.getIndex())) {
      logger->debug("CUID#%d - Deleting request slot blockIndex=%d because"
		    " the block has been acquired.", cuid,
		    slot.getBlockIndex());
      addMessage(createCancelMessage(slot.getIndex(),
				     slot.getBegin(),
				     slot.getLength()));
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }
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

HandshakeMessage* PeerInteraction::receiveHandshake() {
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
  if(handshakeMessage->isFastExtensionSupported()) {
    peer->setFastExtensionEnabled(true);
    logger->info("CUID#%d - Fast extension enabled.");
  }
  return handshakeMessage;
}

HandshakeMessage* PeerInteraction::createHandshakeMessage(const char* msg, int msgLength) {
  HandshakeMessage* message = HandshakeMessage::create(msg, msgLength);

  setPeerMessageCommonProperty(message);
  return message;
}

PeerMessage* PeerInteraction::receiveMessage() {
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

PeerMessage* PeerInteraction::createPeerMessage(const char* msg, int msgLength) {
  PeerMessage* peerMessage;
  if(msgLength == 0) {
    // keep-alive
    peerMessage = new KeepAliveMessage();
  } else {
    int id = PeerMessageUtil::getId(msg);
    switch(id) {
    case ChokeMessage::ID:
      peerMessage = ChokeMessage::create(msg, msgLength);
      break;
    case UnchokeMessage::ID:
      peerMessage = UnchokeMessage::create(msg, msgLength);
      break;
    case InterestedMessage::ID:
      peerMessage = InterestedMessage::create(msg, msgLength);
      break;
    case NotInterestedMessage::ID:
      peerMessage = NotInterestedMessage::create(msg, msgLength);
      break;
    case HaveMessage::ID:
      peerMessage = HaveMessage::create(msg, msgLength);
      ((HaveMessage*)peerMessage)->setPieces(torrentMan->pieces);
      break;
    case BitfieldMessage::ID:
      peerMessage = BitfieldMessage::create(msg, msgLength);
      ((BitfieldMessage*)peerMessage)->setPieces(torrentMan->pieces);
      break;
    case RequestMessage::ID:
      peerMessage = RequestMessage::create(msg, msgLength);
      ((RequestMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((RequestMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((RequestMessage*)peerMessage)->getIndex()));
      break;
    case CancelMessage::ID:
      peerMessage = CancelMessage::create(msg, msgLength);
      ((CancelMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((CancelMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((CancelMessage*)peerMessage)->getIndex()));
      break;
    case PieceMessage::ID:
      peerMessage = PieceMessage::create(msg, msgLength);
      ((PieceMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((PieceMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((PieceMessage*)peerMessage)->getIndex()));
      break;
    case PortMessage::ID:
      peerMessage = PortMessage::create(msg, msgLength);
      break;
    case HaveAllMessage::ID:
      peerMessage = HaveAllMessage::create(msg, msgLength);
      break;
    case HaveNoneMessage::ID:
      peerMessage = HaveNoneMessage::create(msg, msgLength);
      break;
    case RejectMessage::ID:
      peerMessage = RejectMessage::create(msg, msgLength);
      ((RejectMessage*)peerMessage)->setPieces(torrentMan->pieces);
      ((RejectMessage*)peerMessage)->setPieceLength(torrentMan->getPieceLength(((RejectMessage*)peerMessage)->getIndex()));
      break;
    case SuggestPieceMessage::ID:
      peerMessage = SuggestPieceMessage::create(msg, msgLength);
      ((SuggestPieceMessage*)peerMessage)->setPieces(torrentMan->pieces);
      break;
    case AllowedFastMessage::ID:
      peerMessage = AllowedFastMessage::create(msg, msgLength);
      ((AllowedFastMessage*)peerMessage)->setPieces(torrentMan->pieces);
      break;
    default:
      throw new DlAbortEx("invalid message id. id = %d", id);
    }
  }
  setPeerMessageCommonProperty(peerMessage);
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
  int index = torrentMan->getMissingPieceIndex(peer);
  if(pieces.empty() && index == -1) {
    if(peer->amInterested) {
      logger->debug("CUID#%d - Not interested in the peer", cuid);
      addMessage(createNotInterestedMessage());
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
      addMessage(createInterestedMessage());
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
  if(peer->getLatency() < 300) {
    MAX_PENDING_REQUEST = 24;
  } else if(peer->getLatency() < 600) {
    MAX_PENDING_REQUEST = 18;
  } else if(peer->getLatency() < 1000) {
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
	  addMessage(createRequestMessage(piece.getIndex(), blockIndex));
	  count++;
	}
      }
    } else {
      while(countRequestSlot() < MAX_PENDING_REQUEST) {
	int blockIndex = piece.getMissingUnusedBlockIndex();
	if(blockIndex == -1) {
	  break;
	}
	addMessage(createRequestMessage(piece.getIndex(), blockIndex));
      }
    }
    if(countRequestSlot() >= MAX_PENDING_REQUEST) {
      break;
    }
  }
  updatePiece();
}

void PeerInteraction::sendHandshake() {
  HandshakeMessage* handshake = new HandshakeMessage();
  memcpy(handshake->infoHash, torrentMan->getInfoHash(), INFO_HASH_LENGTH);
  memcpy(handshake->peerId, torrentMan->peerId.c_str(), PEER_ID_LENGTH);
  setPeerMessageCommonProperty(handshake);
  addMessage(handshake);
  sendMessages(0);
}

void PeerInteraction::sendBitfield() {
  if(peer->isFastExtensionEnabled()) {
    if(torrentMan->hasAllPieces()) {
      addMessage(createHaveAllMessage());
    } else if(torrentMan->getDownloadLength() > 0) {
      addMessage(createBitfieldMessage());
    } else {
      addMessage(createHaveNoneMessage());
    }
  } else {
    if(torrentMan->getDownloadLength() > 0) {
      addMessage(createBitfieldMessage());
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
      addMessage(createAllowedFastMessage(*itr));
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

void PeerInteraction::setPeerMessageCommonProperty(PeerMessage* peerMessage) {
  peerMessage->setPeer(peer);
  peerMessage->setCuid(cuid);
  peerMessage->setPeerInteraction(this);  
}

RequestMessage* PeerInteraction::createRequestMessage(int index, int blockIndex) {
  RequestMessage* msg = new RequestMessage();
  Piece piece = getDownloadPiece(index);
  msg->setIndex(piece.getIndex());
  msg->setBegin(blockIndex*piece.getBlockLength());
  msg->setLength(piece.getBlockLength(blockIndex));
  msg->setBlockIndex(blockIndex);
  setPeerMessageCommonProperty(msg);
  return msg;
}

CancelMessage* PeerInteraction::createCancelMessage(int index, int begin, int length) {
  CancelMessage* msg = new CancelMessage();
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setLength(length);
  setPeerMessageCommonProperty(msg);
  return msg;
}

PieceMessage* PeerInteraction::createPieceMessage(int index, int begin, int length) {
  PieceMessage* msg = new PieceMessage();
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setBlockLength(length);
  setPeerMessageCommonProperty(msg);
  return msg;  
}

HaveMessage* PeerInteraction::createHaveMessage(int index) {
  HaveMessage* msg = new HaveMessage();
  msg->setIndex(index);
  setPeerMessageCommonProperty(msg);
  return msg;
}

ChokeMessage* PeerInteraction::createChokeMessage() {
  ChokeMessage* msg = new ChokeMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

UnchokeMessage* PeerInteraction::createUnchokeMessage() {
  UnchokeMessage* msg = new UnchokeMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

InterestedMessage* PeerInteraction::createInterestedMessage() {
  InterestedMessage* msg = new InterestedMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

NotInterestedMessage* PeerInteraction::createNotInterestedMessage() {
  NotInterestedMessage* msg = new NotInterestedMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

BitfieldMessage* PeerInteraction::createBitfieldMessage() {
  BitfieldMessage* msg = new BitfieldMessage();
  msg->setBitfield(getTorrentMan()->getBitfield(),
		   getTorrentMan()->getBitfieldLength());
  setPeerMessageCommonProperty(msg);
  return msg;
}

KeepAliveMessage* PeerInteraction::createKeepAliveMessage() {
  KeepAliveMessage* msg = new KeepAliveMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

HaveAllMessage* PeerInteraction::createHaveAllMessage() {
  HaveAllMessage* msg = new HaveAllMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

HaveNoneMessage* PeerInteraction::createHaveNoneMessage() {
  HaveNoneMessage* msg = new HaveNoneMessage();
  setPeerMessageCommonProperty(msg);
  return msg;
}

RejectMessage* PeerInteraction::createRejectMessage(int index, int begin, int length) {
  RejectMessage* msg = new RejectMessage();
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setLength(length);
  setPeerMessageCommonProperty(msg);
  return msg;
}

AllowedFastMessage* PeerInteraction::createAllowedFastMessage(int index) {
  AllowedFastMessage* msg = new AllowedFastMessage();
  msg->setIndex(index);
  setPeerMessageCommonProperty(msg);
  return msg;
}
