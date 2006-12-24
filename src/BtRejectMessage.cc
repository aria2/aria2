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
#include "BtRejectMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"

BtRejectMessageHandle BtRejectMessage::create(const unsigned char* data, uint32_t dataLength) {
  if(dataLength != 13) {
    throw new DlAbortEx("invalid payload size for %s, size = %u. It should be %d", "reject", dataLength, 13);
  }
  uint8_t id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%u for %s. It should be %d.",
			id, "reject", ID);
  }
  BtRejectMessageHandle message = new BtRejectMessage();
  message->setIndex(PeerMessageUtil::getIntParam(data, 1));
  message->setBegin(PeerMessageUtil::getIntParam(data, 5));
  message->setLength(PeerMessageUtil::getIntParam(data, 9));
  return message;
}

void BtRejectMessage::doReceivedAction() {
  if(!peer->isFastExtensionEnabled()) {
    throw new DlAbortEx("%s received while fast extension is disabled.",
			toString().c_str());
  }
  // TODO Current implementation does not close a connection even if
  // a request for this reject message has never sent.
  RequestSlot slot =
    BT_MESSAGE_DISPATCHER(btContext, peer)->getOutstandingRequest(index, begin, length);
  if(RequestSlot::isNull(slot)) {
    //throw new DlAbortEx("reject recieved, but it is not in the request slots.");
  } else {
    BT_MESSAGE_DISPATCHER(btContext, peer)->removeOutstandingRequest(slot);
  }

}

uint32_t BtRejectMessage::MESSAGE_LENGTH = 17;

const unsigned char* BtRejectMessage::getMessage() {
  if(!msg) {
    /**
     * len --- 13, 4bytes
     * id --- 16, 1byte
     * index --- index, 4bytes
     * begin --- begin, 4bytes
     * length -- length, 4bytes
     * total: 17bytes
     */
    msg = new unsigned char[MESSAGE_LENGTH];
    PeerMessageUtil::createPeerMessageString(msg, MESSAGE_LENGTH, 13, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
    PeerMessageUtil::setIntParam(&msg[9], begin);
    PeerMessageUtil::setIntParam(&msg[13], length);
  }
  return msg;
}

uint32_t BtRejectMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

string BtRejectMessage::toString() const {
  return "reject index="+Util::itos(index)+", begin="+Util::itos(begin)+
    ", length="+Util::uitos(length);
}
