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
#ifndef _D_REQUEST_SLOT_H_
#define _D_REQUEST_SLOT_H_

#include "common.h"
#include "TimerA2.h"
#include "Piece.h"
#include "wallclock.h"

namespace aria2 {

class RequestSlot {
private:
  Timer _dispatchedTime;
  size_t _index;
  uint32_t _begin;
  size_t _length;
  size_t _blockIndex;

  // This is the piece whose index is index of this RequestSlot has.
  // To detect duplicate RequestSlot, we have to find the piece using
  // PieceStorage::getPiece() repeatedly. It turns out that this process
  // takes time(about 1.7% of processing time). To reduce it, we put piece here
  // at the construction of RequestSlot as a cache.
  SharedHandle<Piece> _piece;

  // inlined for performance reason
  void copy(const RequestSlot& requestSlot)
  {
    _dispatchedTime = requestSlot._dispatchedTime;
    _index = requestSlot._index;
    _begin = requestSlot._begin;
    _length = requestSlot._length;
    _blockIndex = requestSlot._blockIndex;
    _piece = requestSlot._piece;
  }
public:
  
  RequestSlot(size_t index, uint32_t begin, size_t length, size_t blockIndex,
              const SharedHandle<Piece>& piece = SharedHandle<Piece>()):
    _dispatchedTime(global::wallclock),
    _index(index), _begin(begin), _length(length), _blockIndex(blockIndex),
    _piece(piece) {}

  RequestSlot(const RequestSlot& requestSlot):
    _dispatchedTime(requestSlot._dispatchedTime),
    _index(requestSlot._index),
    _begin(requestSlot._begin),
    _length(requestSlot._length),
    _blockIndex(requestSlot._blockIndex),
    _piece(requestSlot._piece) {}

  RequestSlot():_dispatchedTime(0), _index(0), _begin(0), _length(0),
                _blockIndex(0)
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
    return _index == requestSlot._index && _begin == requestSlot._begin
      && _length == requestSlot._length;
  }

  bool operator!=(const RequestSlot& requestSlot) const
  {
    return !(*this == requestSlot);
  }
  
  bool operator<(const RequestSlot& requestSlot) const
  {
    if(_index == requestSlot._index) {
      return _begin < requestSlot._begin;
    } else {
      return _index < requestSlot._index;
    }
  }

  void setDispatchedTime(time_t secFromEpoch);

  bool isTimeout(time_t timeoutSec) const;

  size_t getIndex() const { return _index; }
  void setIndex(size_t index) { _index = index; }

  uint32_t getBegin() const { return _begin; }
  void setBegin(uint32_t begin) { _begin = begin; }

  size_t getLength() const { return _length; }
  void setLength(size_t length) { _length = length; }

  size_t getBlockIndex() const { return _blockIndex; }
  void setBlockIndex(size_t blockIndex) { _blockIndex = blockIndex; }

  const SharedHandle<Piece>& getPiece() const
  {
    return _piece;
  }

  static RequestSlot nullSlot;

  static bool isNull(const RequestSlot& requestSlot);
};

} // namespace aria2

#endif // _D_REQUEST_SLOT_H_
