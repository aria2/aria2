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
#include "DHTAbstractMessage.h"

#include <cassert>

#include "DHTNode.h"
#include "DHTConnection.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageFactory.h"
#include "DHTRoutingTable.h"
#include "DHTMessageCallback.h"
#include "bencode2.h"

namespace aria2 {

DHTAbstractMessage::DHTAbstractMessage(
    const std::shared_ptr<DHTNode>& localNode,
    const std::shared_ptr<DHTNode>& remoteNode,
    const std::string& transactionID)
    : DHTMessage{localNode, remoteNode, transactionID},
      connection_{nullptr},
      dispatcher_{nullptr},
      factory_{nullptr},
      routingTable_{nullptr}
{
}

std::string DHTAbstractMessage::getBencodedMessage()
{
  Dict msgDict;
  msgDict.put(T, getTransactionID());
  msgDict.put(Y, getType());
  msgDict.put(V, getVersion());
  fillMessage(&msgDict);
  return bencode2::encode(&msgDict);
}

bool DHTAbstractMessage::send()
{
  std::string message = getBencodedMessage();
  ssize_t r = connection_->sendMessage(
      reinterpret_cast<const unsigned char*>(message.c_str()), message.size(),
      getRemoteNode()->getIPAddress(), getRemoteNode()->getPort());
  assert(r >= 0);
  return r == static_cast<ssize_t>(message.size());
}

void DHTAbstractMessage::setConnection(DHTConnection* connection)
{
  connection_ = connection;
}

void DHTAbstractMessage::setMessageDispatcher(DHTMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void DHTAbstractMessage::setMessageFactory(DHTMessageFactory* factory)
{
  factory_ = factory;
}

void DHTAbstractMessage::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

} // namespace aria2
