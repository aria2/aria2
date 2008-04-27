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
#include "BtInterestedMessage.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Peer.h"
#include "BtRegistry.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "StringFormat.h"

namespace aria2 {

BtInterestedMessageHandle BtInterestedMessage::create(const unsigned char* data, size_t dataLength) {
  if(dataLength != 1) {
    throw DlAbortEx
      (StringFormat(EX_INVALID_PAYLOAD_SIZE, "interested", dataLength, 1).str());
  }
  uint8_t id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw DlAbortEx
      (StringFormat(EX_INVALID_BT_MESSAGE_ID, id, "interested", ID).str());
  }
  BtInterestedMessageHandle message(new BtInterestedMessage());
  return message;
}

void BtInterestedMessage::doReceivedAction() {
  peer->peerInterested(true);
  if(!peer->amChoking()) {
    PEER_STORAGE(btContext)->executeChoke();
  }
}

bool BtInterestedMessage::sendPredicate() const {
  return !peer->amInterested();
}

size_t BtInterestedMessage::MESSAGE_LENGTH = 5;

const unsigned char* BtInterestedMessage::getMessage() {
  if(!msg) {
    /**
     * len --- 1, 4bytes
     * id --- 2, 1byte
     * total: 5bytes
     */
    msg = new unsigned char[MESSAGE_LENGTH];
    PeerMessageUtil::createPeerMessageString(msg, MESSAGE_LENGTH, 1, ID);
  }
  return msg;
}

size_t BtInterestedMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

void BtInterestedMessage::onSendComplete() {
  peer->amInterested(true);
}

std::string BtInterestedMessage::toString() const {
  return "interested";
}

} // namespace aria2
