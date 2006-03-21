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
#include "RequestSlot.h"
#include "Util.h"

RequestSlot::RequestSlot(int index, int begin, int length, int blockIndex)
  :index(index), begin(begin), length(length), blockIndex(blockIndex) {
  setDispatchedTime();
}

RequestSlot::RequestSlot(const RequestSlot& requestSlot) {
  copy(requestSlot);
}

RequestSlot& RequestSlot::operator=(const RequestSlot& requestSlot) {
  if(this != &requestSlot) {
    copy(requestSlot);
  }
  return *this;
}

void RequestSlot::copy(const RequestSlot& requestSlot) {
  index = requestSlot.index;
  begin = requestSlot.begin;
  length = requestSlot.length;
  blockIndex = requestSlot.blockIndex;
  dispatchedTime = requestSlot.dispatchedTime;
}

RequestSlot RequestSlot::nullSlot(0, 0, 0, 0);

void RequestSlot::setDispatchedTime() {
  gettimeofday(&dispatchedTime, NULL);
}

bool RequestSlot::isTimeout(int timeoutSec) const {
  struct timeval now;
  gettimeofday(&now, NULL);
  return Util::difftv(now, dispatchedTime) > timeoutSec*1000000;
}

bool RequestSlot::isNull(const RequestSlot& requestSlot) {
  return requestSlot.index == 0 && requestSlot.begin == 0&&
    requestSlot.length == 0;
}

bool RequestSlot::operator==(const RequestSlot& requestSlot) const {
  return index == requestSlot.index &&
    begin == requestSlot.begin &&
    length == requestSlot.length;
}
