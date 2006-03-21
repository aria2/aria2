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
#ifndef _D_PEER_MESSAGE_H_
#define _D_PEER_MESSAGE_H_

#include "common.h"
#include <string>

class PeerMessage {
private:
  int id;
  int index;
  int begin;
  int length;
  unsigned char* bitfield;
  int bitfieldLength;
  char* block;
  int blockLength;
public:
  PeerMessage():bitfield(NULL), bitfieldLength(0),
		block(NULL), blockLength(0) {}
  ~PeerMessage() {
    if(bitfield != NULL) {
      delete [] bitfield;
    }
    if(block != NULL) {
      delete [] block;
    }
  }

  void setBitfield(const unsigned char* bitfield, int bitfieldLength);
  const unsigned char* getBitfield() const { return bitfield; }
  
  void setBlock(const char* block, int blockLength);
  const char* getBlock() const { return block; }
  
  int getBitfieldLength() const { return bitfieldLength; }
  int getBlockLength() const { return blockLength; }

  string toString() const;

  int getId() const { return id; }
  void setId(int id) { this->id = id; }
  int getIndex() const { return index; }
  void setIndex(int index) { this->index = index; }
  int getBegin() const { return begin; }
  void setBegin(int begin) { this->begin = begin; }
  int getLength() const { return length; }
  void setLength(int length) { this->length = length; }

  enum ID {
    CHOKE = 0,
    UNCHOKE = 1,
    INTERESTED = 2,
    NOT_INTERESTED = 3,
    HAVE = 4,
    BITFIELD = 5,
    REQUEST = 6,
    PIECE = 7,
    CANCEL = 8,
    KEEP_ALIVE = 99};
};

#endif // _D_PEER_MESSAGE_H_
