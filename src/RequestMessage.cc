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
#include "RequestMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"

RequestMessage* RequestMessage::create(const char* data, int dataLength) {
  if(dataLength != 13) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "request", dataLength, 13);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "request", ID);
  }
  RequestMessage* requestMessage = new RequestMessage();
  requestMessage->setIndex(PeerMessageUtil::getIntParam(data, 1));
  requestMessage->setBegin(PeerMessageUtil::getIntParam(data, 5));
  requestMessage->setLength(PeerMessageUtil::getIntParam(data, 9));
  return requestMessage;
}

void RequestMessage::receivedAction() {
  if(pieceStorage->hasPiece(index) &&
     (!peer->amChoking ||
      peer->amChoking && peerInteraction->isInFastSet(index))) {
    peerInteraction->addMessage(peerInteraction->getPeerMessageFactory()->
				createPieceMessage(index, begin, length));
  } else {
    if(peer->isFastExtensionEnabled()) {
      peerInteraction->addMessage(peerInteraction->getPeerMessageFactory()->
				  createRejectMessage(index, begin, length));
    }
  }
}

const char* RequestMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 13, 4bytes
     * id --- 6, 1byte
     * index --- index, 4bytes
     * begin --- begin, 4bytes
     * length --- length, 4bytes
     * total: 17bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
    PeerMessageUtil::setIntParam(&msg[9], begin);
    PeerMessageUtil::setIntParam(&msg[13], length);
  }
  return msg;
}

int RequestMessage::getMessageLength() {
  return sizeof(msg);
}

void RequestMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
  PeerMessageUtil::checkBegin(begin, pieceLength);
  PeerMessageUtil::checkLength(length);
  PeerMessageUtil::checkRange(begin, length, pieceLength);
}

string RequestMessage::toString() const {
  return "request index="+Util::itos(index)+", begin="+Util::itos(begin)+
    ", length="+Util::itos(length);
}

void RequestMessage::onPush() {
  RequestSlot requestSlot(index, begin, length, blockIndex);
  peerInteraction->addRequestSlot(requestSlot);
}

void RequestMessage::onAbortPiece(const Piece& piece) {
  if(!invalidate &&
     !inProgress &&
     this->index == piece.getIndex()) {
    invalidate = true;
  }
}
