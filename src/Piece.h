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
#ifndef _D_PIECE_H_
#define _D_PIECE_H_

#include "BitfieldMan.h"
#include "common.h"

#define BLOCK_LENGTH (16*1024)

class Piece {
private:
  int index;
  int length;
  BitfieldMan* bitfield;
public:
  Piece():index(0), length(0), bitfield(NULL) {}
  Piece(int index, int length):index(index), length(length) {
    bitfield = new BitfieldMan(BLOCK_LENGTH, length);
  }

  Piece(const Piece& piece) {
    index = piece.index;
    length = piece.length;
    if(piece.bitfield == NULL) {
      bitfield = NULL;
    } else {
      bitfield = new BitfieldMan(*piece.bitfield);
    }
  }

  ~Piece() {
    delete bitfield;
  }

  Piece& operator=(const Piece& piece) {
    if(this != &piece) {
      index = piece.index;
      length = piece.length;
      if(bitfield != NULL) {
	delete bitfield;
      }
      if(piece.bitfield == NULL) {
	bitfield = NULL;
      } else {
	bitfield = new BitfieldMan(*piece.bitfield);
      }
    }
    return *this;
  }
  
  bool operator==(const Piece& piece) const {
    return index == piece.index;
  }

  int getMissingUnusedBlockIndex() const;
  int getMissingBlockIndex() const;
  BlockIndexes getAllMissingBlockIndexes() const;
  void completeBlock(int blockIndex);
  void cancelBlock(int blockIndex);
  int countCompleteBlock() const {
    return bitfield->countBlock()-bitfield->countMissingBlock();
  }
  bool hasBlock(int blockIndex) const {
    return bitfield->isBitSet(blockIndex);
  }
  /**
   * Returns true if all blocks of this piece have been downloaded, otherwise
   * returns false.
   */
  bool pieceComplete() const;
  int countBlock() const { return bitfield->countBlock(); }
  int getBlockLength(int index) const {
    return bitfield->getBlockLength(index);
  }
  int getBlockLength() const { return bitfield->getBlockLength(); }
  int getIndex() const { return index; }
  void setIndex(int index) { this->index = index; }
  int getLength() const { return length; }
  void setLength(int index) { this->length = length; }

  const unsigned char* getBitfield() const { return bitfield->getBitfield(); }
  void setBitfield(const unsigned char* bitfield, int len);

  void clearAllBlock();
  void setAllBlock();

  static Piece nullPiece;
  static bool isNull(const Piece& piece) {
    return piece.index == 0 && piece.length == 0;
  }
};

#endif // _D_PIECE_H_
