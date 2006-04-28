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

SendMessageQueue::SendMessageQueue(int cuid, PeerConnection* peerConnection,
				   TorrentMan* torrentMan)
  :cuid(cuid), uploadLimit(0) {
  requestSlotMan = new RequestSlotMan(cuid, &pendingMessages, peerConnection,
				      torrentMan);
  logger = LogFactory::getInstance();
}

SendMessageQueue::~SendMessageQueue() {
  delete requestSlotMan;
}

void SendMessageQueue::send(int uploadSpeed) {
  //logger->debug("SendMessageQueue:send start");
  int size = pendingMessages.size();
  for(int i = 0; i < size; i++) {
    PendingMessage msg = pendingMessages.front();
    pendingMessages.pop_front();
    if(uploadLimit != 0 && uploadSpeed >= uploadLimit*1024 &&
       msg.getPeerMessageId() == PeerMessage::PIECE && !msg.isInProgress()) {
      //logger->debug("upload speed limiter enabled, uploadSpeed=%d", uploadSpeed);
      pendingMessages.push_back(msg);
    } else {
      if(!msg.processMessage()) {
	pendingMessages.push_front(msg);
	break;
      }
    }
  }
  //logger->debug("SendMessageQueue:send end");
}

void SendMessageQueue::addPendingMessage(const PendingMessage& pendingMessage) {
  pendingMessages.push_back(pendingMessage);
  if(pendingMessage.getPeerMessageId() == PeerMessage::REQUEST) {
    RequestSlot requestSlot(pendingMessage.getIndex(),
			    pendingMessage.getBegin(),
			    pendingMessage.getLength(),
			    pendingMessage.getBlockIndex());
    requestSlotMan->addRequestSlot(requestSlot);
  }
}

void SendMessageQueue::deletePendingPieceMessage(const PeerMessage* cancelMessage) {
  for(PendingMessages::iterator itr = pendingMessages.begin();
      itr != pendingMessages.end();) {
    PendingMessage& pendingMessage = *itr;
    if(pendingMessage.getPeerMessageId() == PeerMessage::PIECE &&
       pendingMessage.getIndex() == cancelMessage->getIndex() &&
       pendingMessage.getBegin() == cancelMessage->getBegin() &&
       pendingMessage.getLength() == cancelMessage->getLength() &&
       !pendingMessage.isInProgress()) {
      logger->debug("CUID#%d - deleting pending piece message because cancel message received. index=%d, begin=%d, length=%d",
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

void SendMessageQueue::deletePendingRequestMessage() {
  for(PendingMessages::iterator itr = pendingMessages.begin();
      itr != pendingMessages.end();) {
    PendingMessage& pendingMessage = *itr;
    if(pendingMessage.getPeerMessageId() == PeerMessage::REQUEST) {
      itr = pendingMessages.erase(itr);
    } else {
      itr++;
    }
  }
}

void SendMessageQueue::deleteRequestSlot(const RequestSlot& requestSlot) {
  requestSlotMan->deleteRequestSlot(requestSlot);
}

void SendMessageQueue::deleteTimeoutRequestSlot(Piece& piece) {
  requestSlotMan->deleteTimeoutRequestSlot(piece);
}

void SendMessageQueue::deleteCompletedRequestSlot(const Piece& piece) {
  requestSlotMan->deleteCompletedRequestSlot(piece);
}

RequestSlot SendMessageQueue::getCorrespoindingRequestSlot(const PeerMessage* pieceMessage) const {
  return requestSlotMan->getCorrespoindingRequestSlot(pieceMessage);
}

void SendMessageQueue::cancelAllRequest() {
  cancelAllRequest(Piece::nullPiece);
}

void SendMessageQueue::cancelAllRequest(Piece& piece) {
  deletePendingRequestMessage();
  requestSlotMan->deleteAllRequestSlot(piece);
}

int SendMessageQueue::countPendingMessage() const {
  return pendingMessages.size();
}

int SendMessageQueue::countRequestSlot() const {
  return requestSlotMan->countRequestSlot();
}
