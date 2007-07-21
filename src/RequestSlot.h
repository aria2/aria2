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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "TimeA2.h"

class RequestSlot {
private:
  Time dispatchedTime;
  int32_t index;
  int32_t begin;
  int32_t length;
  int32_t blockIndex;
  void copy(const RequestSlot& requestSlot);
public:
  RequestSlot(int32_t index, int32_t begin, int32_t length, int32_t blockIndex);
  RequestSlot(const RequestSlot& requestSlot);
  ~RequestSlot() {}

  RequestSlot& operator=(const RequestSlot& requestSlot) {
    if(this != &requestSlot) {
      copy(requestSlot);
    }
    return *this;
  }

  bool operator==(const RequestSlot& requestSlot) const {
    return index == requestSlot.index &&
      begin == requestSlot.begin &&
      length == requestSlot.length;
  }

  void setDispatchedTime();
  void setDispatchedTime(time_t secFromEpoch);

  bool isTimeout(time_t timeoutSec) const;
  int32_t getLatencyInMillis() const;

  int32_t getIndex() const { return index; }
  void setIndex(int32_t index) { this->index = index; }

  int32_t getBegin() const { return begin; }
  void setBegin(int32_t begin) { this->begin = begin; }

  int32_t getLength() const { return length; }
  void setLength(int32_t length) { this->length = length; }

  int32_t getBlockIndex() const { return blockIndex; }
  void setBlockIndex(int32_t blockIndex) { this->blockIndex = blockIndex; }

  static RequestSlot nullSlot;

  static bool isNull(const RequestSlot& requestSlot);
};

typedef deque<RequestSlot> RequestSlots;

#endif // _D_REQUEST_SLOT_H_
