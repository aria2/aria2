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
#ifndef _D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H_
#define _D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H_

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

namespace aria2 {

class DHTNode;
class DHTMessage;

template<class ResponseMessage>
class DHTAbstractNodeLookupTask:public DHTAbstractTask {
private:
  unsigned char _targetID[DHT_ID_LENGTH];

  std::deque<SharedHandle<DHTNodeLookupEntry> > _entries;
  
  size_t _inFlightMessage;
  
  template<typename Container>
  void toEntries
  (Container& entries, const std::vector<SharedHandle<DHTNode> >& nodes) const
  {
    for(std::vector<SharedHandle<DHTNode> >::const_iterator i = nodes.begin(),
          eoi = nodes.end(); i != eoi; ++i) {
      SharedHandle<DHTNodeLookupEntry> e(new DHTNodeLookupEntry(*i));
      entries.push_back(e);
    }
  }

  void sendMessage()
  {
    for(std::deque<SharedHandle<DHTNodeLookupEntry> >::iterator i =
          _entries.begin(), eoi = _entries.end();
        i != eoi && _inFlightMessage < ALPHA; ++i) {
      if((*i)->used == false) {
        ++_inFlightMessage;
        (*i)->used = true;
        SharedHandle<DHTMessage> m = createMessage((*i)->node);
        SharedHandle<DHTMessageCallback> callback(createCallback());
        getMessageDispatcher()->addMessageToQueue(m, callback);
      }
    }
  }

  void sendMessageAndCheckFinish()
  {
    if(needsAdditionalOutgoingMessage()) {
      sendMessage();
    }
    if(_inFlightMessage == 0) {
      if(getLogger()->debug()) {
        getLogger()->debug("Finished node_lookup for node ID %s",
                           util::toHex(_targetID, DHT_ID_LENGTH).c_str());
      }
      onFinish();
      updateBucket();
      setFinished(true);
    } else {
      if(getLogger()->debug()) {
        getLogger()->debug("%d in flight message for node ID %s",
                           _inFlightMessage,
                           util::toHex(_targetID, DHT_ID_LENGTH).c_str());
      }
    }
  }

  void updateBucket() {}
protected:
  const unsigned char* getTargetID() const
  {
    return _targetID;
  }

  const std::deque<SharedHandle<DHTNodeLookupEntry> >& getEntries() const
  {
    return _entries;
  }

  virtual void getNodesFromMessage
  (std::vector<SharedHandle<DHTNode> >& nodes,
   const ResponseMessage* message) = 0;
  
  virtual void onReceivedInternal
  (const ResponseMessage* message) {}
  
  virtual bool needsAdditionalOutgoingMessage() { return true; }
  
  virtual void onFinish() {}

  virtual SharedHandle<DHTMessage> createMessage
  (const SharedHandle<DHTNode>& remoteNode) = 0;

  virtual SharedHandle<DHTMessageCallback> createCallback() = 0;
public:
  DHTAbstractNodeLookupTask(const unsigned char* targetID):
    _inFlightMessage(0)
  {
    memcpy(_targetID, targetID, DHT_ID_LENGTH);
  }

  static const size_t ALPHA = 3;

  virtual void startup()
  {
    std::vector<SharedHandle<DHTNode> > nodes;
    getRoutingTable()->getClosestKNodes(nodes, _targetID);
    _entries.clear();
    toEntries(_entries, nodes);
    if(_entries.empty()) {
      setFinished(true);
    } else {
      // TODO use RTT here
      _inFlightMessage = 0;
      sendMessage();
      if(_inFlightMessage == 0) {
        if(getLogger()->debug()) {
          getLogger()->debug("No message was sent in this lookup stage. Finished.");
        }
        setFinished(true);
      }
    }
  }

  void onReceived(const ResponseMessage* message)
  {
    --_inFlightMessage;
    onReceivedInternal(message);
    std::vector<SharedHandle<DHTNode> > nodes;
    getNodesFromMessage(nodes, message);
    std::vector<SharedHandle<DHTNodeLookupEntry> > newEntries;
    toEntries(newEntries, nodes);

    size_t count = 0;
    for(std::vector<SharedHandle<DHTNodeLookupEntry> >::const_iterator i =
          newEntries.begin(), eoi = newEntries.end(); i != eoi; ++i) {
      if(memcmp(getLocalNode()->getID(), (*i)->node->getID(),
                DHT_ID_LENGTH) != 0) {
        _entries.push_front(*i);
        ++count;
        if(getLogger()->debug()) {
          getLogger()->debug("Received nodes: id=%s, ip=%s",
                             util::toHex((*i)->node->getID(),
                                         DHT_ID_LENGTH).c_str(),
                             (*i)->node->getIPAddress().c_str());
        }
      }
    }
    if(getLogger()->debug()) {
      getLogger()->debug("%u node lookup entries added.", count);
    }
    std::stable_sort(_entries.begin(), _entries.end(), DHTIDCloser(_targetID));
    _entries.erase(std::unique(_entries.begin(), _entries.end()), _entries.end());
    if(getLogger()->debug()) {
      getLogger()->debug("%u node lookup entries are unique.", _entries.size());
    }
    if(_entries.size() > DHTBucket::K) {
      _entries.erase(_entries.begin()+DHTBucket::K, _entries.end());
    }
    sendMessageAndCheckFinish();
  }

  void onTimeout(const SharedHandle<DHTNode>& node)
  {
    if(getLogger()->debug()) {
      getLogger()->debug("node lookup message timeout for node ID=%s",
                         util::toHex(node->getID(), DHT_ID_LENGTH).c_str());
    }
    --_inFlightMessage;
    for(std::deque<SharedHandle<DHTNodeLookupEntry> >::iterator i =
          _entries.begin(), eoi = _entries.end(); i != eoi; ++i) {
      if((*i)->node == node) {
        _entries.erase(i);
        break;
      }
    }
    sendMessageAndCheckFinish();
  }
};

} // namespace aria2

#endif // _D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H_
