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
  memcpy(_infoHash, infoHash, INFO_HASH_LENGTH);
  memcpy(_peerId, peerId, PEER_ID_LENGTH);
}

void BtHandshakeMessage::init() {
  _pstrlen = 19;
  _pstr = new unsigned char[PSTR_LENGTH];
  _reserved = new unsigned char[RESERVED_LENGTH];
  _infoHash = new unsigned char[INFO_HASH_LENGTH];
  _peerId = new unsigned char[PEER_ID_LENGTH];
  memcpy(_pstr, BT_PSTR, PSTR_LENGTH);
  memset(_reserved, 0, RESERVED_LENGTH);
  // fast extension
  _reserved[7] |= 0x04;
  // extended messaging
  _reserved[5] |= 0x10;
}

SharedHandle<BtHandshakeMessage>
BtHandshakeMessage::create(const unsigned char* data, size_t dataLength)
{
  SharedHandle<BtHandshakeMessage> message(new BtHandshakeMessage());
  message->_pstrlen = data[0];
  memcpy(message->_pstr, &data[1], PSTR_LENGTH);
  memcpy(message->_reserved, &data[20], RESERVED_LENGTH);
  memcpy(message->_infoHash, &data[28], INFO_HASH_LENGTH);
  memcpy(message->_peerId, &data[48], PEER_ID_LENGTH);
  return message;
}

unsigned char* BtHandshakeMessage::createMessage()
{
  unsigned char* msg = new unsigned char[MESSAGE_LENGTH];
  msg[0] = _pstrlen;
  memcpy(msg+1, _pstr, PSTR_LENGTH);
  memcpy(msg+20, _reserved, RESERVED_LENGTH);
  memcpy(msg+28, _infoHash, INFO_HASH_LENGTH);
  memcpy(msg+48, _peerId, PEER_ID_LENGTH);
  return msg;
}

size_t BtHandshakeMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

std::string BtHandshakeMessage::toString() const {
  return strconcat(NAME, " peerId=",
                   util::percentEncode(_peerId, PEER_ID_LENGTH),
                   ", reserved=",
                   util::toHex(_reserved, RESERVED_LENGTH));
}

bool BtHandshakeMessage::isFastExtensionSupported() const {
  return _reserved[7]&0x04;
}

bool BtHandshakeMessage::isExtendedMessagingEnabled() const
{
  return _reserved[5]&0x10;
}

bool BtHandshakeMessage::isDHTEnabled() const
{
  return _reserved[7]&0x01;
}

void BtHandshakeMessage::setInfoHash(const unsigned char* infoHash)
{
  memcpy(_infoHash, infoHash, INFO_HASH_LENGTH);
}

void BtHandshakeMessage::setPeerId(const unsigned char* peerId)
{
  memcpy(_peerId, peerId, PEER_ID_LENGTH);
}

} // namespace aria2
