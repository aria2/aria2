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
#ifndef _D_BT_BITFIELD_MESSAGE_H_
#define _D_BT_BITFIELD_MESSAGE_H_

#include "SimpleBtMessage.h"

class BtBitfieldMessage;

typedef SharedHandle<BtBitfieldMessage> BtBitfieldMessageHandle;

class BtBitfieldMessage : public SimpleBtMessage {
private:
  unsigned char* bitfield;
  uint32_t bitfieldLength;
  unsigned char* msg;
  uint32_t msgLength;

  void init() {
    bitfield = 0;
    bitfieldLength = 0;
    msg = 0;
    msgLength = 0;
  }
public:
  BtBitfieldMessage():SimpleBtMessage()
  {
    init();
  }

  BtBitfieldMessage(const unsigned char* bitfield,
		    uint32_t bitfieldLength):SimpleBtMessage()
  {
    init();
    setBitfield(bitfield, bitfieldLength);
  }

  virtual ~BtBitfieldMessage() {
    delete [] bitfield;
    delete [] msg;
  }

  enum ID_t {
    ID = 5
  };

  void setBitfield(const unsigned char* bitfield, uint32_t bitfieldLength);

  const unsigned char* getBitfield() const { return bitfield; }

  uint32_t getBitfieldLength() const { return bitfieldLength; }

  static BtBitfieldMessageHandle create(const unsigned char* data, uint32_t dataLength);

  virtual uint8_t getId() const { return ID; }

  virtual void doReceivedAction();

  virtual const unsigned char* getMessage();

  virtual uint32_t getMessageLength();

  virtual string toString() const;
};

#endif // _D_BT_BITFIELD_MESSAGE_H_
