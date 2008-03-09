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
#ifndef _D_BT_CANCEL_SENDING_PIECE_EVENT_H_
#define _D_BT_CANCEL_SENDING_PIECE_EVENT_H_

#include "BtEvent.h"

namespace aria2 {

class BtCancelSendingPieceEvent : public BtEvent {
private:
  size_t index;
  uint32_t begin;
  size_t length;
public:
  BtCancelSendingPieceEvent(size_t index, uint32_t begin, size_t length):
    index(index), begin(begin), length(length) {}

  virtual ~BtCancelSendingPieceEvent() {}

  void setIndex(size_t index) {
    this->index = index;
  }

  size_t getIndex() const {
    return index;
  }

  void setBegin(uint32_t begin) {
    this->begin = begin;
  }

  uint32_t getBegin() const {
    return begin;
  }

  void setLength(size_t length) {
    this->length = length;
  }

  size_t getLength() const {
    return length;
  }
};

typedef SharedHandle<BtCancelSendingPieceEvent> BtCancelSendingPieceEventHandle;

} // namespace aria2

#endif // _D_BT_CANCEL_SENDING_PIECE_EVENT_H_
