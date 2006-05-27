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
#include "SuggestPieceMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"

SuggestPieceMessage* SuggestPieceMessage::create(const char* data, int dataLength) {
  if(dataLength != 5) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "suggest piece", dataLength, 5);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "suggest piece", ID);
  }
  SuggestPieceMessage* suggestPieceMessage = new SuggestPieceMessage();
  suggestPieceMessage->setIndex(PeerMessageUtil::getIntParam(data, 1));
  return suggestPieceMessage;
}

void SuggestPieceMessage::receivedAction() {
  // TODO Current implementation ignores this message.
}

const char* SuggestPieceMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 5, 4bytes
     * id --- 13, 1byte
     * piece index --- index, 4bytes
     * total: 9bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
  }
  return msg;
}

int SuggestPieceMessage::getMessageLength() {
  return sizeof(msg);
}

void SuggestPieceMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
}

string SuggestPieceMessage::toString() const {
  return "suggest piece index="+Util::itos(index);
}
