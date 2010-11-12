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
#include "BNode.h"

#include <functional>
#include <algorithm>

#include "DHTBucket.h"
#include "DHTNode.h"

namespace aria2 {

BNode::BNode(const SharedHandle<DHTBucket>& bucket):
  bucket_(bucket),
  up_(0),
  left_(0),
  right_(0) {}

BNode::~BNode()
{
  delete left_;
  delete right_;
}

void BNode::setLeft(BNode* left)
{
  left_ = left;
  left_->up_ = this;
}

void BNode::setRight(BNode* right)
{
  right_ = right;
  right_->up_ = this;
}

void BNode::setUp(BNode* up)
{
  up_ = up;
}

void BNode::setBucket(const SharedHandle<DHTBucket>& bucket)
{
  bucket_ = bucket;
}

bool BNode::isInRange(const unsigned char* key) const
{
  if(!bucket_) {
    return left_->isInRange(key) || right_->isInRange(key);
  } else {
    return bucket_->isInRange(key);
  }
}

BNode* BNode::findBNodeFor(BNode* b, const unsigned char* key)
{
  if(!b->isInRange(key)) {
    return 0;
  }
  while(1) {
    if(b->getBucket()) {
      return b;
    }
    // we assume key fits in either left or right bucket range.
    if(b->getLeft()->isInRange(key)) {
      b = b->getLeft();
    } else {
      b = b->getRight();
    }
  }
  // for properly configured BNode tree, here is unreachable.
  return 0;
}

SharedHandle<DHTBucket> BNode::findBucketFor(BNode* b, const unsigned char* key)
{
  BNode* bnode = findBNodeFor(b, key);
  if(bnode) {
    return bnode->getBucket();
  } else {
    return SharedHandle<DHTBucket>();
  }
}


void BNode::findClosestKNodes(std::vector<SharedHandle<DHTNode> >& nodes,
                              BNode* b, const unsigned char* key)
{
  BNode* bnode = findBNodeFor(b, key);
  if(!bnode) {
    return;
  }
  {
    SharedHandle<DHTBucket> bucket = bnode->getBucket();
    bucket->getGoodNodes(nodes);
  }
  if(nodes.size() >= DHTBucket::K) {
    return;
  }
  std::vector<const BNode*> visited;
  visited.push_back(bnode);

  BNode* up = bnode->getUp();
  if(!up) {
    return;
  }
  bool leftFirst = false;
  if(up->getLeft() == bnode) {
    leftFirst = true;
  }
  bnode = up;

  std::const_mem_fun_t<BNode*, BNode> firstfunc = leftFirst?std::mem_fun(&BNode::getLeft):std::mem_fun(&BNode::getRight);
  std::const_mem_fun_t<BNode*, BNode> secondfunc = leftFirst?std::mem_fun(&BNode::getRight):std::mem_fun(&BNode::getLeft);
  while(nodes.size() < DHTBucket::K) {
    
    if(!bnode->getLeft() && !bnode->getRight()) {
      bnode = bnode->getUp();
    } else {
      if(std::find(visited.begin(), visited.end(), firstfunc(bnode)) == visited.end()) {
        bnode = firstfunc(bnode);
      } else if(std::find(visited.begin(), visited.end(), secondfunc(bnode)) == visited.end()) {
        bnode = secondfunc(bnode);
      } else {
        bnode = bnode->getUp();
      }
    }
    if(!bnode) {
      break;
    }
    visited.push_back(bnode);
    {
      SharedHandle<DHTBucket> bucket = bnode->getBucket();
      if(bucket) {
        std::vector<SharedHandle<DHTNode> > goodNodes;
        bucket->getGoodNodes(goodNodes);
        size_t r = DHTBucket::K-nodes.size();
        if(goodNodes.size() <= r) {
          nodes.insert(nodes.end(), goodNodes.begin(), goodNodes.end());
        } else {
          nodes.insert(nodes.end(), goodNodes.begin(), goodNodes.begin()+r);
        }
      }
    }
  }
}

void BNode::enumerateBucket(std::vector<SharedHandle<DHTBucket> >& buckets,
                            const BNode* b)
{
  std::vector<const BNode*> visited;
  visited.push_back(b);
  while(1) {
    if(!b) {
      break;
    }
    if(b->getBucket()) {
      buckets.push_back(b->getBucket());
      b = b->getUp();
    } else if(std::find(visited.begin(), visited.end(), b->getLeft()) == visited.end()) {
      b = b->getLeft();
      visited.push_back(b);
    } else if(std::find(visited.begin(), visited.end(), b->getRight()) == visited.end()) {
      b = b->getRight();
      visited.push_back(b);
    } else {
      b = b->getUp();
    }
  }
  return;
}

} // namespace aria2
