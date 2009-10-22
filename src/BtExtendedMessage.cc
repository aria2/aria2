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
#include "StringFormat.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtExtendedMessage::NAME("extended");

BtExtendedMessage::BtExtendedMessage
(const ExtensionMessageHandle& extensionMessage):
  SimpleBtMessage(ID, NAME),
  _extensionMessage(extensionMessage),
  _msg(0),
  _msgLength(0)
{}

BtExtendedMessage::~BtExtendedMessage()
{
  delete [] _msg;
}

const unsigned char* BtExtendedMessage::getMessage() {
  if(!_msg) {
    /**
     * len --- 2+extpayload.length, 4bytes
     * id --- 20, 1byte
     * extmsgid --- extmsgid, 1byte
     * extpayload --- extpayload, nbytes
     * total: 6+extpayload.length bytes
     */
    std::string payload = _extensionMessage->getBencodedData();
    _msgLength = 6+payload.size();
    _msg = new unsigned char[_msgLength];
    bittorrent::createPeerMessageString(_msg, _msgLength, 2+payload.size(), ID);
    *(_msg+5) = _extensionMessage->getExtensionMessageID();
    memcpy(_msg+6, payload.c_str(), payload.size());
  }
  return _msg;
}

size_t BtExtendedMessage::getMessageLength() {
  getMessage();
  return _msgLength;
}

bool BtExtendedMessage::sendPredicate() const
{
  return peer->isExtendedMessagingEnabled();
}

std::string BtExtendedMessage::toString() const {
  return strconcat(NAME, " ", _extensionMessage->toString());
}

BtExtendedMessageHandle
BtExtendedMessage::create(const SharedHandle<ExtensionMessageFactory>& factory,
			  const PeerHandle& peer,
			  const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthGreater(1, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  assert(!factory.isNull());
  ExtensionMessageHandle extmsg = factory->createMessage(data+1,
							 dataLength-1);
  BtExtendedMessageHandle message(new BtExtendedMessage(extmsg));
  return message;
}

void BtExtendedMessage::doReceivedAction()
{
  if(!_extensionMessage.isNull()) {
    _extensionMessage->doReceivedAction();
  }
}

} // namespace aria2
