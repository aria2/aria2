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
#include "DHTBucket.h"

#include <cstring>
#include <cassert>
#include <algorithm>

#include "DHTNode.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "DHTConstants.h"
#include "a2functional.h"
#include "bittorrent_helper.h"
#include "bitfield.h"
#include "wallclock.h"
#include "fmt.h"

namespace aria2 {

DHTBucket::DHTBucket(size_t prefixLength, const unsigned char* max,
                     const unsigned char* min,
                     const std::shared_ptr<DHTNode>& localNode)
    : prefixLength_(prefixLength),
      localNode_(localNode),
      lastUpdated_(global::wallclock())
{
  memcpy(max_, max, DHT_ID_LENGTH);
  memcpy(min_, min, DHT_ID_LENGTH);
}

DHTBucket::DHTBucket(const std::shared_ptr<DHTNode>& localNode)
    : prefixLength_(0), localNode_(localNode), lastUpdated_(global::wallclock())
{
  memset(max_, 0xffu, DHT_ID_LENGTH);
  memset(min_, 0, DHT_ID_LENGTH);
}

DHTBucket::~DHTBucket() = default;

void DHTBucket::getRandomNodeID(unsigned char* nodeID) const
{
  if (prefixLength_ == 0) {
    util::generateRandomKey(nodeID);
  }
  else {
    size_t lastByteIndex = (prefixLength_ - 1) / 8;
    util::generateRandomKey(nodeID);
    memcpy(nodeID, min_, lastByteIndex + 1);
  }
}

bool DHTBucket::isInRange(const std::shared_ptr<DHTNode>& node) const
{
  return isInRange(node->getID(), max_, min_);
}

bool DHTBucket::isInRange(const unsigned char* nodeID) const
{
  return isInRange(nodeID, max_, min_);
}

// Returns true if nodeID is in [min, max] (inclusive).
bool DHTBucket::isInRange(const unsigned char* nodeID, const unsigned char* max,
                          const unsigned char* min) const
{
  return !std::lexicographical_compare(&nodeID[0], &nodeID[DHT_ID_LENGTH],
                                       &min[0], &min[DHT_ID_LENGTH]) &&
         !std::lexicographical_compare(&max[0], &max[DHT_ID_LENGTH], &nodeID[0],
                                       &nodeID[DHT_ID_LENGTH]);
}

bool DHTBucket::addNode(const std::shared_ptr<DHTNode>& node)
{
  notifyUpdate();
  auto itr = std::find_if(nodes_.begin(), nodes_.end(), derefEqual(node));
  if (itr == nodes_.end()) {
    if (nodes_.size() < K) {
      nodes_.push_back(node);
      return true;
    }
    else {
      if (nodes_.front()->isBad()) {
        nodes_.erase(nodes_.begin());
        nodes_.push_back(node);
        return true;
      }
      else {
        return false;
      }
    }
  }
  else {
    nodes_.erase(itr);
    nodes_.push_back(node);
    return true;
  }
}

void DHTBucket::cacheNode(const std::shared_ptr<DHTNode>& node)
{
  // cachedNodes_ are sorted by last time seen
  cachedNodes_.push_front(node);
  if (cachedNodes_.size() > CACHE_SIZE) {
    cachedNodes_.resize(CACHE_SIZE, std::shared_ptr<DHTNode>());
  }
}

void DHTBucket::dropNode(const std::shared_ptr<DHTNode>& node)
{
  if (!cachedNodes_.empty()) {
    auto itr = std::find_if(nodes_.begin(), nodes_.end(), derefEqual(node));
    if (itr != nodes_.end()) {
      nodes_.erase(itr);
      nodes_.push_back(cachedNodes_.front());
      cachedNodes_.erase(cachedNodes_.begin());
    }
  }
}

void DHTBucket::moveToHead(const std::shared_ptr<DHTNode>& node)
{
  auto itr = std::find_if(nodes_.begin(), nodes_.end(), derefEqual(node));
  if (itr != nodes_.end()) {
    nodes_.erase(itr);
    nodes_.push_front(node);
  }
}

void DHTBucket::moveToTail(const std::shared_ptr<DHTNode>& node)
{
  auto itr = std::find_if(nodes_.begin(), nodes_.end(), derefEqual(node));
  if (itr != nodes_.end()) {
    nodes_.erase(itr);
    nodes_.push_back(node);
  }
}

bool DHTBucket::splitAllowed() const
{
  return prefixLength_ < DHT_ID_LENGTH * 8 - 1 && isInRange(localNode_);
}

std::unique_ptr<DHTBucket> DHTBucket::split()
{
  assert(splitAllowed());

  unsigned char rMax[DHT_ID_LENGTH];
  memcpy(rMax, max_, DHT_ID_LENGTH);
  bitfield::flipBit(rMax, DHT_ID_LENGTH, prefixLength_);
  unsigned char rMin[DHT_ID_LENGTH];
  memcpy(rMin, min_, DHT_ID_LENGTH);

  bitfield::flipBit(min_, DHT_ID_LENGTH, prefixLength_);

  ++prefixLength_;
  auto rBucket = make_unique<DHTBucket>(prefixLength_, rMax, rMin, localNode_);

  std::deque<std::shared_ptr<DHTNode>> lNodes;
  for (auto& elem : nodes_) {
    if (rBucket->isInRange(elem)) {
      assert(rBucket->addNode(elem));
    }
    else {
      lNodes.push_back(elem);
    }
  }
  nodes_ = lNodes;
  // TODO create toString() and use it.
  A2_LOG_DEBUG(fmt("New bucket. prefixLength=%u, Range:%s-%s",
                   static_cast<unsigned int>(rBucket->getPrefixLength()),
                   util::toHex(rBucket->getMinID(), DHT_ID_LENGTH).c_str(),
                   util::toHex(rBucket->getMaxID(), DHT_ID_LENGTH).c_str()));
  A2_LOG_DEBUG(fmt("Existing bucket. prefixLength=%u, Range:%s-%s",
                   static_cast<unsigned int>(prefixLength_),
                   util::toHex(getMinID(), DHT_ID_LENGTH).c_str(),
                   util::toHex(getMaxID(), DHT_ID_LENGTH).c_str()));
  return rBucket;
}

void DHTBucket::getGoodNodes(
    std::vector<std::shared_ptr<DHTNode>>& goodNodes) const
{
  goodNodes.insert(goodNodes.end(), nodes_.begin(), nodes_.end());
  goodNodes.erase(std::remove_if(goodNodes.begin(), goodNodes.end(),
                                 std::mem_fn(&DHTNode::isBad)),
                  goodNodes.end());
}

std::shared_ptr<DHTNode> DHTBucket::getNode(const unsigned char* nodeID,
                                            const std::string& ipaddr,
                                            uint16_t port) const
{
  auto node = std::make_shared<DHTNode>(nodeID);
  node->setIPAddress(ipaddr);
  node->setPort(port);
  auto itr = std::find_if(nodes_.begin(), nodes_.end(), derefEqual(node));
  if (itr == nodes_.end() || (*itr)->getIPAddress() != ipaddr ||
      (*itr)->getPort() != port) {
    return nullptr;
  }
  else {
    return *itr;
  }
}

bool DHTBucket::operator==(const DHTBucket& bucket) const
{
  return memcmp(max_, bucket.max_, DHT_ID_LENGTH) == 0 &&
         memcmp(min_, bucket.min_, DHT_ID_LENGTH) == 0;
}

bool DHTBucket::needsRefresh() const
{
  return nodes_.size() < K || lastUpdated_.difference(global::wallclock()) >=
                                  DHT_BUCKET_REFRESH_INTERVAL;
}

void DHTBucket::notifyUpdate() { lastUpdated_ = global::wallclock(); }

namespace {
class FindQuestionableNode {
public:
  bool operator()(const std::shared_ptr<DHTNode>& node) const
  {
    return node->isQuestionable();
  }
};
} // namespace

bool DHTBucket::containsQuestionableNode() const
{
  return std::find_if(nodes_.begin(), nodes_.end(), FindQuestionableNode()) !=
         nodes_.end();
}

std::shared_ptr<DHTNode> DHTBucket::getLRUQuestionableNode() const
{
  auto i = std::find_if(nodes_.begin(), nodes_.end(), FindQuestionableNode());
  if (i == nodes_.end()) {
    return nullptr;
  }
  else {
    return *i;
  }
}

} // namespace aria2
