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
#ifndef D_DHT_BUCKET_TREE_H
#define D_DHT_BUCKET_TREE_H

#include "common.h"

#include <vector>

#include "SharedHandle.h"
#include "DHTConstants.h"

namespace aria2 {

class DHTBucket;
class DHTNode;

// This class represents Kademlia DHT routing tree.  The leaf nodes
// have bucket which contains DHT node.  The tree is binary tree but
// highly unbalanced.
class DHTBucketTreeNode {
public:
  // Ctor for internal node
  DHTBucketTreeNode(DHTBucketTreeNode* left, DHTBucketTreeNode* right);
  // Ctor for leaf node
  DHTBucketTreeNode(const SharedHandle<DHTBucket>& bucket);
  ~DHTBucketTreeNode();
  // Returns child node, left or right, which contains key.  If dig is
  // called against leaf node, then returns 0.
  DHTBucketTreeNode* dig(const unsigned char* key);
  bool isInRange(const unsigned char* key) const;
  // Returns true iff this is a leaf node.
  bool leaf() const { return bucket_; }
  const unsigned char* getMaxId() const { return maxId_; }
  const unsigned char* getMinId() const { return minId_; }
  DHTBucketTreeNode* getParent() const { return parent_; }
  DHTBucketTreeNode* getLeft() const { return left_; }
  DHTBucketTreeNode* getRight() const { return right_; }
  const SharedHandle<DHTBucket>& getBucket() const { return bucket_; }
  // Splits this object's bucket using DHTBucket::split() and create
  // left and right child node to hold buckets. The bucket of current
  // node is reseted so this node becomes internal node after this
  // call.
  void split();
private:
  // Do not allow copying
  DHTBucketTreeNode(const DHTBucketTreeNode&);
  DHTBucketTreeNode& operator=(const DHTBucketTreeNode&);

  // Reset relation of children and minId_ and maxId_.
  void resetRelation();
  void setParent(DHTBucketTreeNode* parent) { parent_ = parent; }
  DHTBucketTreeNode* parent_;
  DHTBucketTreeNode* left_;
  DHTBucketTreeNode* right_;
  SharedHandle<DHTBucket> bucket_;
  unsigned char minId_[DHT_ID_LENGTH];
  unsigned char maxId_[DHT_ID_LENGTH];
};

namespace dht {

// Returns leaf node where key fits between node's min and max ID
// range.
DHTBucketTreeNode* findTreeNodeFor
(DHTBucketTreeNode* root, const unsigned char* key);

// Returns bucket where key fits between bucket's min and max ID
// range. This function first use findTreeNodeFor and returns its
// bucket_.
SharedHandle<DHTBucket> findBucketFor
(DHTBucketTreeNode* root, const unsigned char* key);

// Stores most closest K nodes against key in nodes. K is
// DHTBucket::K. This function may returns less than K nodes because
// the routing tree contains less than K nodes. The order of nodes is
// arbitrary.  Caller must pass empty nodes.
void findClosestKNodes
(std::vector<SharedHandle<DHTNode> >& nodes,
 DHTBucketTreeNode* root,
 const unsigned char* key);

// Stores all buckets in buckets.
void enumerateBucket
(std::vector<SharedHandle<DHTBucket> >& buckets, DHTBucketTreeNode* root);

} // namespace dht

} // namespace aria2

#endif // D_DHT_BUCKET_TREE_H
