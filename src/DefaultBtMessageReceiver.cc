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
#include "DefaultBtMessageReceiver.h"
#include "BtHandshakeMessage.h"

BtMessageHandle DefaultBtMessageReceiver::receiveHandshake(bool quickReply) {
  unsigned char data[BtHandshakeMessage::MESSAGE_LENGTH];
  uint32_t dataLength = sizeof(data);
  bool retval = peerConnection->receiveHandshake(data, dataLength);
  // To handle tracker's NAT-checking feature
  if(!handshakeSent && quickReply && dataLength >= 48) {
    handshakeSent = true;
    // check info_hash
    if(memcmp(btContext->getInfoHash(), &data[28], INFO_HASH_LENGTH) == 0) {
      sendHandshake();
    }
  }
  if(!retval) {
    return 0;
  }
  BtHandshakeMessageHandle msg =
    BT_MESSAGE_FACTORY(btContext, peer)->createHandshakeMessage(data, dataLength);
  Errors errors;
  if(msg->validate(errors)) {
    if(msg->isFastExtensionSupported()) {
      peer->setFastExtensionEnabled(true);
      logger->info("CUID#%d - Fast extension enabled.", cuid);
    }
  } else {
    // TODO throw exception here based on errors
  }
  return msg;
}

BtMessageHandle DefaultBtMessageReceiver::receiveAndSendHandshake() {
  return receiveHandshake(true);
}

void DefaultBtMessageReceiver::sendHandshake() {
  BtHandshakeMessageHandle msg =
    BT_MESSAGE_FACTORY(btContext, peer)->createHandshakeMessage(btContext->getInfoHash(),
								 btContext->getPeerId());
  dispatcher->addMessageToQueue(msg);
  dispatcher->sendMessages();
}

BtMessageHandle DefaultBtMessageReceiver::receiveMessage() {
  unsigned char data[MAX_PAYLOAD_LEN];
  uint32_t dataLength = 0;
  if(!peerConnection->receiveMessage(data, dataLength)) {
    return 0;
  }
  BtMessageHandle msg =
    BT_MESSAGE_FACTORY(btContext, peer)->createBtMessage(data, dataLength);
  Errors errors;
  if(msg->validate(errors)) {
    return msg;
  } else {
    // TODO throw exception here based on errors;
    return 0;
  }
}

