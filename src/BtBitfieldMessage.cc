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
#include "BtBitfieldMessage.h"

#include <cstring>

#include "bittorrent_helper.h"
#include "util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Peer.h"
#include "PieceStorage.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtBitfieldMessage::NAME("bitfield");

BtBitfieldMessage::BtBitfieldMessage():SimpleBtMessage(ID, NAME),
                                       bitfield_(0),
                                       bitfieldLength_(0)
{}

BtBitfieldMessage::BtBitfieldMessage
(const unsigned char* bitfield, size_t bitfieldLength):
  SimpleBtMessage(ID, NAME),
  bitfield_(0),
  bitfieldLength_(0)
{
  setBitfield(bitfield, bitfieldLength);
}

BtBitfieldMessage::~BtBitfieldMessage()
{
  delete [] bitfield_;
}

void BtBitfieldMessage::setBitfield
(const unsigned char* bitfield, size_t bitfieldLength) {
  if(bitfield_ == bitfield) {
    return;
  }
  delete [] bitfield_;
  bitfieldLength_ = bitfieldLength;
  bitfield_ = new unsigned char[bitfieldLength_];
  memcpy(bitfield_, bitfield, bitfieldLength_);
}

BtBitfieldMessageHandle
BtBitfieldMessage::create(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthGreater(1,dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  BtBitfieldMessageHandle message(new BtBitfieldMessage());
  message->setBitfield(data+1, dataLength-1);
  return message;
}

void BtBitfieldMessage::doReceivedAction() {
  if(isMetadataGetMode()) {
    return;
  }
  getPieceStorage()->updatePieceStats(bitfield_, bitfieldLength_,
                                      getPeer()->getBitfield());
  getPeer()->setBitfield(bitfield_, bitfieldLength_);
  if(getPeer()->isSeeder() && getPieceStorage()->downloadFinished()) {
    throw DL_ABORT_EX(MSG_GOOD_BYE_SEEDER);
  }
}

unsigned char* BtBitfieldMessage::createMessage() {
  /**
   * len --- 1+bitfieldLength, 4bytes
   * id --- 5, 1byte
   * bitfield --- bitfield, bitfieldLength bytes
   * total: 5+bitfieldLength bytes
   */
  const size_t msgLength = 5+bitfieldLength_;
  unsigned char* msg = new unsigned char[msgLength];
  bittorrent::createPeerMessageString(msg, msgLength, 1+bitfieldLength_, ID);
  memcpy(msg+5, bitfield_, bitfieldLength_);
  return msg;
}

size_t BtBitfieldMessage::getMessageLength() {
  return 5+bitfieldLength_;
}

std::string BtBitfieldMessage::toString() const {
  std::string s = NAME;
  s += " ";
  s += util::toHex(bitfield_, bitfieldLength_);
  return s;
}

} // namespace aria2
