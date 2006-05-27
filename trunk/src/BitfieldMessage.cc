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
#include "DlAbortEx.h"

void BitfieldMessage::setBitfield(const unsigned char* bitfield, int bitfieldLength) {
  if(this->bitfield != NULL) {
    delete [] this->bitfield;
  }
  this->bitfieldLength = bitfieldLength;
  this->bitfield = new unsigned char[this->bitfieldLength];
  memcpy(this->bitfield, bitfield, this->bitfieldLength);
}

BitfieldMessage* BitfieldMessage::create(const char* data, int dataLength) {
  if(dataLength <= 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be greater than %d", "bitfield", dataLength, 1);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "bitfield", ID);
  }
  BitfieldMessage* bitfieldMessage = new BitfieldMessage();
  bitfieldMessage->setBitfield((unsigned char*)data+1, dataLength-1);
  return bitfieldMessage;
}

void BitfieldMessage::receivedAction() {
  peer->setBitfield(bitfield, bitfieldLength);
}

const char* BitfieldMessage::getMessage() {
  if(!inProgress && msg == NULL) {
    /**
     * len --- 1+bitfieldLength, 4bytes
     * id --- 5, 1byte
     * bitfield --- bitfield, len bytes
     * total: 5+len bytes
     */
    msgLength = 5+bitfieldLength;
    msg = new char[msgLength];
    PeerMessageUtil::createPeerMessageString(msg, msgLength,
					     1+bitfieldLength, ID);
    memcpy(msg+5, bitfield, bitfieldLength);
  }
  return msg;
}

int BitfieldMessage::getMessageLength() {
  getMessage();
  return msgLength;
}

void BitfieldMessage::check() const {
  PeerMessageUtil::checkBitfield(bitfield, bitfieldLength, pieces);
}

string BitfieldMessage::toString() const {
  return "bitfield "+Util::toHex(bitfield, bitfieldLength);
}
