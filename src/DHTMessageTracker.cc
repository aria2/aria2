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
#include "fmt.h"

namespace aria2 {

DHTMessageTracker::DHTMessageTracker() {}

DHTMessageTracker::~DHTMessageTracker() {}

void DHTMessageTracker::addMessage(const SharedHandle<DHTMessage>& message, time_t timeout, const SharedHandle<DHTMessageCallback>& callback)
{
  SharedHandle<DHTMessageTrackerEntry> e(new DHTMessageTrackerEntry(message, timeout, callback));
  entries_.push_back(e);
}

std::pair<SharedHandle<DHTResponseMessage>, SharedHandle<DHTMessageCallback> >
DHTMessageTracker::messageArrived
(const Dict* dict, const std::string& ipaddr, uint16_t port)
{
  const String* tid = downcast<String>(dict->get(DHTMessage::T));
  if(!tid) {
    throw DL_ABORT_EX(fmt("Malformed DHT message. From:%s:%u",
                          ipaddr.c_str(), port));
  }
  A2_LOG_DEBUG(fmt("Searching tracker entry for TransactionID=%s, Remote=%s:%u",
                   util::toHex(tid->s()).c_str(),
                   ipaddr.c_str(), port));
  for(std::deque<SharedHandle<DHTMessageTrackerEntry> >::iterator i =
        entries_.begin(), eoi = entries_.end(); i != eoi; ++i) {
    if((*i)->match(tid->s(), ipaddr, port)) {
      SharedHandle<DHTMessageTrackerEntry> entry = *i;
      entries_.erase(i);
      A2_LOG_DEBUG("Tracker entry found.");
      SharedHandle<DHTNode> targetNode = entry->getTargetNode();
      try {
        SharedHandle<DHTResponseMessage> message =
          factory_->createResponseMessage(entry->getMessageType(), dict,
                                          targetNode->getIPAddress(),
                                          targetNode->getPort());

        int64_t rtt = entry->getElapsedMillis();
        A2_LOG_DEBUG(fmt("RTT is %" PRId64 "", rtt));
        message->getRemoteNode()->updateRTT(rtt);
        SharedHandle<DHTMessageCallback> callback = entry->getCallback();
        if(!(*targetNode == *message->getRemoteNode())) {
          // Node ID has changed. Drop previous node ID from
          // DHTRoutingTable
          A2_LOG_DEBUG
            (fmt("Node ID has changed: old:%s, new:%s",
                 util::toHex(targetNode->getID(), DHT_ID_LENGTH).c_str(),
                 util::toHex(message->getRemoteNode()->getID(),
                             DHT_ID_LENGTH).c_str()));
          routingTable_->dropNode(targetNode);
        }
        return std::make_pair(message, callback);
      } catch(RecoverableException& e) {
        handleTimeoutEntry(entry);
        throw;
      }
    }
  }
  A2_LOG_DEBUG("Tracker entry not found.");
  return std::pair<SharedHandle<DHTResponseMessage>,
                   SharedHandle<DHTMessageCallback> >();
}

void DHTMessageTracker::handleTimeoutEntry
(const SharedHandle<DHTMessageTrackerEntry>& entry)
{
  try {
    SharedHandle<DHTNode> node = entry->getTargetNode();
    A2_LOG_DEBUG(fmt("Message timeout: To:%s:%u",
                     node->getIPAddress().c_str(), node->getPort()));
    node->updateRTT(entry->getElapsedMillis());
    node->timeout();
    if(node->isBad()) {
      A2_LOG_DEBUG(fmt("Marked bad: %s:%u",
                       node->getIPAddress().c_str(), node->getPort()));
      routingTable_->dropNode(node);
    }
    SharedHandle<DHTMessageCallback> callback = entry->getCallback();
    if(callback) {
      callback->onTimeout(node);
    }
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX("Exception thrown while handling timeouts.", e);
  }
}

void DHTMessageTracker::handleTimeout()
{
  for(std::deque<SharedHandle<DHTMessageTrackerEntry> >::iterator i =
        entries_.begin(), eoi = entries_.end(); i != eoi;) {
    if((*i)->isTimeout()) {
        SharedHandle<DHTMessageTrackerEntry> entry = *i;
        i = entries_.erase(i);
        eoi = entries_.end();
        handleTimeoutEntry(entry);
    } else {
      ++i;
    }
  }
}

SharedHandle<DHTMessageTrackerEntry>
DHTMessageTracker::getEntryFor(const SharedHandle<DHTMessage>& message) const
{
  for(std::deque<SharedHandle<DHTMessageTrackerEntry> >::const_iterator i =
        entries_.begin(), eoi = entries_.end(); i != eoi; ++i) {
    if((*i)->match(message->getTransactionID(),
                   message->getRemoteNode()->getIPAddress(),
                   message->getRemoteNode()->getPort())) {
      return *i;
    }
  }
  return SharedHandle<DHTMessageTrackerEntry>();
}

size_t DHTMessageTracker::countEntry() const
{
  return entries_.size();
}

void DHTMessageTracker::setRoutingTable
(const SharedHandle<DHTRoutingTable>& routingTable)
{
  routingTable_ = routingTable;
}

void DHTMessageTracker::setMessageFactory
(const SharedHandle<DHTMessageFactory>& factory)
{
  factory_ = factory;
}

} // namespace aria2
