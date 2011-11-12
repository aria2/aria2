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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <cstring>

#include "util.h"
#include "BtConstants.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtHandshakeMessage::NAME("handshake");

const unsigned char* BtHandshakeMessage::BT_PSTR =
  reinterpret_cast<const unsigned char*>("BitTorrent protocol");

BtHandshakeMessage::BtHandshakeMessage():SimpleBtMessage(ID, NAME)
{
  init();
}

BtHandshakeMessage::BtHandshakeMessage(const unsigned char* infoHash,
                                       const unsigned char* peerId):
  SimpleBtMessage(ID, NAME)
{
  init();
  memcpy(infoHash_, infoHash, INFO_HASH_LENGTH);
  memcpy(peerId_, peerId, PEER_ID_LENGTH);
}

void BtHandshakeMessage::init() {
  pstrlen_ = 19;
  pstr_ = new unsigned char[PSTR_LENGTH];
  reserved_ = new unsigned char[RESERVED_LENGTH];
  infoHash_ = new unsigned char[INFO_HASH_LENGTH];
  peerId_ = new unsigned char[PEER_ID_LENGTH];
  memcpy(pstr_, BT_PSTR, PSTR_LENGTH);
  memset(reserved_, 0, RESERVED_LENGTH);
  // fast extension
  reserved_[7] |= 0x04u;
  // extended messaging
  reserved_[5] |= 0x10u;
}

SharedHandle<BtHandshakeMessage>
BtHandshakeMessage::create(const unsigned char* data, size_t dataLength)
{
  SharedHandle<BtHandshakeMessage> message(new BtHandshakeMessage());
  message->pstrlen_ = data[0];
  memcpy(message->pstr_, &data[1], PSTR_LENGTH);
  memcpy(message->reserved_, &data[20], RESERVED_LENGTH);
  memcpy(message->infoHash_, &data[28], INFO_HASH_LENGTH);
  memcpy(message->peerId_, &data[48], PEER_ID_LENGTH);
  return message;
}

unsigned char* BtHandshakeMessage::createMessage()
{
  unsigned char* msg = new unsigned char[MESSAGE_LENGTH];
  msg[0] = pstrlen_;
  memcpy(msg+1, pstr_, PSTR_LENGTH);
  memcpy(msg+20, reserved_, RESERVED_LENGTH);
  memcpy(msg+28, infoHash_, INFO_HASH_LENGTH);
  memcpy(msg+48, peerId_, PEER_ID_LENGTH);
  return msg;
}

size_t BtHandshakeMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

std::string BtHandshakeMessage::toString() const {
  return fmt("%s peerId=%s, reserved=%s",
             NAME.c_str(),
             util::percentEncode(peerId_, PEER_ID_LENGTH).c_str(),
             util::toHex(reserved_, RESERVED_LENGTH).c_str());
}

bool BtHandshakeMessage::isFastExtensionSupported() const {
  return reserved_[7]&0x04u;
}

bool BtHandshakeMessage::isExtendedMessagingEnabled() const
{
  return reserved_[5]&0x10u;
}

bool BtHandshakeMessage::isDHTEnabled() const
{
  return reserved_[7]&0x01u;
}

void BtHandshakeMessage::setInfoHash(const unsigned char* infoHash)
{
  memcpy(infoHash_, infoHash, INFO_HASH_LENGTH);
}

void BtHandshakeMessage::setPeerId(const unsigned char* peerId)
{
  memcpy(peerId_, peerId, PEER_ID_LENGTH);
}

} // namespace aria2
