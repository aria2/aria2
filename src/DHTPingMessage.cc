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
#include "DHTPingMessage.h"
#include "DHTNode.h"
#include "DHTConstants.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageFactory.h"
#include "DHTMessageCallback.h"

namespace aria2 {

const std::string DHTPingMessage::PING("ping");

DHTPingMessage::DHTPingMessage(const SharedHandle<DHTNode>& localNode,
                               const SharedHandle<DHTNode>& remoteNode,
                               const std::string& transactionID):
  DHTQueryMessage(localNode, remoteNode, transactionID) {}

DHTPingMessage::~DHTPingMessage() {}

void DHTPingMessage::doReceivedAction()
{
  // send back ping reply
  SharedHandle<DHTMessage> reply =
    getMessageFactory()->createPingReplyMessage
    (getRemoteNode(), getLocalNode()->getID(), getTransactionID());
  getMessageDispatcher()->addMessageToQueue(reply);
}

SharedHandle<Dict> DHTPingMessage::getArgument()
{
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  return aDict;
}

const std::string& DHTPingMessage::getMessageType() const
{
  return PING;
}

} // namespace aria2
