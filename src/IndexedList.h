/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_INDEXED_LIST_H
#define D_INDEXED_LIST_H

#include "common.h"

#include <list>
#include <map>

namespace aria2 {

enum A2_HOW {
  A2_POS_SET,
  A2_POS_CUR,
  A2_POS_END
};

// List with O(logN) look-up using std::map as an index.
template<typename KeyType, typename ValuePtrType>
class IndexedList {
public:
  IndexedList() {}
  ~IndexedList() {}

  typedef std::list<std::pair<KeyType, ValuePtrType> > SeqType;
  typedef std::map<KeyType, typename SeqType::iterator> IndexType;

  // Inserts (|key|, |value|) to the end of the list. If the same key
  // has been already added, this function fails. This function
  // returns true if it succeeds. Complexity: O(logN)
  bool push_back(KeyType key, ValuePtrType value)
  {
    typename IndexType::iterator i = index_.lower_bound(key);
    if(i == index_.end() || (*i).first != key) {
      seq_.push_back(std::make_pair(key, value));
      typename SeqType::iterator j = seq_.end();
      --j;
      index_.insert(i, std::make_pair(key, j));
      return true;
    } else {
      return false;
    }
  }

  // Inserts (|key|, |value|) to the front of the list. If the same
  // key has been already added, this function fails. This function
  // returns true if it succeeds. Complexity: O(logN)
  bool push_front(KeyType key, ValuePtrType value)
  {
    typename IndexType::iterator i = index_.lower_bound(key);
    if(i == index_.end() || (*i).first != key) {
      seq_.push_front(std::make_pair(key, value));
      typename SeqType::iterator j = seq_.begin();
      index_.insert(i, std::make_pair(key, j));
      return true;
    } else {
      return false;
    }
  }

  // Inserts (|key|, |value|) to the position |dest|. If the same key
  // has been already added, this function fails. This function
  // returns the iterator to the newly added element if it is
  // succeeds, or end(). Complexity: O(N)
  typename SeqType::iterator insert(size_t dest, KeyType key,
                                    ValuePtrType value)
  {
    if(dest > size()) {
      return seq_.end();
    }
    typename IndexType::iterator i = index_.lower_bound(key);
    if(i == index_.end() || (*i).first != key) {
      typename SeqType::iterator j = seq_.begin();
      std::advance(j, dest);
      j = seq_.insert(j, std::make_pair(key, value));
      index_.insert(i, std::make_pair(key, j));
      return j;
    } else {
      return seq_.end();
    }
  }

  // Inserts (|key|, |value|) to the position |dest|. If the same key
  // has been already added, this function fails. This function
  // returns the iterator to the newly added element if it is
  // succeeds, or end(). Complexity: O(logN)
  typename SeqType::iterator insert(typename SeqType::iterator dest,
                                    KeyType key,
                                    ValuePtrType value)
  {
    typename IndexType::iterator i = index_.lower_bound(key);
    if(i == index_.end() || (*i).first != key) {
      dest = seq_.insert(dest, std::make_pair(key, value));
      index_.insert(i, std::make_pair(key, dest));
      return dest;
    } else {
      return seq_.end();
    }
  }

  // Removes |key| from the list. If the element is not found, this
  // function fails. This function returns true if it
  // succeeds. Complexity: O(logN)
  bool erase(KeyType key)
  {
    typename IndexType::iterator i = index_.find(key);
    if(i == index_.end()) {
      return false;
    }
    seq_.erase((*i).second);
    index_.erase(i);
    return true;
  }

  // Removes element at the front of the list. If the list is empty,
  // this function fails. This function returns true if it
  // succeeds. Complexity: O(logN)
  bool pop_front()
  {
    if(seq_.empty()) {
      return false;
    }
    KeyType key = seq_.front().first;
    index_.erase(key);
    seq_.pop_front();
    return true;
  }

  // Moves element with |key| to the specified position. If |how| is
  // A2_POS_CUR, the element is moved to the position |offset|
  // relative to the current position. If |how| is A2_POS_SET, the
  // element is moved to the position |offset|. If |how| is
  // A2_POS_END, the element is moved to the position |offset|
  // relative to the end of the list.  This function returns the
  // position the elment is moved to if it succeeds, or -1 if no
  // element with |key| is found or |how| is invalid.  Complexity:
  // O(N)
  ssize_t move(KeyType key, ssize_t offset, A2_HOW how)
  {
    typename IndexType::iterator idxent = index_.find(key);
    if(idxent == index_.end()) {
      return -1;
    }
    ssize_t dest;
    typename SeqType::iterator x = (*idxent).second;
    typename SeqType::iterator d;
    if(how == A2_POS_CUR) {
      // Because aria2.changePosition() RPC method must return the
      // absolute position after move, we have to calculate absolute
      // position here.
      if(offset > 0) {
        d = x;
        for(; offset >= 0 && d != seq_.end(); --offset, ++d);
        dest = std::distance(seq_.begin(), d)-1;
      } else {
        d = x;
        for(; offset < 0 && d != seq_.begin(); ++offset, --d);
        dest = std::distance(seq_.begin(), d);
      }
    } else {
      ssize_t size = index_.size();
      if(how == A2_POS_END) {
        dest = std::min(size-1, size-1+offset);
      } else if(how == A2_POS_SET) {
        dest = std::min(size-1, offset);
      } else {
        return -1;
      }
      dest = std::max(dest, static_cast<ssize_t>(0));
      d = seq_.begin();
      for(ssize_t i = 0; i < dest; ++i, ++d) {
        if(d == x) {
          ++d;
        }
      }
    }
    seq_.splice(d, seq_, x);
    return dest;
  }

  // Returns the value associated by |key|. If it is not found,
  // returns ValuePtrType().  Complexity: O(logN)
  ValuePtrType get(KeyType key) const
  {
    typename IndexType::const_iterator idxent = index_.find(key);
    if(idxent == index_.end()) {
      return ValuePtrType();
    } else {
      return (*(*idxent).second).second;
    }
  }

  // Returns the iterator to the element associated by |key|. If it is
  // not found, end() is returned. Complexity: O(logN)
  typename SeqType::iterator find(KeyType key)
  {
    typename IndexType::iterator idxent = index_.find(key);
    if(idxent == index_.end()) {
      return seq_.end();
    } else {
      return (*idxent).second;
    }
  }

  // Returns the iterator to the element associated by |key|. If it is
  // not found, end() is returned. Complexity: O(logN)
  typename SeqType::const_iterator find(KeyType key) const
  {
    typename IndexType::const_iterator idxent = index_.find(key);
    if(idxent == index_.end()) {
      return seq_.end();
    } else {
      return (*idxent).second;
    }
  }

  size_t size() const
  {
    return index_.size();
  }

  size_t empty() const
  {
    return index_.empty();
  }

  typename SeqType::iterator begin()
  {
    return seq_.begin();
  }

  typename SeqType::iterator end()
  {
    return seq_.end();
  }

  typename SeqType::const_iterator begin() const
  {
    return seq_.begin();
  }

  typename SeqType::const_iterator end() const
  {
    return seq_.end();
  }

  // Removes all elements from the list.
  void clear()
  {
    index_.clear();
    seq_.clear();
  }
private:
  SeqType seq_;
  IndexType index_;
};

} // namespace aria2

#endif // D_INDEXED_LIST_H
