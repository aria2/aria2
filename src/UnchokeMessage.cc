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
#include "UnchokeMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"

UnchokeMessage* UnchokeMessage::create(const char* data, int dataLength) {
  if(dataLength != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "unchoke", dataLength, 1);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "unchoke", ID);
  }
  UnchokeMessage* unchokeMessage = new UnchokeMessage();
  return unchokeMessage;
}

void UnchokeMessage::receivedAction() {
  peer->peerChoking = false;
}

bool UnchokeMessage::sendPredicate() const {
  return peer->amChoking;
}

const char* UnchokeMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 1, 4bytes
     * id --- 1, 1byte
     * total: 5bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, ID);
  }
  return msg;
}

int UnchokeMessage::getMessageLength() {
  return sizeof(msg);
}

void UnchokeMessage::onSendComplete() {
  peer->amChoking = false;
}

string UnchokeMessage::toString() const {
  return "unchoke";
}
