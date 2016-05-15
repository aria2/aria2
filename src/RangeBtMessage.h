/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#ifndef D_RANGE_BT_MESSAGE_H
#define D_RANGE_BT_MESSAGE_H

#include "SimpleBtMessage.h"
#include "bittorrent_helper.h"

namespace aria2 {

class RangeBtMessage : public SimpleBtMessage {
private:
  size_t index_;
  int32_t begin_;
  int32_t length_;

  static const size_t MESSAGE_LENGTH = 17;

protected:
  template <typename T>
  static std::unique_ptr<T> create(const unsigned char* data, size_t dataLength)
  {
    bittorrent::assertPayloadLengthEqual(13, dataLength, T::NAME);
    bittorrent::assertID(T::ID, data, T::NAME);
    return make_unique<T>(bittorrent::getIntParam(data, 1),
                          bittorrent::getIntParam(data, 5),
                          bittorrent::getIntParam(data, 9));
  }

public:
  RangeBtMessage(uint8_t id, const char* name, size_t index, int32_t begin,
                 int32_t length);

  size_t getIndex() const { return index_; }

  void setIndex(size_t index) { index_ = index; }

  int32_t getBegin() const { return begin_; }

  void setBegin(int32_t begin) { begin_ = begin; }

  int32_t getLength() const { return length_; }

  void setLength(int32_t length) { length_ = length; }

  virtual std::vector<unsigned char> createMessage() CXX11_OVERRIDE;

  virtual std::string toString() const CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_RANGE_BT_MESSAGE_H
