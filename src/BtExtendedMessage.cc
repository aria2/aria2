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
#include "BtRegistry.h"
#include "PeerObject.h"
#include "BtMessageFactory.h"
#include "BtMessageReceiver.h"
#include "BtMessageDispatcher.h"
#include "BtRequestFactory.h"
#include "PeerConnection.h"
#include "ExtensionMessage.h"
#include "ExtensionMessageFactory.h"
#include "PeerMessageUtil.h"
#include "Peer.h"
#include "BtContext.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Util.h"
#include <cassert>
#include <cstring>

namespace aria2 {

BtExtendedMessage::BtExtendedMessage(const ExtensionMessageHandle& extensionMessage):_extensionMessage(extensionMessage), _msg(0), _msgLength(0)
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
    PeerMessageUtil::createPeerMessageString(_msg, _msgLength, 2+payload.size(), ID);
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
  return "extended "+_extensionMessage->toString();
}

BtExtendedMessageHandle
BtExtendedMessage::create(const BtContextHandle& btContext,
			  const PeerHandle& peer,
			  const unsigned char* data, size_t dataLength)
{
  if(dataLength < 2) {
    throw new DlAbortEx(MSG_TOO_SMALL_PAYLOAD_SIZE, "extended", dataLength);
  }
  uint8_t id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx(EX_INVALID_BT_MESSAGE_ID, id, "extended", ID);
  }
  ExtensionMessageFactoryHandle factory = EXTENSION_MESSAGE_FACTORY(btContext,
								    peer);
  assert(!factory.isNull());
  ExtensionMessageHandle extmsg = factory->createMessage(data+1,
							 dataLength-1);
  BtExtendedMessageHandle message = new BtExtendedMessage(extmsg);
  return message;
}

void BtExtendedMessage::doReceivedAction()
{
  if(!_extensionMessage.isNull()) {
    _extensionMessage->doReceivedAction();
  }
}

ExtensionMessageHandle BtExtendedMessage::getExtensionMessage() const
{
  return _extensionMessage;
}

} // namespace aria2
