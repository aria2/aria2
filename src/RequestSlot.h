/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_REQUEST_SLOT_H_
#define _D_REQUEST_SLOT_H_

#include "common.h"
#include <sys/time.h>

class RequestSlot {
private:
  struct timeval dispatchedTime;
  int index;
  int begin;
  int length;
  int blockIndex;
  void copy(const RequestSlot& requestSlot);
public:
  RequestSlot(int index, int begin, int legnth, int blockIndex);
  RequestSlot(const RequestSlot& requestSlot);
  ~RequestSlot() {}

  RequestSlot& operator=(const RequestSlot& requestSlot);

  void setDispatchedTime();

  bool isTimeout(int timeoutSec) const;

  bool operator==(const RequestSlot& requestSlot) const;

  int getIndex() const { return index; }
  void setIndex(int index) { this->index = index; }
  int getBegin() const { return begin; }
  void setBegin(int begin) { this->begin = begin; }
  int getLength() const { return length; }
  void setLength(int length) { this->length = length; }
  int getBlockIndex() const { return blockIndex; }
  void setBlockIndex(int blockIndex) { this->blockIndex = blockIndex; }

  static RequestSlot nullSlot;

  static bool isNull(const RequestSlot& requestSlot);
};

#endif // _D_REQUEST_SLOT_H_
