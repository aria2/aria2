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
#include "BtHandshakeMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"

const unsigned char* BtHandshakeMessage::BT_PSTR = (const unsigned char*)"BitTorrent protocol";

BtHandshakeMessage::BtHandshakeMessage() {
  init();
}

BtHandshakeMessage::BtHandshakeMessage(const unsigned char* infoHash,
				       const unsigned char* peerId)
{
  init();
  memcpy(this->infoHash, infoHash, INFO_HASH_LENGTH);
  memcpy(this->peerId, peerId, PEER_ID_LENGTH);
}

void BtHandshakeMessage::init() {
  msg = 0;
  this->pstrlen = 19;
  pstr = new unsigned char[PSTR_LENGTH];
  reserved = new unsigned char[RESERVED_LENGTH];
  infoHash = new unsigned char[INFO_HASH_LENGTH];
  peerId = new unsigned char[PEER_ID_LENGTH];
  memcpy(this->pstr, BT_PSTR, PSTR_LENGTH);
  memset(this->reserved, 0, RESERVED_LENGTH);
  // fast extension
  this->reserved[7] |= 0x04;
}

BtHandshakeMessageHandle BtHandshakeMessage::create(const unsigned char* data, int32_t dataLength) {
  BtHandshakeMessageHandle message = new BtHandshakeMessage();
  message->pstrlen = data[0];
  memcpy(message->pstr, &data[1], PSTR_LENGTH);
  memcpy(message->reserved, &data[20], RESERVED_LENGTH);
  memcpy(message->infoHash, &data[28], INFO_HASH_LENGTH);
  memcpy(message->peerId, &data[48], PEER_ID_LENGTH);
  return message;
}

const unsigned char* BtHandshakeMessage::getMessage() {
  if(!msg) {
    msg = new unsigned char[MESSAGE_LENGTH];
    msg[0] = pstrlen;
    memcpy(msg+1, pstr, PSTR_LENGTH);
    memcpy(msg+20, reserved, RESERVED_LENGTH);
    memcpy(msg+28, infoHash, INFO_HASH_LENGTH);
    memcpy(msg+48, peerId, PEER_ID_LENGTH);
  }
  return msg;
}

int32_t BtHandshakeMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

string BtHandshakeMessage::toString() const {
  return "handshake peerId="+
    Util::urlencode(peerId, PEER_ID_LENGTH)+
    ", reserved="+Util::toHex(reserved, RESERVED_LENGTH);
}

bool BtHandshakeMessage::isFastExtensionSupported() const {
  return reserved[7]&0x04;
}
