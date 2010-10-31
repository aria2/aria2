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
#ifndef D_ZERO_BT_MESSAGE_H
#define D_ZERO_BT_MESSAGE_H

#include "SimpleBtMessage.h"
#include "bittorrent_helper.h"

namespace aria2 {

class ZeroBtMessage : public SimpleBtMessage {
private:
  static const size_t MESSAGE_LENGTH = 5;
protected:
  template<typename T>
  static SharedHandle<T> create(const unsigned char* data, size_t dataLength)
  {
    bittorrent::assertPayloadLengthEqual(1, dataLength, T::NAME);
    bittorrent::assertID(T::ID, data, T::NAME);
    SharedHandle<T> message(new T());
    return message;
  }

public:
  ZeroBtMessage(uint8_t id, const std::string& name):
    SimpleBtMessage(id, name) {}

  virtual unsigned char* createMessage();

  virtual size_t getMessageLength();

  virtual std::string toString() const;
};

} // namespace aria2

#endif // D_ZERO_BT_MESSAGE_H
