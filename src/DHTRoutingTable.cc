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
#include "DHTRoutingTable.h"
#include "DHTNode.h"
#include "DHTBucket.h"
#include "Util.h"
#include "LogFactory.h"
#include "BNode.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"

DHTRoutingTable::DHTRoutingTable(const DHTNodeHandle& localNode):
  _localNode(localNode),
  _root(new BNode(new DHTBucket(localNode))),
  _numBucket(1),
  _taskQueue(0),
  _taskFactory(0),
  _logger(LogFactory::getInstance())
{}

DHTRoutingTable::~DHTRoutingTable()
{
  delete _root;
}

bool DHTRoutingTable::addNode(const DHTNodeHandle& node)
{
  return addNode(node, false);
}

bool DHTRoutingTable::addGoodNode(const DHTNodeHandle& node)
{
  return addNode(node, true);
}

bool DHTRoutingTable::addNode(const DHTNodeHandle& node, bool good)
{
  _logger->debug("Trying to add node:%s", node->toString().c_str());
  BNode* bnode = BNode::findBNodeFor(_root, node->getID());
  DHTBucketHandle bucket = bnode->getBucket();
  while(1) {
    if(bucket->addNode(node)) {
      _logger->debug("Added DHTNode.");
      return true;
    } else if(bucket->splitAllowed()) {
      _logger->debug("Splitting bucket. Range:%s-%s",
		     Util::toHex(bucket->getMinID(), DHT_ID_LENGTH).c_str(),
		     Util::toHex(bucket->getMaxID(), DHT_ID_LENGTH).c_str());
      DHTBucketHandle r = bucket->split();

      bnode->setBucket(0);
      BNode* lbnode = new BNode(bucket);
      BNode* rbnode = new BNode(r);
      bnode->setLeft(lbnode);
      bnode->setRight(rbnode);
      ++_numBucket;

      if(r->isInRange(node)) {
	bucket = r;
	bnode = rbnode;
      } else {
	bnode = lbnode;
      }
    } else {
      if(good && bucket->containsQuestionableNode()) {
	_logger->debug("Issuing ReplaceNodeTask: new node=%s", node->toString().c_str());
	_taskQueue->addImmediateTask(_taskFactory->createReplaceNodeTask(bucket, node));
      }
      return false;
    }
  }
  return false;
}

DHTNodes DHTRoutingTable::getClosestKNodes(const unsigned char* key) const
{
  return BNode::findClosestKNodes(_root, key);
}

size_t DHTRoutingTable::countBucket() const
{
  return _numBucket;
}

void DHTRoutingTable::showBuckets() const
{/*
  for(DHTBuckets::const_iterator itr = _buckets.begin(); itr != _buckets.end(); ++itr) {
    cerr << "prefix = " << (*itr)->getPrefixLength() << ", "
	 << "nodes = " << (*itr)->countNode() << endl;
  }
 */
}

DHTBucketHandle DHTRoutingTable::getBucketFor(const unsigned char* nodeID) const
{
  return BNode::findBucketFor(_root, nodeID);
}

DHTBucketHandle DHTRoutingTable::getBucketFor(const DHTNodeHandle& node) const
{
  return getBucketFor(node->getID());
}

DHTNodeHandle DHTRoutingTable::getNode(const unsigned char* nodeID, const string& ipaddr, uint16_t port) const
{
  DHTBucketHandle bucket = getBucketFor(nodeID);
  return bucket->getNode(nodeID, ipaddr, port);
}

void DHTRoutingTable::dropNode(const DHTNodeHandle& node)
{
  getBucketFor(node)->dropNode(node);
}
/*
void DHTRoutingTable::moveBucketHead(const DHTNodeHandle& node)
{
  getBucketFor(node)->moveToHead(node);
}
*/
void DHTRoutingTable::moveBucketTail(const DHTNodeHandle& node)
{
  getBucketFor(node)->moveToTail(node);
}

DHTBuckets DHTRoutingTable::getBuckets() const
{
  return BNode::enumerateBucket(_root);
}

void DHTRoutingTable::setTaskQueue(const DHTTaskQueueHandle& taskQueue)
{
  _taskQueue = taskQueue;
}

void DHTRoutingTable::setTaskFactory(const DHTTaskFactoryHandle& taskFactory)
{
  _taskFactory = taskFactory;
}
