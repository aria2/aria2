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
#include "CancelMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"

CancelMessage* CancelMessage::create(const char* data, int dataLength) {
  if(dataLength != 13) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "cancel", dataLength, 13);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "cancel", ID);
  }
  CancelMessage* cancelMessage = new CancelMessage();
  cancelMessage->setIndex(PeerMessageUtil::getIntParam(data, 1));
  cancelMessage->setBegin(PeerMessageUtil::getIntParam(data, 5));
  cancelMessage->setLength(PeerMessageUtil::getIntParam(data, 9));
  return cancelMessage;
}

void CancelMessage::receivedAction() {
  peerInteraction->rejectPieceMessageInQueue(index, begin, length);
}

const char* CancelMessage::getMessage() {
  if(!inProgress) {
    /**
     * len --- 13, 4bytes
     * id --- 8, 1byte
     * index --- index, 4bytes
     * begin --- begin, 4bytes
     * length -- length, 4bytes
     * total: 17bytes
     */
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
    PeerMessageUtil::setIntParam(&msg[9], begin);
    PeerMessageUtil::setIntParam(&msg[13], length);
  }
  return msg;
}

int CancelMessage::getMessageLength() {
  return sizeof(msg);
}

void CancelMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
  PeerMessageUtil::checkBegin(begin, pieceLength);
  PeerMessageUtil::checkLength(length);
  PeerMessageUtil::checkRange(begin, length, pieceLength);
}

string CancelMessage::toString() const {
  return "cancel index="+Util::itos(index)+", begin="+Util::itos(begin)+
    ", length="+Util::itos(length);
}
