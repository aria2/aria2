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
#ifndef D_DHT_BUCKET_H
#define D_DHT_BUCKET_H

#include "common.h"

#include <string>
#include <deque>
#include <vector>
#include <memory>

#include "DHTConstants.h"
#include "TimerA2.h"

namespace aria2 {

class DHTNode;

class DHTBucket {
private:
  size_t prefixLength_;

  // this bucket contains nodes of distance between [min_, max_](inclusive).
  unsigned char min_[DHT_ID_LENGTH];

  unsigned char max_[DHT_ID_LENGTH];

  std::shared_ptr<DHTNode> localNode_;

  // sorted in ascending order
  std::deque<std::shared_ptr<DHTNode>> nodes_;

  // a replacement cache. The maximum size is specified by CACHE_SIZE.
  // This is sorted by last time seen.
  std::deque<std::shared_ptr<DHTNode>> cachedNodes_;

  Timer lastUpdated_;

  bool isInRange(const unsigned char* nodeID, const unsigned char* max,
                 const unsigned char* min) const;

public:
  DHTBucket(const std::shared_ptr<DHTNode>& localNode);

  DHTBucket(size_t prefixLength, const unsigned char* max,
            const unsigned char* min,
            const std::shared_ptr<DHTNode>& localNode);

  ~DHTBucket();

  static const size_t K = 8;

  static const size_t CACHE_SIZE = 2;

  void getRandomNodeID(unsigned char* nodeID) const;

  std::unique_ptr<DHTBucket> split();

  bool isInRange(const std::shared_ptr<DHTNode>& node) const;

  bool isInRange(const unsigned char* nodeID) const;

  bool addNode(const std::shared_ptr<DHTNode>& node);

  void cacheNode(const std::shared_ptr<DHTNode>& node);

  bool splitAllowed() const;

  size_t getPrefixLength() const { return prefixLength_; }

  const unsigned char* getMaxID() const { return max_; }

  const unsigned char* getMinID() const { return min_; }

  size_t countNode() const { return nodes_.size(); }

  const std::deque<std::shared_ptr<DHTNode>>& getNodes() const
  {
    return nodes_;
  }

  void getGoodNodes(std::vector<std::shared_ptr<DHTNode>>& nodes) const;

  void dropNode(const std::shared_ptr<DHTNode>& node);

  void moveToHead(const std::shared_ptr<DHTNode>& node);

  void moveToTail(const std::shared_ptr<DHTNode>& node);

  bool contains(const std::shared_ptr<DHTNode>& node) const;

  std::shared_ptr<DHTNode> getNode(const unsigned char* nodeID,
                                   const std::string& ipaddr,
                                   uint16_t port) const;

  bool operator==(const DHTBucket& bucket) const;

  bool needsRefresh() const;

  void notifyUpdate();

  bool containsQuestionableNode() const;

  std::shared_ptr<DHTNode> getLRUQuestionableNode() const;

  const std::deque<std::shared_ptr<DHTNode>>& getCachedNodes() const
  {
    return cachedNodes_;
  }
};

} // namespace aria2

#endif // D_DHT_BUCKET_H
