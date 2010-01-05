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
#include "DHTMessageTracker.h"

#include <utility>

#include "DHTMessage.h"
#include "DHTMessageCallback.h"
#include "DHTMessageTrackerEntry.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DlAbortEx.h"
#include "DHTConstants.h"
#include "StringFormat.h"
#include "bencode.h"

namespace aria2 {

DHTMessageTracker::DHTMessageTracker():
  _logger(LogFactory::getInstance()) {}

DHTMessageTracker::~DHTMessageTracker() {}

void DHTMessageTracker::addMessage(const SharedHandle<DHTMessage>& message, time_t timeout, const SharedHandle<DHTMessageCallback>& callback)
{
  SharedHandle<DHTMessageTrackerEntry> e(new DHTMessageTrackerEntry(message, timeout, callback));
  _entries.push_back(e);
}

void DHTMessageTracker::addMessage(const SharedHandle<DHTMessage>& message, const SharedHandle<DHTMessageCallback>& callback)
{
  addMessage(message, DHT_MESSAGE_TIMEOUT, callback);
}

std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> >
DHTMessageTracker::messageArrived(const BDE& dict,
                                  const std::string& ipaddr, uint16_t port)
{
  const BDE& tid = dict[DHTMessage::T];
  if(!tid.isString()) {
    throw DL_ABORT_EX(StringFormat("Malformed DHT message. From:%s:%u",
                                   ipaddr.c_str(), port).str());
  }
  _logger->debug("Searching tracker entry for TransactionID=%s, Remote=%s:%u",
                 util::toHex(tid.s()).c_str(), ipaddr.c_str(), port);
  for(std::deque<SharedHandle<DHTMessageTrackerEntry> >::iterator i =
        _entries.begin(); i != _entries.end(); ++i) {
    if((*i)->match(tid.s(), ipaddr, port)) {
      SharedHandle<DHTMessageTrackerEntry> entry = *i;
      _entries.erase(i);
      _logger->debug("Tracker entry found.");
      SharedHandle<DHTNode> targetNode = entry->getTargetNode();

      SharedHandle<DHTMessage> message =
        _factory->createResponseMessage(entry->getMessageType(), dict,
                                        targetNode->getIPAddress(),
                                        targetNode->getPort());

      int64_t rtt = entry->getElapsedMillis();
      _logger->debug("RTT is %s", util::itos(rtt).c_str());
      message->getRemoteNode()->updateRTT(rtt);
      SharedHandle<DHTMessageCallback> callback = entry->getCallback();
      return std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> >(message, callback);
    }
  }
  _logger->debug("Tracker entry not found.");
  return std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> >();
}

void DHTMessageTracker::handleTimeout()
{
  for(std::deque<SharedHandle<DHTMessageTrackerEntry> >::iterator i = _entries.begin();
      i != _entries.end();) {
    if((*i)->isTimeout()) {
      try {
        SharedHandle<DHTMessageTrackerEntry> entry = *i;
        i = _entries.erase(i);
        SharedHandle<DHTNode> node = entry->getTargetNode();
        _logger->debug("Message timeout: To:%s:%u",
                       node->getIPAddress().c_str(), node->getPort());
        node->updateRTT(entry->getElapsedMillis());
        node->timeout();
        if(node->isBad()) {
          _logger->debug("Marked bad: %s:%u",
                         node->getIPAddress().c_str(), node->getPort());
          _routingTable->dropNode(node);
        }
        SharedHandle<DHTMessageCallback> callback = entry->getCallback();
        if(!callback.isNull()) {
          callback->onTimeout(node);
        }
      } catch(RecoverableException& e) {
        _logger->info("Exception thrown while handling timeouts.", e);
      }
    } else {
      ++i;
    }
  }
}

SharedHandle<DHTMessageTrackerEntry>
DHTMessageTracker::getEntryFor(const SharedHandle<DHTMessage>& message) const
{
  for(std::deque<SharedHandle<DHTMessageTrackerEntry> >::const_iterator i = _entries.begin();
      i != _entries.end(); ++i) {
    if((*i)->match(message->getTransactionID(), message->getRemoteNode()->getIPAddress(), message->getRemoteNode()->getPort())) {
      return *i;
    }
  }
  return SharedHandle<DHTMessageTrackerEntry>();
}

size_t DHTMessageTracker::countEntry() const
{
  return _entries.size();
}

void DHTMessageTracker::setRoutingTable(const SharedHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

void DHTMessageTracker::setMessageFactory(const SharedHandle<DHTMessageFactory>& factory)
{
  _factory = factory;
}

} // namespace aria2
