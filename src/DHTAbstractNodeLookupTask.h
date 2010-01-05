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
#include "DHTMessageCallbackListener.h"
#include "DHTConstants.h"
#include <deque>

namespace aria2 {

class DHTNode;
class DHTNodeLookupEntry;
class DHTMessage;

class DHTAbstractNodeLookupTask:public DHTAbstractTask, public DHTMessageCallbackListener {
protected:
  unsigned char _targetID[DHT_ID_LENGTH];

  std::deque<SharedHandle<DHTNodeLookupEntry> > _entries;
  
  size_t _inFlightMessage;
  
  void toEntries(std::deque<SharedHandle<DHTNodeLookupEntry> >& entries,
                 const std::deque<SharedHandle<DHTNode> >& nodes) const;

  void sendMessage();

  void updateBucket();

  void sendMessageAndCheckFinish();
public:
  DHTAbstractNodeLookupTask(const unsigned char* targetID);

  static const size_t ALPHA = 3;

  virtual void startup();

  virtual void onReceived(const SharedHandle<DHTMessage>& message);

  virtual void onTimeout(const SharedHandle<DHTNode>& node);

  virtual void getNodesFromMessage(std::deque<SharedHandle<DHTNode> >& nodes,
                                   const SharedHandle<DHTMessage>& message) = 0;
  
  virtual void onReceivedInternal(const SharedHandle<DHTMessage>& message) {}
  
  virtual bool needsAdditionalOutgoingMessage() { return true; }
  
  virtual void onFinish() {}

  virtual SharedHandle<DHTMessage> createMessage(const SharedHandle<DHTNode>& remoteNode) = 0;
};

} // namespace aria2

#endif // _D_DHT_ABSTRACT_NODE_LOOKUP_TASK_H_
