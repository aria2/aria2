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

#include <deque>
#include <map>
#include <vector>
#include <algorithm>

namespace aria2 {

enum A2_HOW {
  A2_POS_SET,
  A2_POS_CUR,
  A2_POS_END
};

template<typename KeyType, typename ValuePtrType>
class IndexedList {
public:
  IndexedList() {}
  ~IndexedList() {}

  typedef std::deque<std::pair<KeyType, ValuePtrType> > SeqType;
  typedef std::map<KeyType, ValuePtrType> IndexType;

  // Inserts (|key|, |value|) to the end of the list. If the same key
  // has been already added, this function fails. This function
  // returns true if it succeeds. Complexity: O(logN)
  bool push_back(KeyType key, ValuePtrType value)
  {
    typename IndexType::iterator i = index_.lower_bound(key);
    if(i == index_.end() || (*i).first != key) {
      std::pair<KeyType, ValuePtrType> p(key, value);
      seq_.push_back(p);
      index_.insert(i, p);
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
      std::pair<KeyType, ValuePtrType> p(key, value);
      seq_.push_front(p);
      index_.insert(i, p);
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
      std::pair<KeyType, ValuePtrType> p(key, value);
      j = seq_.insert(j, p);
      index_.insert(i, p);
      return j;
    } else {
      return seq_.end();
    }
  }

  // Inserts (|key|, |value|) to the position |dest|. If the same key
  // has been already added, this function fails. This function
  // returns the iterator to the newly added element if it is
  // succeeds, or end(). Complexity: O(logN) if inserted to the first
  // or last, otherwise O(N)
  typename SeqType::iterator insert(typename SeqType::iterator dest,
                                    KeyType key,
                                    ValuePtrType value)
  {
    typename IndexType::iterator i = index_.lower_bound(key);
    if(i == index_.end() || (*i).first != key) {
      std::pair<KeyType, ValuePtrType> p(key, value);
      dest = seq_.insert(dest, p);
      index_.insert(i, p);
      return dest;
    } else {
      return seq_.end();
    }
  }

  // Inserts values in iterator range [first, last). The key for each
  // value is retrieved by functor |keyFunc|. The insertion position
  // is given by |dest|.
  template<typename KeyFunc, typename InputIterator>
  void insert(typename SeqType::iterator dest, KeyFunc keyFunc,
              InputIterator first, InputIterator last)
  {
    std::vector<typename SeqType::value_type> v;
    v.reserve(std::distance(first, last));
    for(; first != last; ++first) {
      KeyType key = keyFunc(*first);
      typename IndexType::iterator i = index_.lower_bound(key);
      if(i == index_.end() || (*i).first != key) {
        std::pair<KeyType, ValuePtrType> p(key, *first);
        v.push_back(p);
        index_.insert(i, p);
      }
    }
    seq_.insert(dest, v.begin(), v.end());
  }

  // Removes |key| from the list. If the element is not found, this
  // function fails. This function returns true if it
  // succeeds. Complexity: O(N)
  bool erase(KeyType key)
  {
    typename IndexType::iterator i = index_.find(key);
    if(i == index_.end()) {
      return false;
    }
    index_.erase(i);
    for(typename SeqType::iterator j = seq_.begin(), eoj = seq_.end();
        j != eoj; ++j) {
      if((*j).first == key) {
        seq_.erase(j);
        break;
      }
    }
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
    typename SeqType::iterator x = seq_.begin(), eseq = seq_.end();
    for(; x != eseq; ++x) {
      if((*x).first == key) {
        break;
      }
    }
    ssize_t xp = std::distance(seq_.begin(), x);
    ssize_t size = index_.size();
    ssize_t dest;
    if(how == A2_POS_CUR) {
      if(offset > 0) {
        dest = std::min(xp+offset, static_cast<ssize_t>(size-1));
      } else {
        dest = std::max(xp+offset, static_cast<ssize_t>(0));
      }
    } else {
      if(how == A2_POS_END) {
        dest = std::min(size-1+offset, size-1);
      } else if(how == A2_POS_SET) {
        dest = std::min(offset, size-1);
      } else {
        return -1;
      }
      dest = std::max(dest, static_cast<ssize_t>(0));
    }
    typename SeqType::iterator d = seq_.begin();
    std::advance(d, dest);
    if(xp < dest) {
      std::rotate(x, x+1, d+1);
    } else {
      std::rotate(d, x, x+1);
    }
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
