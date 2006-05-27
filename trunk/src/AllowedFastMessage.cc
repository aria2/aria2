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
#include "AllowedFastMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"

AllowedFastMessage* AllowedFastMessage::create(const char* data, int dataLength) {
  if(dataLength != 5) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "allowed fast", dataLength, 5);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "allowed fast", ID);
  }
  AllowedFastMessage* allowedFastMessage = new AllowedFastMessage();
  allowedFastMessage->setIndex(PeerMessageUtil::getIntParam(data, 1));
  return allowedFastMessage;
}

void AllowedFastMessage::receivedAction() {
  if(!peer->isFastExtensionEnabled()) {
    throw new DlAbortEx("%s received while fast extension is disabled",
			toString().c_str());
  }
  peer->addFastSetIndex(index);
}

const char* AllowedFastMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 5, 4bytes
     * id --- 17, 1byte
     * piece index --- index, 4bytes
     * total: 9bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
  }
  return msg;
}

int AllowedFastMessage::getMessageLength() {
  return sizeof(msg);
}

void AllowedFastMessage::onSendComplete() {
  peerInteraction->addFastSetIndex(index);
}

void AllowedFastMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
}

string AllowedFastMessage::toString() const {
  return "allowed fast index="+Util::itos(index);
}
