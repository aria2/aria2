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
#ifndef D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H
#define D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H

#include "DHTAbstractTask.h"

#include <cstring>
#include <algorithm>
#include <deque>
#include <vector>

#include "DHTConstants.h"
#include "DHTNodeLookupEntry.h"
#include "DHTRoutingTable.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageFactory.h"
#include "DHTMessage.h"
#include "DHTNode.h"
#include "DHTBucket.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "DHTIDCloser.h"
#include "a2functional.h"
#include "fmt.h"

namespace aria2 {

class DHTNode;
class DHTMessage;

template <class ResponseMessage>
class DHTAbstractNodeLookupTask : public DHTAbstractTask {
private:
  unsigned char targetID_[DHT_ID_LENGTH];

  std::deque<std::unique_ptr<DHTNodeLookupEntry>> entries_;

  size_t inFlightMessage_;

  template <typename Container>
  void toEntries(Container& entries,
                 const std::vector<std::shared_ptr<DHTNode>>& nodes) const
  {
    for (auto& node : nodes) {
      entries.push_back(make_unique<DHTNodeLookupEntry>(node));
    }
  }

  void sendMessage()
  {
    for (auto i = std::begin(entries_), eoi = std::end(entries_);
         i != eoi && inFlightMessage_ < ALPHA; ++i) {
      if ((*i)->used == false) {
        ++inFlightMessage_;
        (*i)->used = true;
        getMessageDispatcher()->addMessageToQueue(createMessage((*i)->node),
                                                  createCallback());
      }
    }
  }

  void sendMessageAndCheckFinish()
  {
    if (needsAdditionalOutgoingMessage()) {
      sendMessage();
    }
    if (inFlightMessage_ == 0) {
      A2_LOG_DEBUG(fmt("Finished node_lookup for node ID %s",
                       util::toHex(targetID_, DHT_ID_LENGTH).c_str()));
      onFinish();
      updateBucket();
      setFinished(true);
    }
    else {
      A2_LOG_DEBUG(fmt("%lu in flight message for node ID %s",
                       static_cast<unsigned long>(inFlightMessage_),
                       util::toHex(targetID_, DHT_ID_LENGTH).c_str()));
    }
  }

  void updateBucket() {}

protected:
  const unsigned char* getTargetID() const { return targetID_; }

  const std::deque<std::unique_ptr<DHTNodeLookupEntry>>& getEntries() const
  {
    return entries_;
  }

  virtual void getNodesFromMessage(std::vector<std::shared_ptr<DHTNode>>& nodes,
                                   const ResponseMessage* message) = 0;

  virtual void onReceivedInternal(const ResponseMessage* message) {}

  virtual bool needsAdditionalOutgoingMessage() { return true; }

  virtual void onFinish() {}

  virtual std::unique_ptr<DHTMessage>
  createMessage(const std::shared_ptr<DHTNode>& remoteNode) = 0;

  virtual std::unique_ptr<DHTMessageCallback> createCallback() = 0;

public:
  DHTAbstractNodeLookupTask(const unsigned char* targetID) : inFlightMessage_(0)
  {
    memcpy(targetID_, targetID, DHT_ID_LENGTH);
  }

  static const size_t ALPHA = 3;

  virtual void startup() CXX11_OVERRIDE
  {
    std::vector<std::shared_ptr<DHTNode>> nodes;
    getRoutingTable()->getClosestKNodes(nodes, targetID_);
    entries_.clear();
    toEntries(entries_, nodes);
    if (entries_.empty()) {
      setFinished(true);
    }
    else {
      // TODO use RTT here
      inFlightMessage_ = 0;
      sendMessage();
      if (inFlightMessage_ == 0) {
        A2_LOG_DEBUG("No message was sent in this lookup stage. Finished.");
        setFinished(true);
      }
    }
  }

  void onReceived(const ResponseMessage* message)
  {
    --inFlightMessage_;
    // Replace old Node ID with new Node ID.
    for (auto& entry : entries_) {
      if (entry->node->getIPAddress() ==
              message->getRemoteNode()->getIPAddress() &&
          entry->node->getPort() == message->getRemoteNode()->getPort()) {
        entry->node = message->getRemoteNode();
      }
    }
    onReceivedInternal(message);
    std::vector<std::shared_ptr<DHTNode>> nodes;
    getNodesFromMessage(nodes, message);
    std::vector<std::unique_ptr<DHTNodeLookupEntry>> newEntries;
    toEntries(newEntries, nodes);

    size_t count = 0;
    for (auto& ne : newEntries) {
      if (memcmp(getLocalNode()->getID(), ne->node->getID(), DHT_ID_LENGTH) !=
          0) {
        A2_LOG_DEBUG(fmt("Received nodes: id=%s, ip=%s",
                         util::toHex(ne->node->getID(), DHT_ID_LENGTH).c_str(),
                         ne->node->getIPAddress().c_str()));
        entries_.push_front(std::move(ne));
        ++count;
      }
    }
    A2_LOG_DEBUG(fmt("%lu node lookup entries added.",
                     static_cast<unsigned long>(count)));
    std::stable_sort(std::begin(entries_), std::end(entries_),
                     DHTIDCloser(targetID_));
    entries_.erase(
        std::unique(std::begin(entries_), std::end(entries_),
                    DerefEqualTo<std::unique_ptr<DHTNodeLookupEntry>>{}),
        std::end(entries_));
    A2_LOG_DEBUG(fmt("%lu node lookup entries are unique.",
                     static_cast<unsigned long>(entries_.size())));
    if (entries_.size() > DHTBucket::K) {
      entries_.erase(std::begin(entries_) + DHTBucket::K, std::end(entries_));
    }
    sendMessageAndCheckFinish();
  }

  void onTimeout(const std::shared_ptr<DHTNode>& node)
  {
    A2_LOG_DEBUG(fmt("node lookup message timeout for node ID=%s",
                     util::toHex(node->getID(), DHT_ID_LENGTH).c_str()));
    --inFlightMessage_;
    for (auto i = std::begin(entries_), eoi = std::end(entries_); i != eoi;
         ++i) {
      if (*(*i)->node == *node) {
        entries_.erase(i);
        break;
      }
    }
    sendMessageAndCheckFinish();
  }
};

} // namespace aria2

#endif // D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H
