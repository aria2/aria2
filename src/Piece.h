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
  Piece();

  Piece(int index, int length);

  Piece(const Piece& piece);

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

  int getBitfieldLength() const {
    return bitfield->getBitfieldLength();
  }

  void clearAllBlock();
  void setAllBlock();

  string toString() const;

  bool isBlockUsed(int index) const {
    return bitfield->isUseBitSet(index);
  }
};

typedef SharedHandle<Piece> PieceHandle;
typedef deque<PieceHandle> Pieces;

#endif // _D_PIECE_H_
