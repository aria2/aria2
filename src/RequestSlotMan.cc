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
#include "RequestSlotMan.h"

void RequestSlotMan::addRequestSlot(const RequestSlot& requestSlot) {
  requestSlots.push_back(requestSlot);
}

void RequestSlotMan::deleteRequestSlot(const RequestSlot& requestSlot) {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    if(*itr == requestSlot) {
      requestSlots.erase(itr);
      break;
    }
  }
}

void RequestSlotMan::deleteAllRequestSlot(Piece& piece) {
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

void RequestSlotMan::deleteTimedoutRequestSlot(Piece& piece) {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    if(itr->isTimeout(timeout)) {
      logger->debug("CUID#%d - deleting requestslot blockIndex %d because of time out", cuid,
		    itr->getBlockIndex());
      if(!Piece::isNull(piece)) {
	piece.cancelBlock(itr->getBlockIndex());
      }
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }
  torrentMan->updatePiece(piece);
}

void RequestSlotMan::deleteCompletedRequestSlot(const Piece& piece) {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    if(Piece::isNull(piece) || piece.hasBlock(itr->getBlockIndex())) {
      logger->debug("CUID#%d - deleting requestslot blockIndex %d because the block is already acquired.", cuid,
		    itr->getBlockIndex());
      PendingMessage pendingMessage =
	PendingMessage::createCancelMessage(itr->getIndex(),
					    itr->getBegin(),
					    itr->getLength(),
					    peerConnection);
      pendingMessages->push_back(pendingMessage);
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }
}

RequestSlot RequestSlotMan::getCorrespoindingRequestSlot(const PeerMessage* pieceMessage) const {
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
