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
#include "HaveMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"

HaveMessage* HaveMessage::create(const char* data, int dataLength) {
  if(dataLength != 5) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "have", dataLength, 5);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "have", ID);
  }
  HaveMessage* haveMessage = new HaveMessage();
  haveMessage->setIndex(PeerMessageUtil::getIntParam(data, 1));
  return haveMessage;
}

void HaveMessage::receivedAction() {
  peer->updateBitfield(index, 1);
}

bool HaveMessage::sendPredicate() const {
  return !peer->hasPiece(index);
}

const char* HaveMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 5, 4bytes
     * id --- 4, 1byte
     * piece index --- index, 4bytes
     * total: 9bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
  }
  return msg;
}

int HaveMessage::getMessageLength() {
  return sizeof(msg);
}

void HaveMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
}

string HaveMessage::toString() const {
  return "have index="+Util::itos(index);
}
