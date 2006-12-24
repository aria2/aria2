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
#include "BtChokeMessage.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"

BtChokeMessageHandle BtChokeMessage::create(const unsigned char* data, uint32_t dataLength) {
  if(dataLength != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "choke", dataLength, 1);
  }
  int32_t id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "choke", ID);
  }
  BtChokeMessageHandle chokeMessage = new BtChokeMessage();
  return chokeMessage;
}

void BtChokeMessage::doReceivedAction() {
  peer->peerChoking = true;
  BT_MESSAGE_DISPATCHER(btContext, peer)->doChokedAction();
}

bool BtChokeMessage::sendPredicate() const {
  return !peer->amChoking;
}

uint32_t BtChokeMessage::MESSAGE_LENGTH = 5;

const char* BtChokeMessage::getMessage() {
  if(!msg) {
    /**
     * len --- 1, 4bytes
     * id --- 0, 1byte
     * total: 5bytes
     */
    msg = new char[MESSAGE_LENGTH];
    PeerMessageUtil::createPeerMessageString(msg, MESSAGE_LENGTH, 1, ID);
  }
  return msg;
}

uint32_t BtChokeMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

void BtChokeMessage::onSendComplete() {
  peer->amChoking = true;
  BT_MESSAGE_DISPATCHER(btContext, peer)->doChokingAction();
}

string BtChokeMessage::toString() const {
  return "choke";
}
