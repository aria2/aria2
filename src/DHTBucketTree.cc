/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "DHTBucketTree.h"

#include <cstring>
#include <algorithm>

#include "DHTBucket.h"
#include "DHTNode.h"

namespace aria2 {

DHTBucketTreeNode::DHTBucketTreeNode
(DHTBucketTreeNode* left,
 DHTBucketTreeNode* right)
  : parent_(0),
    left_(left),
    right_(right)
{
  resetRelation();
}

DHTBucketTreeNode::DHTBucketTreeNode(const SharedHandle<DHTBucket>& bucket)
  : parent_(0),
    left_(0),
    right_(0),
    bucket_(bucket)
{
  memcpy(minId_, bucket_->getMinID(), DHT_ID_LENGTH);
  memcpy(maxId_, bucket_->getMaxID(), DHT_ID_LENGTH);
}

DHTBucketTreeNode::~DHTBucketTreeNode()
{
  delete left_;
  delete right_;
}

void DHTBucketTreeNode::resetRelation()
{
  left_->setParent(this);
  right_->setParent(this);
  memcpy(minId_, left_->getMinId(), DHT_ID_LENGTH);
  memcpy(maxId_, right_->getMaxId(), DHT_ID_LENGTH);
}

DHTBucketTreeNode* DHTBucketTreeNode::dig(const unsigned char* key)
{
  if(leaf()) {
    return 0;
  }
  if(left_->isInRange(key)) {
    return left_;
  } else {
    return right_;
  }
}

bool DHTBucketTreeNode::isInRange(const unsigned char* key) const
{
  return
    !std::lexicographical_compare(&key[0], &key[DHT_ID_LENGTH],
                                  &minId_[0], &minId_[DHT_ID_LENGTH]) &&
    !std::lexicographical_compare(&maxId_[0], &maxId_[DHT_ID_LENGTH],
                                  &key[0], &key[DHT_ID_LENGTH]);
}

void DHTBucketTreeNode::split()
{
  SharedHandle<DHTBucket> leftBucket = bucket_->split();
  left_ = new DHTBucketTreeNode(leftBucket);
  right_ = new DHTBucketTreeNode(bucket_);
  bucket_.reset();
  resetRelation();
}

namespace dht {

DHTBucketTreeNode* findTreeNodeFor
(DHTBucketTreeNode* root, const unsigned char* key)
{
  if(root->leaf()) {
    return root;
  } else {
    return findTreeNodeFor(root->dig(key), key);
  }
}

SharedHandle<DHTBucket> findBucketFor
(DHTBucketTreeNode* root, const unsigned char* key)
{
  DHTBucketTreeNode* leaf = findTreeNodeFor(root, key);
  return leaf->getBucket();
}

namespace {
void collectNodes
(std::vector<SharedHandle<DHTNode> >& nodes,
 const SharedHandle<DHTBucket>& bucket)
{
  std::vector<SharedHandle<DHTNode> > goodNodes;
  bucket->getGoodNodes(goodNodes);
  nodes.insert(nodes.end(), goodNodes.begin(), goodNodes.end());
}
} // namespace

namespace {
void collectDownwardLeftFirst
(std::vector<SharedHandle<DHTNode> >& nodes,  DHTBucketTreeNode* tnode)
{
  if(tnode->leaf()) {
    collectNodes(nodes, tnode->getBucket());
  } else {
    collectDownwardLeftFirst(nodes, tnode->getLeft());
    if(nodes.size() < DHTBucket::K) {
      collectDownwardLeftFirst(nodes, tnode->getRight());
    }
  }
}
} //namespace

namespace {
void collectDownwardRightFirst
(std::vector<SharedHandle<DHTNode> >& nodes,  DHTBucketTreeNode* tnode)
{
  if(tnode->leaf()) {
    collectNodes(nodes, tnode->getBucket());
  } else {
    collectDownwardRightFirst(nodes, tnode->getRight());
    if(nodes.size() < DHTBucket::K) {
      collectDownwardRightFirst(nodes, tnode->getLeft());
    }
  }
}
} //namespace

namespace {
void collectUpward
(std::vector<SharedHandle<DHTNode> >& nodes, DHTBucketTreeNode* from)
{
  while(1) {
    DHTBucketTreeNode* parent = from->getParent();
    if(!parent) {
      break;
    }
    if(parent->getLeft() == from) {
      collectNodes(nodes, parent->getRight()->getBucket());
    } else {
      collectNodes(nodes, parent->getLeft()->getBucket());
    }
    from = parent;
    parent = parent->getParent();
    if(DHTBucket::K <= nodes.size()) {
      break;
    }
  }
}
} // namespace

void findClosestKNodes
(std::vector<SharedHandle<DHTNode> >& nodes,
 DHTBucketTreeNode* root,
 const unsigned char* key)
{
  size_t nodesSize = nodes.size();
  if(DHTBucket::K <= nodesSize) {
    return;
  }
  DHTBucketTreeNode* leaf = findTreeNodeFor(root, key);
  if(leaf == root) {
    collectNodes(nodes, leaf->getBucket());
  } else {
    DHTBucketTreeNode* parent = leaf->getParent();
    if(parent->getLeft() == leaf) {
      collectDownwardLeftFirst(nodes, parent);
    } else {
      collectDownwardRightFirst(nodes, parent);
    }
    if(nodes.size() < DHTBucket::K) {
      collectUpward(nodes, parent);
    }
  }
  if(DHTBucket::K < nodes.size()) {
    nodes.erase(nodes.begin()+DHTBucket::K, nodes.end());
  }
}

void enumerateBucket
(std::vector<SharedHandle<DHTBucket> >& buckets,  DHTBucketTreeNode* root)
{
  if(root->leaf()) {
    buckets.push_back(root->getBucket());
  } else {
    enumerateBucket(buckets, root->getLeft());
    enumerateBucket(buckets, root->getRight());
  }
}

} // namespace dht

} // namespace aria2
