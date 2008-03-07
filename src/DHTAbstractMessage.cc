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
#include "DHTAbstractMessage.h"
#include "DHTNode.h"
#include "BencodeVisitor.h"
#include "DHTConnection.h"
#include "Dictionary.h"
#include "Data.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageFactory.h"
#include "DHTRoutingTable.h"

namespace aria2 {

DHTAbstractMessage::DHTAbstractMessage(const SharedHandle<DHTNode>& localNode,
				       const SharedHandle<DHTNode>& remoteNode,
				       const std::string& transactionID):
  DHTMessage(localNode, remoteNode, transactionID) {}

DHTAbstractMessage::~DHTAbstractMessage() {}

std::string DHTAbstractMessage::getBencodedMessage()
{
  SharedHandle<Dictionary> msg = new Dictionary();
  msg->put(std::string("t"), new Data(_transactionID));
  msg->put(std::string("y"), new Data(getType()));
  fillMessage(msg.get());
  
  BencodeVisitor v;
  msg->accept(&v);
  return v.getBencodedData();
}

void DHTAbstractMessage::send()
{
  std::string message = getBencodedMessage();
  _connection->sendMessage(reinterpret_cast<const unsigned char*>(message.c_str()),
			   message.size(),
			   _remoteNode->getIPAddress(),
			   _remoteNode->getPort());
}

void DHTAbstractMessage::setConnection(const WeakHandle<DHTConnection>& connection)
{
  _connection = connection;
}

void DHTAbstractMessage::setMessageDispatcher(const WeakHandle<DHTMessageDispatcher>& dispatcher)
{
  _dispatcher = dispatcher;
}

void DHTAbstractMessage::setMessageFactory(const WeakHandle<DHTMessageFactory>& factory)
{
  _factory = factory;
}

void DHTAbstractMessage::setRoutingTable(const WeakHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

} // namespace aria2
