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
#ifndef D_DHT_ROUTING_TABLE_H
#define D_DHT_ROUTING_TABLE_H

#include "common.h"

#include <string>
#include <vector>
#include <memory>

namespace aria2 {

class DHTNode;
class DHTBucket;
class DHTTaskQueue;
class DHTTaskFactory;
class DHTBucketTreeNode;

class DHTRoutingTable {
private:
  std::shared_ptr<DHTNode> localNode_;

  std::unique_ptr<DHTBucketTreeNode> root_;

  int numBucket_;

  DHTTaskQueue* taskQueue_;

  DHTTaskFactory* taskFactory_;

  bool addNode(const std::shared_ptr<DHTNode>& node, bool good);

public:
  DHTRoutingTable(const std::shared_ptr<DHTNode>& localNode);

  ~DHTRoutingTable();

  bool addNode(const std::shared_ptr<DHTNode>& node);

  bool addGoodNode(const std::shared_ptr<DHTNode>& node);

  void getClosestKNodes(std::vector<std::shared_ptr<DHTNode>>& nodes,
                        const unsigned char* key) const;

  int getNumBucket() const;

  void showBuckets() const;

  void dropNode(const std::shared_ptr<DHTNode>& node);

  void moveBucketHead(const std::shared_ptr<DHTNode>& node);

  void moveBucketTail(const std::shared_ptr<DHTNode>& node);

  std::shared_ptr<DHTBucket> getBucketFor(const unsigned char* nodeID) const;

  std::shared_ptr<DHTBucket>
  getBucketFor(const std::shared_ptr<DHTNode>& node) const;

  std::shared_ptr<DHTNode> getNode(const unsigned char* id,
                                   const std::string& ipaddr,
                                   uint16_t port) const;

  void getBuckets(std::vector<std::shared_ptr<DHTBucket>>& buckets) const;

  void setTaskQueue(DHTTaskQueue* taskQueue);

  void setTaskFactory(DHTTaskFactory* taskFactory);
};

} // namespace aria2

#endif // D_DHT_ROUTING_TABLE_H
