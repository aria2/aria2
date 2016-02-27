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
#ifndef D_DHT_TASK_FACTORY_IMPL_H
#define D_DHT_TASK_FACTORY_IMPL_H

#include "DHTTaskFactory.h"
#include "a2time.h"

namespace aria2 {

class DHTNode;
class DHTRoutingTable;
class DHTMessageDispatcher;
class DHTMessageFactory;
class DHTTaskQueue;
class DHTAbstractTask;

class DHTTaskFactoryImpl : public DHTTaskFactory {
private:
  std::shared_ptr<DHTNode> localNode_;

  DHTRoutingTable* routingTable_;

  DHTMessageDispatcher* dispatcher_;

  DHTMessageFactory* factory_;

  DHTTaskQueue* taskQueue_;

  std::chrono::seconds timeout_;

  void setCommonProperty(const std::shared_ptr<DHTAbstractTask>& task);

public:
  DHTTaskFactoryImpl();

  virtual ~DHTTaskFactoryImpl();

  virtual std::shared_ptr<DHTTask>
  createPingTask(const std::shared_ptr<DHTNode>& remoteNode,
                 int numRetry = 0) CXX11_OVERRIDE;

  virtual std::shared_ptr<DHTTask>
  createNodeLookupTask(const unsigned char* targetID) CXX11_OVERRIDE;

  virtual std::shared_ptr<DHTTask> createBucketRefreshTask() CXX11_OVERRIDE;

  virtual std::shared_ptr<DHTTask> createPeerLookupTask(
      const std::shared_ptr<DownloadContext>& ctx, uint16_t tcpPort,
      const std::shared_ptr<PeerStorage>& peerStorage) CXX11_OVERRIDE;

  virtual std::shared_ptr<DHTTask>
  createPeerAnnounceTask(const unsigned char* infoHash) CXX11_OVERRIDE;

  virtual std::shared_ptr<DHTTask>
  createReplaceNodeTask(const std::shared_ptr<DHTBucket>& bucket,
                        const std::shared_ptr<DHTNode>& newNode) CXX11_OVERRIDE;

  void setRoutingTable(DHTRoutingTable* routingTable);

  void setMessageDispatcher(DHTMessageDispatcher* dispatcher);

  void setMessageFactory(DHTMessageFactory* factory);

  void setTaskQueue(DHTTaskQueue* taskQueue);

  void setLocalNode(const std::shared_ptr<DHTNode>& localNode);

  void setTimeout(std::chrono::seconds timeout)
  {
    timeout_ = std::move(timeout);
  }
};

} // namespace aria2

#endif // D_DHT_TASK_FACTORY_IMPL_H
