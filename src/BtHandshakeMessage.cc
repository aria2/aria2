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
#include "a2functional.h"

namespace aria2 {

const unsigned char BtHandshakeMessage::BT_PSTR[] = "BitTorrent protocol";
const char BtHandshakeMessage::NAME[] = "handshake";

BtHandshakeMessage::BtHandshakeMessage() : SimpleBtMessage(ID, NAME) { init(); }

BtHandshakeMessage::BtHandshakeMessage(const unsigned char* infoHash,
                                       const unsigned char* peerId)
    : SimpleBtMessage(ID, NAME)
{
  init();
  std::copy_n(infoHash, infoHash_.size(), std::begin(infoHash_));
  std::copy_n(peerId, peerId_.size(), std::begin(peerId_));
}

BtHandshakeMessage::~BtHandshakeMessage() = default;

void BtHandshakeMessage::init()
{
  pstrlen_ = 19;
  std::copy_n(BT_PSTR, pstr_.size(), std::begin(pstr_));
  std::fill(std::begin(reserved_), std::end(reserved_), 0);
  // fast extension
  reserved_[7] |= 0x04u;
  // extended messaging
  reserved_[5] |= 0x10u;
}

std::unique_ptr<BtHandshakeMessage>
BtHandshakeMessage::create(const unsigned char* data, size_t dataLength)
{
  auto msg = make_unique<BtHandshakeMessage>();
  msg->pstrlen_ = data[0];
  std::copy_n(&data[1], msg->pstr_.size(), std::begin(msg->pstr_));
  std::copy_n(&data[20], msg->reserved_.size(), std::begin(msg->reserved_));
  std::copy_n(&data[28], msg->infoHash_.size(), std::begin(msg->infoHash_));
  std::copy_n(&data[48], msg->peerId_.size(), std::begin(msg->peerId_));

  return msg;
}

std::vector<unsigned char> BtHandshakeMessage::createMessage()
{
  auto msg = std::vector<unsigned char>(MESSAGE_LENGTH);
  auto dst = msg.data();
  *dst++ = pstrlen_;
  dst = std::copy(std::begin(pstr_), std::end(pstr_), dst);
  dst = std::copy(std::begin(reserved_), std::end(reserved_), dst);
  dst = std::copy(std::begin(infoHash_), std::end(infoHash_), dst);
  std::copy(std::begin(peerId_), std::end(peerId_), dst);
  return msg;
}

std::string BtHandshakeMessage::toString() const
{
  return fmt("%s peerId=%s, reserved=%s", NAME,
             util::percentEncode(peerId_.data(), peerId_.size()).c_str(),
             util::toHex(reserved_.data(), reserved_.size()).c_str());
}

bool BtHandshakeMessage::isFastExtensionSupported() const
{
  return reserved_[7] & 0x04u;
}

bool BtHandshakeMessage::isExtendedMessagingEnabled() const
{
  return reserved_[5] & 0x10u;
}

bool BtHandshakeMessage::isDHTEnabled() const { return reserved_[7] & 0x01u; }

void BtHandshakeMessage::setInfoHash(const unsigned char* infoHash)
{
  std::copy_n(infoHash, infoHash_.size(), std::begin(infoHash_));
}

void BtHandshakeMessage::setPeerId(const unsigned char* peerId)
{
  std::copy_n(peerId, peerId_.size(), std::begin(peerId_));
}

} // namespace aria2
