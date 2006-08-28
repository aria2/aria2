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
#include "HandshakeMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"

HandshakeMessage::HandshakeMessage() {
  init();
}

HandshakeMessage::HandshakeMessage(const unsigned char* infoHash,
				   const char* peerId)
{
  init();
  memcpy(this->infoHash, infoHash, INFO_HASH_LENGTH);
  memcpy(this->peerId, peerId, PEER_ID_LENGTH);
}

void HandshakeMessage::init() {
  this->pstrlen = 19;
  this->pstr = PSTR;
  memset(this->reserved, 0, sizeof(this->reserved));
  // fast extension
  this->reserved[7] |= 0x04;
}

HandshakeMessage* HandshakeMessage::create(const char* data, int dataLength) {
  HandshakeMessage* message = new HandshakeMessage();
  message->pstrlen = data[0];
  char pstrTemp[20];
  memcpy(pstrTemp, &data[1], sizeof(pstrTemp)-1);
  pstrTemp[sizeof(pstrTemp)-1] = '\0';
  message->pstr = pstrTemp;
  memcpy(message->reserved, &data[20], 8);
  memcpy(message->infoHash, &data[28], 20);
  memcpy(message->peerId, &data[48], 20);
  return message;
}

const char* HandshakeMessage::getMessage() {
  if(!inProgress) {
    msg[0] = pstrlen;
    memcpy(msg+1, pstr.c_str(), pstr.size());
    memcpy(msg+20, reserved, 8);
    memcpy(msg+28, infoHash, 20);
    memcpy(msg+48, peerId, 20);
  }
  return msg;
}

int HandshakeMessage::getMessageLength() {
  return sizeof(msg);
}

string HandshakeMessage::toString() const {
  return "handshake peerId="+
    Util::urlencode(peerId, sizeof(peerId))+
    " reserved="+Util::toHex(reserved, sizeof(reserved));
}

void HandshakeMessage::check() const {
  PeerMessageUtil::checkHandshake(this,
				  peerInteraction->getTorrentMan()->getInfoHash());
}

bool HandshakeMessage::isFastExtensionSupported() const {
  return reserved[7]&0x04;
}
