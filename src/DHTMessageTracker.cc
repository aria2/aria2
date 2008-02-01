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
#include "DHTMessageTracker.h"
#include "DHTMessage.h"
#include "DHTMessageCallback.h"
#include "DHTMessageTrackerEntry.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "Util.h"
#include "LogFactory.h"
#include "Dictionary.h"
#include "Data.h"
#include "DlAbortEx.h"
#include "DHTConstants.h"

DHTMessageTracker::DHTMessageTracker():
  _routingTable(0),
  _factory(0),
  _logger(LogFactory::getInstance()) {}

DHTMessageTracker::~DHTMessageTracker() {}

void DHTMessageTracker::addMessage(const DHTMessageHandle& message, time_t timeout, const DHTMessageCallbackHandle& callback)
{
  _entries.push_back(new DHTMessageTrackerEntry(message, timeout, callback));
}

void DHTMessageTracker::addMessage(const DHTMessageHandle& message, const DHTMessageCallbackHandle& callback)
{
  addMessage(message, DHT_MESSAGE_TIMEOUT, callback);
}

pair<DHTMessageHandle, DHTMessageCallbackHandle>
DHTMessageTracker::messageArrived(const Dictionary* d,
				  const string& ipaddr, uint16_t port)
{
  const Data* tid = dynamic_cast<const Data*>(d->get("t"));
  if(!tid) {
    throw new DlAbortEx("Malformed DHT message. From:%s:%u", ipaddr.c_str(), port);
  }
  _logger->debug("Searching tracker entry for TransactionID=%s, Remote=%s:%u",
		 Util::toHex(tid->toString()).c_str(), ipaddr.c_str(), port);
  for(DHTMessageTrackerEntries::iterator i = _entries.begin();
      i != _entries.end(); ++i) {
    if((*i)->match(tid->toString(), ipaddr, port)) {
      DHTMessageTrackerEntryHandle entry = *i;
      _entries.erase(i);
      _logger->debug("Tracker entry found.");
      DHTNodeHandle targetNode = entry->getTargetNode();

      DHTMessageHandle message = _factory->createResponseMessage(entry->getMessageType(),
								 d, targetNode);
      int64_t rtt = entry->getElapsedMillis();
      _logger->debug("RTT is %s", Util::llitos(rtt).c_str());
      targetNode->updateRTT(rtt);
      DHTMessageCallbackHandle callback = entry->getCallback();
      return pair<DHTMessageHandle, DHTMessageCallbackHandle>(message, callback);
    }
  }
  _logger->debug("Tracker entry not found.");
  return pair<DHTMessageHandle, DHTMessageCallbackHandle>(0, 0);
}

void DHTMessageTracker::handleTimeout()
{
  for(DHTMessageTrackerEntries::iterator i = _entries.begin();
      i != _entries.end();) {
    if((*i)->isTimeout()) {
      DHTMessageTrackerEntryHandle entry = *i;
      i = _entries.erase(i);
      DHTNodeHandle node = entry->getTargetNode();
      _logger->debug("Message timeout: To:%s:%u",
		     node->getIPAddress().c_str(), node->getPort());
      node->updateRTT(entry->getElapsedMillis());
      node->timeout();
      if(node->isBad()) {
	_logger->debug("Marked bad: %s:%u",
		       node->getIPAddress().c_str(), node->getPort());
	_routingTable->dropNode(node);
      }
      DHTMessageCallbackHandle callback = entry->getCallback();
      if(!callback.isNull()) {
	callback->onTimeout(node);
      }
    } else {
      ++i;
    }
  }
}

DHTMessageTrackerEntryHandle
DHTMessageTracker::getEntryFor(const DHTMessageHandle& message) const
{
  for(DHTMessageTrackerEntries::const_iterator i = _entries.begin();
      i != _entries.end(); ++i) {
    if((*i)->match(message->getTransactionID(), message->getRemoteNode()->getIPAddress(), message->getRemoteNode()->getPort())) {
      return *i;
    }
  }
  return 0;
}

size_t DHTMessageTracker::countEntry() const
{
  return _entries.size();
}

void DHTMessageTracker::setRoutingTable(const DHTRoutingTableHandle& routingTable)
{
  _routingTable = routingTable;
}

void DHTMessageTracker::setMessageFactory(const DHTMessageFactoryHandle& factory)
{
  _factory = factory;
}
