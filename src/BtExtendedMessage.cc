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
#include "BtExtendedMessage.h"

#include <cassert>
#include <cstring>

#include "ExtensionMessage.h"
#include "ExtensionMessageFactory.h"
#include "bittorrent_helper.h"
#include "Peer.h"
#include "DlAbortEx.h"
#include "message.h"
#include "util.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtExtendedMessage::NAME("extended");

BtExtendedMessage::BtExtendedMessage
(const ExtensionMessageHandle& extensionMessage):
  SimpleBtMessage(ID, NAME),
  extensionMessage_(extensionMessage),
  msgLength_(0)
{}

BtExtendedMessage::~BtExtendedMessage() {}

unsigned char* BtExtendedMessage::createMessage()
{
  /**
   * len --- 2+extpayload.length, 4bytes
   * id --- 20, 1byte
   * extmsgid --- extmsgid, 1byte
   * extpayload --- extpayload, nbytes
   * total: 6+extpayload.length bytes
   */
  std::string payload = extensionMessage_->getPayload();
  msgLength_ = 6+payload.size();
  unsigned char* msg = new unsigned char[msgLength_];
  bittorrent::createPeerMessageString(msg, msgLength_, 2+payload.size(), ID);
  *(msg+5) = extensionMessage_->getExtensionMessageID();
  memcpy(msg+6, payload.data(), payload.size());
  return msg;
}

size_t BtExtendedMessage::getMessageLength() {
  if(!msgLength_) {
    msgLength_ = 6+extensionMessage_->getPayload().size();
  }
  return msgLength_;
}

bool BtExtendedMessage::sendPredicate() const
{
  return getPeer()->isExtendedMessagingEnabled();
}

std::string BtExtendedMessage::toString() const {
  std::string s = NAME;
  s += " ";
  s += extensionMessage_->toString();
  return s;
}

BtExtendedMessageHandle
BtExtendedMessage::create(const SharedHandle<ExtensionMessageFactory>& factory,
                          const SharedHandle<Peer>& peer,
                          const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthGreater(1, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  assert(factory);
  ExtensionMessageHandle extmsg = factory->createMessage(data+1,
                                                         dataLength-1);
  BtExtendedMessageHandle message(new BtExtendedMessage(extmsg));
  return message;
}

void BtExtendedMessage::doReceivedAction()
{
  if(extensionMessage_) {
    extensionMessage_->doReceivedAction();
  }
}

} // namespace aria2
