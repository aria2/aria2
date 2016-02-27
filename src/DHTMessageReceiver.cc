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
#include "fmt.h"

namespace aria2 {

DHTMessageReceiver::DHTMessageReceiver(
    const std::shared_ptr<DHTMessageTracker>& tracker)
    : tracker_{tracker}, factory_{nullptr}, routingTable_{nullptr}
{
}

std::unique_ptr<DHTMessage>
DHTMessageReceiver::receiveMessage(const std::string& remoteAddr,
                                   uint16_t remotePort, unsigned char* data,
                                   size_t length)
{
  try {
    bool isReply = false;
    auto decoded = bencode2::decode(data, length);
    const Dict* dict = downcast<Dict>(decoded);
    if (dict) {
      const String* y = downcast<String>(dict->get(DHTMessage::Y));
      if (y) {
        if (y->s() == DHTResponseMessage::R || y->s() == DHTUnknownMessage::E) {
          isReply = true;
        }
      }
      else {
        A2_LOG_INFO(fmt("Malformed DHT message. Missing 'y' key. From:%s:%u",
                        remoteAddr.c_str(), remotePort));
        return handleUnknownMessage(data, length, remoteAddr, remotePort);
      }
    }
    else {
      A2_LOG_INFO(fmt("Malformed DHT message. This is not a bencoded directory."
                      " From:%s:%u",
                      remoteAddr.c_str(), remotePort));
      return handleUnknownMessage(data, length, remoteAddr, remotePort);
    }
    if (isReply) {
      auto p = tracker_->messageArrived(dict, remoteAddr, remotePort);
      if (!p.first) {
        // timeout or malicious? message
        return handleUnknownMessage(data, length, remoteAddr, remotePort);
      }
      onMessageReceived(p.first.get());
      if (p.second) {
        p.second->onReceived(p.first.get());
      }
      return std::move(p.first);
    }
    else {
      auto message = factory_->createQueryMessage(dict, remoteAddr, remotePort);
      if (*message->getLocalNode() == *message->getRemoteNode()) {
        // drop message from localnode
        A2_LOG_INFO("Received DHT message from localnode.");
        return handleUnknownMessage(data, length, remoteAddr, remotePort);
      }
      onMessageReceived(message.get());
      return std::move(message);
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX("Exception thrown while receiving DHT message.", e);
    return handleUnknownMessage(data, length, remoteAddr, remotePort);
  }
}

void DHTMessageReceiver::onMessageReceived(DHTMessage* message)
{
  A2_LOG_INFO(fmt("Message received: %s", message->toString().c_str()));
  message->validate();
  message->doReceivedAction();
  message->getRemoteNode()->markGood();
  message->getRemoteNode()->updateLastContact();
  routingTable_->addGoodNode(message->getRemoteNode());
}

void DHTMessageReceiver::handleTimeout() { tracker_->handleTimeout(); }

std::unique_ptr<DHTUnknownMessage> DHTMessageReceiver::handleUnknownMessage(
    const unsigned char* data, size_t length, const std::string& remoteAddr,
    uint16_t remotePort)
{
  auto m = factory_->createUnknownMessage(data, length, remoteAddr, remotePort);
  A2_LOG_INFO(fmt("Message received: %s", m->toString().c_str()));
  return m;
}

void DHTMessageReceiver::setMessageFactory(DHTMessageFactory* factory)
{
  factory_ = factory;
}

void DHTMessageReceiver::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

} // namespace aria2
