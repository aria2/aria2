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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "DHTUtil.h"
#include "DHTNode.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Util.h"
#include "DHTConstants.h"
#include "a2functional.h"

namespace aria2 {

DHTBucket::DHTBucket(size_t prefixLength,
		     const unsigned char* max, const unsigned char* min,
		     const SharedHandle<DHTNode>& localNode):
  _prefixLength(prefixLength),
  _localNode(localNode),
  _logger(LogFactory::getInstance())
{
  memcpy(_max, max, DHT_ID_LENGTH);
  memcpy(_min, min, DHT_ID_LENGTH);
}

DHTBucket::DHTBucket(const SharedHandle<DHTNode>& localNode):
  _prefixLength(0),
  _localNode(localNode),
  _logger(LogFactory::getInstance())
{
  memset(_max, 0xff, DHT_ID_LENGTH);
  memset(_min, 0, DHT_ID_LENGTH);
}

DHTBucket::~DHTBucket() {}

void DHTBucket::getRandomNodeID(unsigned char* nodeID) const
{
  if(_prefixLength == 0) {
    DHTUtil::generateRandomKey(nodeID);
  } else {
    size_t lastByteIndex = (_prefixLength-1)/8;
    DHTUtil::generateRandomKey(nodeID);
    memcpy(nodeID, _min, lastByteIndex+1);
  }
}

bool DHTBucket::isInRange(const SharedHandle<DHTNode>& node) const
{
  return isInRange(node->getID(), _max, _min);
}

bool DHTBucket::isInRange(const unsigned char* nodeID) const
{
  return isInRange(nodeID, _max, _min);
}

// Returns true if nodeID is in [min, max] (inclusive).
bool DHTBucket::isInRange(const unsigned char* nodeID,
			  const unsigned char* max,
			  const unsigned char* min) const
{
  return
    !std::lexicographical_compare(&nodeID[0], &nodeID[DHT_ID_LENGTH],
				  &min[0], &min[DHT_ID_LENGTH]) &&
    !std::lexicographical_compare(&max[0], &max[DHT_ID_LENGTH],
				  &nodeID[0], &nodeID[DHT_ID_LENGTH]);
}

bool DHTBucket::addNode(const SharedHandle<DHTNode>& node)
{
  notifyUpdate();
  std::deque<SharedHandle<DHTNode> >::iterator itr = std::find(_nodes.begin(), _nodes.end(), node);
  if(itr == _nodes.end()) {
    if(_nodes.size() < K) {
      _nodes.push_back(node);
      return true;
    } else {
      if(_nodes.front()->isBad()) {
	_nodes.erase(_nodes.begin());
	_nodes.push_back(node);
	return true;
      } else {
	return false;
      }
    }
  } else {
    _nodes.erase(itr);
    _nodes.push_back(node);
    return true;
  }
}

void DHTBucket::cacheNode(const SharedHandle<DHTNode>& node)
{
  // _cachedNodes are sorted by last time seen
  _cachedNodes.push_front(node);
  if(_cachedNodes.size() > CACHE_SIZE) {
    _cachedNodes.resize(CACHE_SIZE, SharedHandle<DHTNode>());
  }
}

void DHTBucket::dropNode(const SharedHandle<DHTNode>& node)
{
  if(_cachedNodes.size()) {
    std::deque<SharedHandle<DHTNode> >::iterator itr = find(_nodes.begin(), _nodes.end(), node);
    if(itr != _nodes.end()) {
      _nodes.erase(itr);
      _nodes.push_back(_cachedNodes.front());
      _cachedNodes.erase(_cachedNodes.begin());
    }
  }
}

void DHTBucket::moveToHead(const SharedHandle<DHTNode>& node)
{
  std::deque<SharedHandle<DHTNode> >::iterator itr =
    std::find(_nodes.begin(), _nodes.end(), node);
  if(itr != _nodes.end()) {
    _nodes.erase(itr);
    _nodes.push_front(node);
  }
}

void DHTBucket::moveToTail(const SharedHandle<DHTNode>& node)
{
  std::deque<SharedHandle<DHTNode> >::iterator itr =
    std::find(_nodes.begin(), _nodes.end(), node);
  if(itr != _nodes.end()) {
    _nodes.erase(itr);
    _nodes.push_back(node);
  }
}

bool DHTBucket::splitAllowed() const
{
  return _prefixLength < DHT_ID_LENGTH*8-1 && isInRange(_localNode);
}

SharedHandle<DHTBucket> DHTBucket::split()
{
  assert(splitAllowed());

  unsigned char rMax[DHT_ID_LENGTH];
  memcpy(rMax, _max, DHT_ID_LENGTH);
  DHTUtil::flipBit(rMax, DHT_ID_LENGTH, _prefixLength);
  unsigned char rMin[DHT_ID_LENGTH];
  memcpy(rMin, _min, DHT_ID_LENGTH);

  DHTUtil::flipBit(_min, DHT_ID_LENGTH, _prefixLength);

  ++_prefixLength;
  SharedHandle<DHTBucket> rBucket(new DHTBucket(_prefixLength,
						rMax, rMin, _localNode));

  std::deque<SharedHandle<DHTNode> > lNodes;
  for(std::deque<SharedHandle<DHTNode> >::iterator i = _nodes.begin();
      i != _nodes.end(); ++i) {
    if(rBucket->isInRange(*i)) {
      assert(rBucket->addNode(*i));
    } else {
      lNodes.push_back(*i);
    }      
  }
  _nodes = lNodes;
  // TODO create toString() and use it.
  _logger->debug("New bucket. prefixLength=%u, Range:%s-%s",
		 static_cast<unsigned int>(rBucket->getPrefixLength()),
 		 Util::toHex(rBucket->getMinID(), DHT_ID_LENGTH).c_str(),
 		 Util::toHex(rBucket->getMaxID(), DHT_ID_LENGTH).c_str());
  _logger->debug("Existing bucket. prefixLength=%u, Range:%s-%s",
		 static_cast<unsigned int>(_prefixLength),
		 Util::toHex(getMinID(), DHT_ID_LENGTH).c_str(),
		 Util::toHex(getMaxID(), DHT_ID_LENGTH).c_str());

  return rBucket;
}

size_t DHTBucket::countNode() const
{
  return _nodes.size();
}

const std::deque<SharedHandle<DHTNode> >& DHTBucket::getNodes() const
{
  return _nodes;
}

void DHTBucket::getGoodNodes(std::deque<SharedHandle<DHTNode> >& goodNodes) const
{
  goodNodes = _nodes;
  goodNodes.erase(std::remove_if(goodNodes.begin(), goodNodes.end(),
				 mem_fun_sh(&DHTNode::isBad)), goodNodes.end());
}

SharedHandle<DHTNode> DHTBucket::getNode(const unsigned char* nodeID, const std::string& ipaddr, uint16_t port) const
{
  SharedHandle<DHTNode> node(new DHTNode(nodeID));
  node->setIPAddress(ipaddr);
  node->setPort(port);
  std::deque<SharedHandle<DHTNode> >::const_iterator itr =
    std::find(_nodes.begin(), _nodes.end(), node);
  if(itr == _nodes.end()) {
    return SharedHandle<DHTNode>();
  } else {
    return *itr;
  }
}

bool DHTBucket::operator==(const DHTBucket& bucket) const
{
  return memcmp(_max, bucket._max, DHT_ID_LENGTH) == 0 &&
    memcmp(_min, bucket._min, DHT_ID_LENGTH) == 0;
}

bool DHTBucket::needsRefresh() const
{
  return _nodes.size() < K || _lastUpdated.elapsed(DHT_BUCKET_REFRESH_INTERVAL);
}

void DHTBucket::notifyUpdate()
{
  _lastUpdated.reset();
}

class FindQuestionableNode {
public:
  bool operator()(const SharedHandle<DHTNode>& node) const
  {
    return node->isQuestionable();
  }
};

bool DHTBucket::containsQuestionableNode() const
{
  return std::find_if(_nodes.begin(), _nodes.end(), FindQuestionableNode()) != _nodes.end();
}

SharedHandle<DHTNode> DHTBucket::getLRUQuestionableNode() const
{
  std::deque<SharedHandle<DHTNode> >::const_iterator i =
    std::find_if(_nodes.begin(), _nodes.end(), FindQuestionableNode());
  if(i == _nodes.end()) {
    return SharedHandle<DHTNode>();
  } else {
    return *i;
  }
}

const std::deque<SharedHandle<DHTNode> >& DHTBucket::getCachedNodes() const
{
  return _cachedNodes;
}

} // namespace aria2
