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
#include "DHTMessageReceiver.h"
#include "DHTMessageTracker.h"
#include "DHTConnection.h"
#include "DHTMessage.h"
#include "DHTMessageFactory.h"
#include "DHTRoutingTable.h"
#include "DHTNode.h"
#include "DHTMessageCallback.h"
#include "MetaEntry.h"
#include "MetaFileUtil.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Util.h"

DHTMessageReceiver::DHTMessageReceiver(const DHTMessageTrackerHandle& tracker):
  _tracker(tracker),
  _connection(0),
  _factory(0),
  _routingTable(0),
  _logger(LogFactory::getInstance())
{}

DHTMessageReceiver::~DHTMessageReceiver() {}

DHTMessageHandle DHTMessageReceiver::receiveMessage()
{
  string remoteAddr;
  uint16_t remotePort;
  char data[64*1024];
  ssize_t length = _connection->receiveMessage(data, sizeof(data),
					       remoteAddr,
					       remotePort);
  if(length <= 0) {
    return 0;
  }
  try {
    bool isReply = false;
    MetaEntryHandle msgroot = MetaFileUtil::bdecoding(data, length);
    const Dictionary* d = dynamic_cast<const Dictionary*>(msgroot.get());
    if(d) {
      const Data* y = dynamic_cast<const Data*>(d->get("y"));
      if(y->toString() == "r" || y->toString() == "e") {
	isReply = true;
      }
    } else {
      _logger->info("Malformed DHT message. This is not a bencoded directory. From:%s:%u", remoteAddr.c_str(), remotePort);
      return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
    }
    DHTMessageHandle message = 0;
    DHTMessageCallbackHandle callback = 0;
    if(isReply) {
      std::pair<DHTMessageHandle, DHTMessageCallbackHandle> p = _tracker->messageArrived(d, remoteAddr, remotePort);
      message = p.first;
      callback = p.second;
      if(message.isNull()) {
	// timeout or malicious? message
	return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
      }
    } else {
      message = _factory->createQueryMessage(d, remoteAddr, remotePort);
    }
    _logger->info("Message received: %s", message->toString().c_str());
    message->validate();
    message->doReceivedAction();
    message->getRemoteNode()->markGood();
    message->getRemoteNode()->updateLastContact();
    _routingTable->addGoodNode(message->getRemoteNode());
    if(!callback.isNull()) {
      callback->onReceived(message);
    }
    return message;
  } catch(RecoverableException* e) {
    _logger->info("Exception thrown while receiving DHT message.", e);
    delete e;
    return handleUnknownMessage(data, sizeof(data), remoteAddr, remotePort);
  }
}

void DHTMessageReceiver::handleTimeout()
{
  _tracker->handleTimeout();
}

SharedHandle<DHTMessage>
DHTMessageReceiver::handleUnknownMessage(const char* data, size_t length,
					 const string& remoteAddr,
					 uint16_t remotePort)
{
  SharedHandle<DHTMessage> m =
    _factory->createUnknownMessage(data, length, remoteAddr, remotePort);
  _logger->info("Message received: %s", m->toString().c_str());
  return m;
}

DHTConnectionHandle DHTMessageReceiver::getConnection() const
{
  return _connection;
}

DHTMessageTrackerHandle DHTMessageReceiver::getMessageTracker() const
{
  return _tracker;
}

void DHTMessageReceiver::setConnection(const DHTConnectionHandle& connection)
{
  _connection = connection;
}

void DHTMessageReceiver::setMessageFactory(const DHTMessageFactoryHandle& factory)
{
  _factory = factory;
}

void DHTMessageReceiver::setRoutingTable(const DHTRoutingTableHandle& routingTable)
{
  _routingTable = routingTable;
}
