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
#include "NotInterestedMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"

NotInterestedMessage* NotInterestedMessage::create(const char* data, int dataLength) {
  if(dataLength != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "not interested", dataLength, 1);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "not interested", ID);
  }
  NotInterestedMessage* notInterestedMessage = new NotInterestedMessage();
  return notInterestedMessage;
}

void NotInterestedMessage::receivedAction() {
  peer->peerInterested = false;
}

bool NotInterestedMessage::sendPredicate() const {
  return peer->amInterested;
}

const char* NotInterestedMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 1, 4bytes
     * id --- 3, 1byte
     * total: 5bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, ID);
  }
  return msg;
}

int NotInterestedMessage::getMessageLength() {
  return sizeof(msg);
}

void NotInterestedMessage::onSendComplete() {
  peer->amInterested = false;
}

string NotInterestedMessage::toString() const {
  return "not interested";
}
