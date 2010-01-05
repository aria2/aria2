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
#ifndef _D_BNODE_H_
#define _D_BNODE_H_

#include "common.h"
#include "SharedHandle.h"
#include <deque>

namespace aria2 {

class DHTBucket;
class DHTNode;

class BNode {
private:
  SharedHandle<DHTBucket> _bucket;

  BNode* _up;

  BNode* _left;
  
  BNode* _right;

public:
  BNode(const SharedHandle<DHTBucket>& bucket = SharedHandle<DHTBucket>());

  ~BNode();

  const SharedHandle<DHTBucket>& getBucket() const
  {
    return _bucket;
  }

  void setBucket(const SharedHandle<DHTBucket>& bucket);

  BNode* getLeft() const
  {
    return _left;
  }

  void setLeft(BNode* left);

  BNode* getRight() const
  {
    return _right;
  }

  void setRight(BNode* right);

  BNode* getUp() const
  {
    return _up;
  }

  void setUp(BNode* up);
  
  bool isInRange(const unsigned char* key) const;

  static BNode* findBNodeFor(BNode* b, const unsigned char* key);

  static SharedHandle<DHTBucket> findBucketFor(BNode* b, const unsigned char* key);

  static void findClosestKNodes(std::deque<SharedHandle<DHTNode> >& nodes,
                                BNode* b, const unsigned char* key);

  static void enumerateBucket(std::deque<SharedHandle<DHTBucket> >& buckets,
                              const BNode* b);
};

} // namespace aria2

#endif // _D_BNODE_H_
