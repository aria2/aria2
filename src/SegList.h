/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#ifndef D_SEG_LIST_H
#define D_SEG_LIST_H

#include <cstdlib>
#include <vector>
#include <limits>
#include <algorithm>

namespace aria2 {

template<typename T>
class SegList {
public:
  SegList()
    : index_(0), val_(std::numeric_limits<T>::min())
  {}

  void clear()
  {
    segs_.clear();
    index_ = 0;
    val_ = std::numeric_limits<T>::min();
  }

  // Transforms list of segments so that they are sorted ascending
  // order of starting value and intersecting and touching segments
  // are all merged into one. This function resets current position.
  void normalize()
  {
    if(!segs_.empty()) {
      std::sort(segs_.begin(), segs_.end());
      std::vector<std::pair<T, T> > s;
      s.push_back(segs_.front());
      for(size_t i = 1, len = segs_.size(); i < len; ++i) {
        const std::pair<T, T>& x = segs_[i];
        if(x.first <= s.back().second) {
          if(s.back().second < x.second) {
            s.back().second = x.second;
          }
        } else {
          s.push_back(x);
        }
      }
      s.swap(segs_);
      index_ = 0;
      val_ = segs_.front().first;
    }
  }

  // Add segment [a, b). If a >= b, do nothing.
  void add(T a, T b)
  {
    if(a < b) {
      if(segs_.empty()) {
        val_ = std::max(val_, a);
      }
      segs_.push_back(std::make_pair(a, b));
    }
  }

  // Returns true if next value is available. Otherwise returns false.
  bool hasNext() const
  {
    return index_ < segs_.size() && val_ < segs_[index_].second;
  }

  // Returns next value. Advance current position to the next.  If
  // this fuction is called when hasNext() returns false, returns 0.
  T next()
  {
    T res;
    size_t len = segs_.size();
    if(index_ < len) {
      res = val_++;
      if(val_ == segs_[index_].second) {
        ++index_;
        if(index_ < len) {
          val_ = segs_[index_].first;
        }
      }
    } else {
      res = 0;
    }
    return res;
  }

  // Returns next value. Current position is not advanced.  If
  // this fuction is called when hasNext() returns false, returns 0.
  T peek() const
  {
    T res;
    if(index_ < segs_.size()) {
      res = val_;
    } else {
      res = 0;
    }
    return res;
  }
private:
  std::vector<std::pair<T, T> > segs_;
  size_t index_;
  T val_;
  // Don't allow copying
  SegList(const SegList<T>&);
  SegList& operator=(const SegList<T>&);
};

} // namespace aria2

#endif // D_SEG_LIST_H
