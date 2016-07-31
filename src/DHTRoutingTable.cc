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
#include "DHTRoutingTable.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTBucket.h"
#include "DHTBucketTree.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "fmt.h"

namespace aria2 {

DHTRoutingTable::DHTRoutingTable(const std::shared_ptr<DHTNode>& localNode)
    : localNode_(localNode),
      root_(make_unique<DHTBucketTreeNode>(
          std::make_shared<DHTBucket>(localNode_))),
      numBucket_(1),
      taskQueue_{nullptr},
      taskFactory_{nullptr}
{
}

DHTRoutingTable::~DHTRoutingTable() = default;

bool DHTRoutingTable::addNode(const std::shared_ptr<DHTNode>& node)
{
  return addNode(node, false);
}

bool DHTRoutingTable::addGoodNode(const std::shared_ptr<DHTNode>& node)
{
  return addNode(node, true);
}

bool DHTRoutingTable::addNode(const std::shared_ptr<DHTNode>& node, bool good)
{
  A2_LOG_DEBUG(fmt("Trying to add node:%s", node->toString().c_str()));
  if (*localNode_ == *node) {
    A2_LOG_DEBUG("Adding node with the same ID with localnode is not allowed.");
    return false;
  }
  auto treeNode = dht::findTreeNodeFor(root_.get(), node->getID());
  while (1) {
    const std::shared_ptr<DHTBucket>& bucket = treeNode->getBucket();
    if (bucket->addNode(node)) {
      A2_LOG_DEBUG("Added DHTNode.");
      return true;
    }
    else if (bucket->splitAllowed()) {
      A2_LOG_DEBUG(fmt("Splitting bucket. Range:%s-%s",
                       util::toHex(bucket->getMinID(), DHT_ID_LENGTH).c_str(),
                       util::toHex(bucket->getMaxID(), DHT_ID_LENGTH).c_str()));
      treeNode->split();
      ++numBucket_;
      if (treeNode->getLeft()->isInRange(node->getID())) {
        treeNode = treeNode->getLeft();
      }
      else {
        treeNode = treeNode->getRight();
      }
    }
    else {
      if (good) {
        bucket->cacheNode(node);
        A2_LOG_DEBUG(fmt("Cached node=%s", node->toString().c_str()));
      }
      return false;
    }
  }
  return false;
}

void DHTRoutingTable::getClosestKNodes(
    std::vector<std::shared_ptr<DHTNode>>& nodes,
    const unsigned char* key) const
{
  dht::findClosestKNodes(nodes, root_.get(), key);
}

int DHTRoutingTable::getNumBucket() const { return numBucket_; }

void DHTRoutingTable::showBuckets() const
{
  /*
    for(std::deque<std::shared_ptr<DHTBucket> >::const_iterator itr =
    buckets_.begin(); itr != buckets_.end(); ++itr) {
    cerr << "prefix = " << (*itr)->getPrefixLength() << ", "
    << "nodes = " << (*itr)->countNode() << endl;
    }
  */
}

std::shared_ptr<DHTBucket>
DHTRoutingTable::getBucketFor(const unsigned char* nodeID) const
{
  return dht::findBucketFor(root_.get(), nodeID);
}

std::shared_ptr<DHTBucket>
DHTRoutingTable::getBucketFor(const std::shared_ptr<DHTNode>& node) const
{
  return getBucketFor(node->getID());
}

std::shared_ptr<DHTNode> DHTRoutingTable::getNode(const unsigned char* nodeID,
                                                  const std::string& ipaddr,
                                                  uint16_t port) const
{
  std::shared_ptr<DHTBucket> bucket = getBucketFor(nodeID);
  return bucket->getNode(nodeID, ipaddr, port);
}

void DHTRoutingTable::dropNode(const std::shared_ptr<DHTNode>& node)
{
  getBucketFor(node)->dropNode(node);
}
/*
  void DHTRoutingTable::moveBucketHead(const std::shared_ptr<DHTNode>& node)
  {
  getBucketFor(node)->moveToHead(node);
  }
*/
void DHTRoutingTable::moveBucketTail(const std::shared_ptr<DHTNode>& node)
{
  getBucketFor(node)->moveToTail(node);
}

void DHTRoutingTable::getBuckets(
    std::vector<std::shared_ptr<DHTBucket>>& buckets) const
{
  dht::enumerateBucket(buckets, root_.get());
}

void DHTRoutingTable::setTaskQueue(DHTTaskQueue* taskQueue)
{
  taskQueue_ = taskQueue;
}

void DHTRoutingTable::setTaskFactory(DHTTaskFactory* taskFactory)
{
  taskFactory_ = taskFactory;
}

} // namespace aria2
