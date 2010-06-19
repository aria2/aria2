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
#include "DHTMessageReceiver.h"

#include <cstring>
#include <utility>

#include "DHTMessageTracker.h"
#include "DHTConnection.h"
#include "DHTMessage.h"
#include "DHTQueryMessage.h"
#include "DHTResponseMessage.h"
#include "DHTUnknownMessage.h"
#include "DHTMessageFactory.h"
#include "DHTRoutingTable.h"
#include "DHTNode.h"
#include "DHTMessageCallback.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "bencode2.h"

namespace aria2 {

DHTMessageReceiver::DHTMessageReceiver(const SharedHandle<DHTMessageTracker>& tracker):
  _tracker(tracker),
  _logger(LogFactory::getInstance())
{}

DHTMessageReceiver::~DHTMessageReceiver() {}

SharedHandle<DHTMessage> DHTMessageReceiver::receiveMessage()
{
  std::string remoteAddr;
  uint16_t remotePort;
  unsigned char data[64*1024];
  try {
    ssize_t length = _connection->receiveMessage(data, sizeof(data),
                                                 remoteAddr,
                                                 remotePort);
    if(length <= 0) {
      return SharedHandle<DHTMessage>();
    }
    bool isReply = false;
    SharedHandle<ValueBase> decoded = bencode2::decode(data, length);
    const Dict* dict = asDict(decoded);
    if(dict) {
      const String* y = asString(dict->get(DHTMessage::Y));
      if(y) {
        if(y->s() == DHTResponseMessage::R || y->s() == DHTUnknownMessage::E) {
          isReply = true;
        }
      } else {
        _logger->info("Malformed DHT message. Missing 'y' key. From:%s:%u",
                      remoteAddr.c_str(), remotePort);
        return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
      }
    } else {
      _logger->info("Malformed DHT message. This is not a bencoded directory."
                    " From:%s:%u", remoteAddr.c_str(), remotePort);
      return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
    }
    if(isReply) {
      std::pair<SharedHandle<DHTResponseMessage>,
                SharedHandle<DHTMessageCallback> > p =
        _tracker->messageArrived(dict, remoteAddr, remotePort);
      if(p.first.isNull()) {
        // timeout or malicious? message
        return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
      }
      onMessageReceived(p.first);
      if(!p.second.isNull()) {
        p.second->onReceived(p.first);
      }
      return p.first;
    } else {
      SharedHandle<DHTQueryMessage> message =
        _factory->createQueryMessage(dict, remoteAddr, remotePort);
      if(message->getLocalNode() == message->getRemoteNode()) {
        // drop message from localnode
        _logger->info("Received DHT message from localnode.");
        return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
      }
      onMessageReceived(message);
      return message;
    }
  } catch(RecoverableException& e) {
    _logger->info("Exception thrown while receiving DHT message.", e);
    return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
  }
}

void DHTMessageReceiver::onMessageReceived
(const SharedHandle<DHTMessage>& message)
{
  if(_logger->info()) {
    _logger->info("Message received: %s", message->toString().c_str());
  }
  message->validate();
  message->doReceivedAction();
  message->getRemoteNode()->markGood();
  message->getRemoteNode()->updateLastContact();
  _routingTable->addGoodNode(message->getRemoteNode());
}

void DHTMessageReceiver::handleTimeout()
{
  _tracker->handleTimeout();
}

SharedHandle<DHTMessage>
DHTMessageReceiver::handleUnknownMessage(const unsigned char* data,
                                         size_t length,
                                         const std::string& remoteAddr,
                                         uint16_t remotePort)
{
  SharedHandle<DHTMessage> m =
    _factory->createUnknownMessage(data, length, remoteAddr, remotePort);
  if(_logger->info()) {
    _logger->info("Message received: %s", m->toString().c_str());
  }
  return m;
}

void DHTMessageReceiver::setConnection(const SharedHandle<DHTConnection>& connection)
{
  _connection = connection;
}

void DHTMessageReceiver::setMessageFactory(const SharedHandle<DHTMessageFactory>& factory)
{
  _factory = factory;
}

void DHTMessageReceiver::setRoutingTable(const SharedHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

} // namespace aria2
