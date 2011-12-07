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
#ifndef D_REQUEST_SLOT_H
#define D_REQUEST_SLOT_H

#include "common.h"
#include "TimerA2.h"
#include "Piece.h"
#include "wallclock.h"

namespace aria2 {

class RequestSlot {
private:
  Timer dispatchedTime_;
  size_t index_;
  int32_t begin_;
  int32_t length_;
  size_t blockIndex_;

  // This is the piece whose index is index of this RequestSlot has.
  // To detect duplicate RequestSlot, we have to find the piece using
  // PieceStorage::getPiece() repeatedly. It turns out that this process
  // takes time(about 1.7% of processing time). To reduce it, we put piece here
  // at the construction of RequestSlot as a cache.
  SharedHandle<Piece> piece_;

  // inlined for performance reason
  void copy(const RequestSlot& requestSlot)
  {
    dispatchedTime_ = requestSlot.dispatchedTime_;
    index_ = requestSlot.index_;
    begin_ = requestSlot.begin_;
    length_ = requestSlot.length_;
    blockIndex_ = requestSlot.blockIndex_;
    piece_ = requestSlot.piece_;
  }
public:
  
  RequestSlot(size_t index, int32_t begin, int32_t length, size_t blockIndex,
              const SharedHandle<Piece>& piece = SharedHandle<Piece>()):
    dispatchedTime_(global::wallclock()),
    index_(index), begin_(begin), length_(length), blockIndex_(blockIndex),
    piece_(piece) {}

  RequestSlot(const RequestSlot& requestSlot):
    dispatchedTime_(requestSlot.dispatchedTime_),
    index_(requestSlot.index_),
    begin_(requestSlot.begin_),
    length_(requestSlot.length_),
    blockIndex_(requestSlot.blockIndex_),
    piece_(requestSlot.piece_) {}

  RequestSlot():dispatchedTime_(0), index_(0), begin_(0), length_(0),
                blockIndex_(0)
  {}

  ~RequestSlot() {}

  RequestSlot& operator=(const RequestSlot& requestSlot)
  {
    if(this != &requestSlot) {
      copy(requestSlot);
    }
    return *this;
  }

  bool operator==(const RequestSlot& requestSlot) const
  {
    return index_ == requestSlot.index_ && begin_ == requestSlot.begin_
      && length_ == requestSlot.length_;
  }

  bool operator!=(const RequestSlot& requestSlot) const
  {
    return !(*this == requestSlot);
  }
  
  bool operator<(const RequestSlot& requestSlot) const
  {
    if(index_ == requestSlot.index_) {
      return begin_ < requestSlot.begin_;
    } else {
      return index_ < requestSlot.index_;
    }
  }

  void setDispatchedTime(time_t secFromEpoch);

  bool isTimeout(time_t timeoutSec) const;

  size_t getIndex() const { return index_; }
  void setIndex(size_t index) { index_ = index; }

  int32_t getBegin() const { return begin_; }
  void setBegin(int32_t begin) { begin_ = begin; }

  int32_t getLength() const { return length_; }
  void setLength(int32_t length) { length_ = length; }

  size_t getBlockIndex() const { return blockIndex_; }
  void setBlockIndex(size_t blockIndex) { blockIndex_ = blockIndex; }

  const SharedHandle<Piece>& getPiece() const
  {
    return piece_;
  }

  static RequestSlot nullSlot;

  static bool isNull(const RequestSlot& requestSlot);
};

} // namespace aria2

#endif // D_REQUEST_SLOT_H
