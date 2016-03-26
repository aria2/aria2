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

DHTMessageTracker::DHTMessageTracker()
    : routingTable_{nullptr}, factory_{nullptr}
{
}

void DHTMessageTracker::addMessage(DHTMessage* message,
                                   std::chrono::seconds timeout,
                                   std::unique_ptr<DHTMessageCallback> callback)
{
  entries_.push_back(make_unique<DHTMessageTrackerEntry>(
      message->getRemoteNode(), message->getTransactionID(),
      message->getMessageType(), std::move(timeout), std::move(callback)));
}

std::pair<std::unique_ptr<DHTResponseMessage>,
          std::unique_ptr<DHTMessageCallback>>
DHTMessageTracker::messageArrived(const Dict* dict, const std::string& ipaddr,
                                  uint16_t port)
{
  const String* tid = downcast<String>(dict->get(DHTMessage::T));
  if (!tid) {
    throw DL_ABORT_EX(
        fmt("Malformed DHT message. From:%s:%u", ipaddr.c_str(), port));
  }
  A2_LOG_DEBUG(fmt("Searching tracker entry for TransactionID=%s, Remote=%s:%u",
                   util::toHex(tid->s()).c_str(), ipaddr.c_str(), port));
  for (auto i = std::begin(entries_), eoi = std::end(entries_); i != eoi; ++i) {
    if ((*i)->match(tid->s(), ipaddr, port)) {
      auto entry = std::move(*i);
      entries_.erase(i);
      A2_LOG_DEBUG("Tracker entry found.");
      auto& targetNode = entry->getTargetNode();
      try {
        auto message = factory_->createResponseMessage(
            entry->getMessageType(), dict, targetNode->getIPAddress(),
            targetNode->getPort());

        auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(
            entry->getElapsed());
        A2_LOG_DEBUG(
            fmt("RTT is %" PRId64 "", static_cast<int64_t>(rtt.count())));
        message->getRemoteNode()->updateRTT(rtt);
        if (*targetNode != *message->getRemoteNode()) {
          // Node ID has changed. Drop previous node ID from
          // DHTRoutingTable
          A2_LOG_DEBUG(
              fmt("Node ID has changed: old:%s, new:%s",
                  util::toHex(targetNode->getID(), DHT_ID_LENGTH).c_str(),
                  util::toHex(message->getRemoteNode()->getID(), DHT_ID_LENGTH)
                      .c_str()));
          routingTable_->dropNode(targetNode);
        }
        return std::make_pair(std::move(message), entry->popCallback());
      }
      catch (RecoverableException& e) {
        handleTimeoutEntry(entry.get());
        throw;
      }
    }
  }
  A2_LOG_DEBUG("Tracker entry not found.");
  return std::pair<std::unique_ptr<DHTResponseMessage>,
                   std::unique_ptr<DHTMessageCallback>>{};
}

void DHTMessageTracker::handleTimeoutEntry(DHTMessageTrackerEntry* entry)
{
  try {
    auto& node = entry->getTargetNode();
    A2_LOG_DEBUG(fmt("Message timeout: To:%s:%u", node->getIPAddress().c_str(),
                     node->getPort()));
    node->updateRTT(std::chrono::duration_cast<std::chrono::milliseconds>(
        entry->getElapsed()));
    node->timeout();
    if (node->isBad()) {
      A2_LOG_DEBUG(fmt("Marked bad: %s:%u", node->getIPAddress().c_str(),
                       node->getPort()));
      routingTable_->dropNode(node);
    }
    auto& callback = entry->getCallback();
    if (callback) {
      callback->onTimeout(node);
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX("Exception thrown while handling timeouts.", e);
  }
}

void DHTMessageTracker::handleTimeout()
{
  entries_.erase(
      std::remove_if(std::begin(entries_), std::end(entries_),
                     [&](const std::unique_ptr<DHTMessageTrackerEntry>& ent) {
                       if (ent->isTimeout()) {
                         handleTimeoutEntry(ent.get());
                         return true;
                       }
                       else {
                         return false;
                       }
                     }),
      std::end(entries_));
}

const DHTMessageTrackerEntry*
DHTMessageTracker::getEntryFor(const DHTMessage* message) const
{
  for (auto& ent : entries_) {
    if (ent->match(message->getTransactionID(),
                   message->getRemoteNode()->getIPAddress(),
                   message->getRemoteNode()->getPort())) {
      return ent.get();
    }
  }
  return nullptr;
}

size_t DHTMessageTracker::countEntry() const { return entries_.size(); }

void DHTMessageTracker::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

void DHTMessageTracker::setMessageFactory(DHTMessageFactory* factory)
{
  factory_ = factory;
}

} // namespace aria2
