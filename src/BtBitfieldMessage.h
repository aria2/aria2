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
#ifndef D_BT_BITFIELD_MESSAGE_H
#define D_BT_BITFIELD_MESSAGE_H

#include "SimpleBtMessage.h"

namespace aria2 {

class BtBitfieldMessage : public SimpleBtMessage {
private:
  std::unique_ptr<unsigned char[]> bitfield_;
  size_t bitfieldLength_;

public:
  BtBitfieldMessage();

  BtBitfieldMessage(const unsigned char* bitfield, size_t bitfieldLength);

  virtual ~BtBitfieldMessage();

  static const uint8_t ID = 5;

  static const char NAME[];

  void setBitfield(const unsigned char* bitfield, size_t bitfieldLength);

  const unsigned char* getBitfield() const { return bitfield_.get(); }

  size_t getBitfieldLength() const { return bitfieldLength_; }

  static std::unique_ptr<BtBitfieldMessage> create(const unsigned char* data,
                                                   size_t dataLength);

  virtual void doReceivedAction() CXX11_OVERRIDE;

  virtual unsigned char* createMessage() CXX11_OVERRIDE;

  virtual size_t getMessageLength() CXX11_OVERRIDE;

  virtual std::string toString() const CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_BT_BITFIELD_MESSAGE_H
