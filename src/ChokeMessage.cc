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
#include "ChokeMessage.h"
#include "PeerInteraction.h"
#include "message.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"

ChokeMessage* ChokeMessage::create(const char* data, int dataLength) {
  if(dataLength != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "choke", dataLength, 1);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "choke", ID);
  }
  ChokeMessage* chokeMessage = new ChokeMessage();
  return chokeMessage;
}

void ChokeMessage::receivedAction() {
  peer->peerChoking = true;
  peer->snubbing = false;
  peerInteraction->onChoked();
}

bool ChokeMessage::sendPredicate() const {
  return !peer->amChoking;
}

const char* ChokeMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 1, 4bytes
     * id --- 0, 1byte
     * total: 5bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, ID);
  }
  return msg;
}

int ChokeMessage::getMessageLength() {
  return sizeof(msg);
}

void ChokeMessage::onSendComplete() {
  peer->amChoking = true;
  peerInteraction->rejectAllPieceMessageInQueue();
}

string ChokeMessage::toString() const {
  return "choke";
}
