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
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <aria2/aria2.h>

namespace aria2 {

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
struct IndexedListIterator {
  typedef IndexedListIterator<SeqType, ValueType, ValueType&, ValueType*,
                              typename SeqType::iterator>
      iterator;
  typedef IndexedListIterator<SeqType, ValueType, const ValueType&,
                              const ValueType*,
                              typename SeqType::const_iterator>
      const_iterator;

  typedef typename SeqIteratorType::iterator_category iterator_category;
  typedef ValueType value_type;
  typedef PointerType pointer;
  typedef ReferenceType reference;
  typedef typename SeqType::size_type size_type;
  typedef typename SeqType::difference_type difference_type;
  typedef IndexedListIterator SelfType;

  IndexedListIterator() = default;
  IndexedListIterator(const iterator& other) : p(other.p) {}
  IndexedListIterator(const SeqIteratorType& p) : p(p) {}

  reference operator*() const { return (*p).second; }

  pointer operator->() const { return &(*p).second; }

  SelfType& operator++()
  {
    ++p;
    return *this;
  }

  SelfType operator++(int)
  {
    SelfType copy = *this;
    ++*this;
    return copy;
  }

  SelfType& operator--()
  {
    --p;
    return *this;
  }

  SelfType operator--(int)
  {
    SelfType copy = *this;
    --*this;
    return copy;
  }

  SelfType& operator+=(difference_type n)
  {
    std::advance(p, n);
    return *this;
  }

  SelfType operator+(difference_type n) const
  {
    SelfType copy = *this;
    return copy += n;
  }

  SelfType& operator-=(difference_type n)
  {
    std::advance(p, -n);
    return *this;
  }

  SelfType operator-(difference_type n) const
  {
    SelfType copy = *this;
    return copy -= n;
  }

  reference operator[](size_type n) const { return p[n].second; }

  SeqIteratorType p;
};

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
bool operator==(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& rhs)
{
  return lhs.p == rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
bool operator==(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                          PointerTypeL, SeqIteratorTypeL>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                          PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return lhs.p == rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
bool operator!=(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& rhs)
{
  return lhs.p != rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
bool operator!=(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                          PointerTypeL, SeqIteratorTypeL>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                          PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return lhs.p != rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
bool operator<(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                         PointerType, SeqIteratorType>& lhs,
               const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                         PointerType, SeqIteratorType>& rhs)
{
  return lhs.p < rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
bool operator<(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                         PointerTypeL, SeqIteratorTypeL>& lhs,
               const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                         PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return lhs.p < rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
bool operator>(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                         PointerType, SeqIteratorType>& lhs,
               const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                         PointerType, SeqIteratorType>& rhs)
{
  return lhs.p > rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
bool operator>(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                         PointerTypeL, SeqIteratorTypeL>& lhs,
               const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                         PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return lhs.p > rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
bool operator<=(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& rhs)
{
  return lhs.p <= rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
bool operator<=(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                          PointerTypeL, SeqIteratorTypeL>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                          PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return lhs.p <= rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
bool operator>=(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                          PointerType, SeqIteratorType>& rhs)
{
  return lhs.p >= rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
bool operator>=(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                          PointerTypeL, SeqIteratorTypeL>& lhs,
                const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                          PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return lhs.p >= rhs.p;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
IndexedListIterator<SeqType, ValueType, ReferenceType, PointerType,
                    SeqIteratorType>
operator+(
    typename IndexedListIterator<SeqType, ValueType, ReferenceType, PointerType,
                                 SeqIteratorType>::difference_type n,
    const IndexedListIterator<SeqType, ValueType, ReferenceType, PointerType,
                              SeqIteratorType>& lhs)
{
  return lhs + n;
}

template <typename SeqType, typename ValueType, typename ReferenceType,
          typename PointerType, typename SeqIteratorType>
typename IndexedListIterator<SeqType, ValueType, ReferenceType, PointerType,
                             SeqIteratorType>::difference_type
operator-(const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                    PointerType, SeqIteratorType>& lhs,
          const IndexedListIterator<SeqType, ValueType, ReferenceType,
                                    PointerType, SeqIteratorType>& rhs)
{
  return typename IndexedListIterator<SeqType, ValueType, ReferenceType,
                                      PointerType,
                                      SeqIteratorType>::difference_type(lhs.p -
                                                                        rhs.p);
}

template <typename SeqType, typename ValueType, typename ReferenceTypeL,
          typename PointerTypeL, typename SeqIteratorTypeL,
          typename ReferenceTypeR, typename PointerTypeR,
          typename SeqIteratorTypeR>
typename IndexedListIterator<SeqType, ValueType, ReferenceTypeL, PointerTypeL,
                             SeqIteratorTypeL>::difference_type
operator-(const IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                    PointerTypeL, SeqIteratorTypeL>& lhs,
          const IndexedListIterator<SeqType, ValueType, ReferenceTypeR,
                                    PointerTypeR, SeqIteratorTypeR>& rhs)
{
  return typename IndexedListIterator<SeqType, ValueType, ReferenceTypeL,
                                      PointerTypeL,
                                      SeqIteratorTypeL>::difference_type(lhs.p -
                                                                         rhs.p);
}

template <typename KeyType, typename ValuePtrType> class IndexedList {
public:
  IndexedList() = default;
  ~IndexedList() = default;

  typedef KeyType key_type;
  typedef ValuePtrType value_type;
  typedef std::unordered_map<KeyType, ValuePtrType> IndexType;
  typedef std::deque<std::pair<KeyType, ValuePtrType>> SeqType;

  typedef IndexedListIterator<SeqType, ValuePtrType, ValuePtrType&,
                              ValuePtrType*, typename SeqType::iterator>
      iterator;
  typedef IndexedListIterator<SeqType, ValuePtrType, const ValuePtrType&,
                              const ValuePtrType*,
                              typename SeqType::const_iterator>
      const_iterator;

  ValuePtrType& operator[](size_t n) { return seq_[n].second; }

  const ValuePtrType& operator[](size_t n) const { return seq_[n].second; }

  // Inserts (|key|, |value|) to the end of the list. If the same key
  // has been already added, this function fails. This function
  // returns true if it succeeds. Complexity: O(1)
  bool push_back(KeyType key, ValuePtrType value)
  {
    auto i = index_.find(key);
    if (i == std::end(index_)) {
      index_.insert({key, value});
      seq_.emplace_back(key, value);
      return true;
    }
    else {
      return false;
    }
  }

  // Inserts (|key|, |value|) to the front of the list. If the same
  // key has been already added, this function fails. This function
  // returns true if it succeeds. Complexity: O(1)
  bool push_front(KeyType key, ValuePtrType value)
  {
    auto i = index_.find(key);
    if (i == std::end(index_)) {
      index_.insert({key, value});
      seq_.emplace_front(key, value);
      return true;
    }
    else {
      return false;
    }
  }

  // Inserts (|key|, |value|) to the position |dest|. If the same key
  // has been already added, this function fails. This function
  // returns the iterator to the newly added element if it is
  // succeeds, or end(). Complexity: O(N)
  iterator insert(size_t dest, KeyType key, ValuePtrType value)
  {
    if (dest > size()) {
      return std::end(seq_);
    }
    auto i = index_.find(key);
    if (i == std::end(index_)) {
      auto j = std::begin(seq_);
      std::advance(j, dest);
      index_.insert({key, value});
      return iterator(seq_.insert(j, {key, value}));
    }
    else {
      return iterator(std::end(seq_));
    }
  }

  // Inserts (|key|, |value|) to the position |dest|. If the same key
  // has been already added, this function fails. This function
  // returns the iterator to the newly added element if it is
  // succeeds, or end(). Complexity: O(1) if inserted to the first or
  // last, otherwise O(N)
  iterator insert(iterator dest, KeyType key, ValuePtrType value)
  {
    auto i = index_.find(key);
    if (i == std::end(index_)) {
      index_.insert({key, value});
      return iterator(seq_.insert(dest.p, {key, value}));
    }
    else {
      return iterator(std::end(seq_));
    }
  }

  // Inserts values in iterator range [first, last). The key for each
  // value is retrieved by functor |keyFunc|. The insertion position
  // is given by |dest|.
  template <typename KeyFunc, typename InputIterator>
  void insert(iterator dest, KeyFunc keyFunc, InputIterator first,
              InputIterator last)
  {
    std::vector<typename SeqType::value_type> v;
    v.reserve(std::distance(first, last));
    for (; first != last; ++first) {
      auto key = keyFunc(*first);
      auto i = index_.find(key);
      if (i == std::end(index_)) {
        index_.insert({key, *first});
        v.emplace_back(key, *first);
      }
    }
    seq_.insert(dest.p, std::begin(v), std::end(v));
  }

  template <typename KeyFunc, typename InputIterator>
  void insert(size_t pos, KeyFunc keyFunc, InputIterator first,
              InputIterator last)
  {
    if (pos > size()) {
      return;
    }
    std::vector<typename SeqType::value_type> v;
    v.reserve(std::distance(first, last));
    for (; first != last; ++first) {
      auto key = keyFunc(*first);
      auto i = index_.find(key);
      if (i == std::end(index_)) {
        index_.insert({key, *first});
        v.emplace_back(key, *first);
      }
    }
    seq_.insert(std::begin(seq_) + pos, std::begin(v), std::end(v));
  }

  // Removes |key| from the list. If the element is not found, this
  // function fails. This function returns true if it
  // succeeds. Complexity: O(N)
  bool remove(KeyType key)
  {
    auto i = index_.find(key);
    if (i == std::end(index_)) {
      return false;
    }
    for (auto j = std::begin(seq_), eoj = std::end(seq_); j != eoj; ++j) {
      if ((*j).first == key) {
        seq_.erase(j);
        break;
      }
    }
    index_.erase(i);
    return true;
  }

  // Removes element pointed by iterator |k| from the list. If the
  // iterator must be valid. This function returns the iterator
  // pointing to the element following the erased element. Complexity:
  // O(N)
  iterator erase(iterator k)
  {
    index_.erase((*k.p).first);
    return iterator(seq_.erase(k.p));
  }

  // Removes elements for which Pred returns true. The pred is called
  // against each each element once per each.
  template <typename Pred> void remove_if(Pred pred)
  {
    auto first = std::begin(seq_), last = std::end(seq_);
    for (; first != last && !pred((*first).second); ++first)
      ;
    if (first == last) {
      return;
    }
    index_.erase((*first).first);
    auto store = first;
    ++first;
    for (; first != last; ++first) {
      if (pred((*first).second)) {
        index_.erase((*first).first);
      }
      else {
        *store++ = *first;
      }
    }
    seq_.erase(store, last);
  }

  // Removes element at the front of the list. If the list is empty,
  // this function fails. This function returns true if it
  // succeeds. Complexity: O(1)
  bool pop_front()
  {
    if (seq_.empty()) {
      return false;
    }
    index_.erase(seq_.front().first);
    seq_.pop_front();
    return true;
  }

  // Moves element with |key| to the specified position. If |how| is
  // OFFSET_MODE_CUR, the element is moved to the position |offset|
  // relative to the current position. If |how| is OFFSET_MODE_SET,
  // the element is moved to the position |offset|. If |how| is
  // OFFSET_MODE_END, the element is moved to the position |offset|
  // relative to the end of the list.  This function returns the
  // position the element is moved to if it succeeds, or -1 if no
  // element with |key| is found or |how| is invalid.  Complexity:
  // O(N)
  ssize_t move(KeyType key, ssize_t offset, OffsetMode how)
  {
    auto idxent = index_.find(key);
    if (idxent == std::end(index_)) {
      return -1;
    }
    auto x = std::begin(seq_), eseq = std::end(seq_);
    for (; x != eseq; ++x) {
      if ((*x).first == (*idxent).first) {
        break;
      }
    }
    ssize_t xp = std::distance(std::begin(seq_), x);
    ssize_t size = index_.size();
    ssize_t dest;
    if (how == OFFSET_MODE_CUR) {
      if (offset > 0) {
        dest = std::min(xp + offset, static_cast<ssize_t>(size - 1));
      }
      else {
        dest = std::max(xp + offset, static_cast<ssize_t>(0));
      }
    }
    else {
      if (how == OFFSET_MODE_END) {
        dest = std::min(size - 1 + offset, size - 1);
      }
      else if (how == OFFSET_MODE_SET) {
        dest = std::min(offset, size - 1);
      }
      else {
        return -1;
      }
      dest = std::max(dest, static_cast<ssize_t>(0));
    }
    auto d = std::begin(seq_);
    std::advance(d, dest);
    if (xp < dest) {
      std::rotate(x, x + 1, d + 1);
    }
    else {
      std::rotate(d, x, x + 1);
    }
    return dest;
  }

  // Returns the value associated by |key|. If it is not found,
  // returns ValuePtrType().  Complexity: O(1)
  ValuePtrType get(KeyType key) const
  {
    auto idxent = index_.find(key);
    if (idxent == std::end(index_)) {
      return ValuePtrType();
    }
    else {
      return (*idxent).second;
    }
  }

  size_t size() const { return index_.size(); }

  size_t empty() const { return index_.empty(); }

  iterator begin() { return iterator(std::begin(seq_)); }

  iterator end() { return iterator(std::end(seq_)); }

  const_iterator begin() const { return const_iterator(std::begin(seq_)); }

  const_iterator end() const { return const_iterator(std::end(seq_)); }

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
