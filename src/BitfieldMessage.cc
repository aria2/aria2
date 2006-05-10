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
#include "BitfieldMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"

void BitfieldMessage::setBitfield(const unsigned char* bitfield, int bitfieldLength) {
  if(this->bitfield != NULL) {
    delete [] this->bitfield;
  }
  this->bitfieldLength = bitfieldLength;
  this->bitfield = new unsigned char[this->bitfieldLength];
  memcpy(this->bitfield, bitfield, this->bitfieldLength);
}

void BitfieldMessage::receivedAction() {
  peer->setBitfield(bitfield, bitfieldLength);
}

void BitfieldMessage::send() {
  peerInteraction->getPeerConnection()->sendBitfield();
}

void BitfieldMessage::check() const {
  PeerMessageUtil::checkBitfield(bitfield, bitfieldLength, pieces);
}

string BitfieldMessage::toString() const {
  return "bitfield "+Util::toHex(bitfield, bitfieldLength);
}
