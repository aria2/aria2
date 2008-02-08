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

#include "common.h"
#include "SharedHandle.h"
#include <stdint.h>
#include <deque>
#include <string>

namespace aria2 {

class BitfieldMan;

class Piece {
private:
  int32_t index;
  int32_t length;
  int32_t _blockLength;
  BitfieldMan* bitfield;

  std::deque<SharedHandle<Piece> > _subPieces;
public:

  static const int32_t BLOCK_LENGTH  = 16*1024;

  Piece();

  Piece(int32_t index, int32_t length, int32_t blockLength = BLOCK_LENGTH);

  Piece(const Piece& piece);

  ~Piece();

  Piece& operator=(const Piece& piece);
  
  bool operator==(const Piece& piece) const;

  int32_t getMissingUnusedBlockIndex() const;
  int32_t getMissingBlockIndex() const;
  int32_t getFirstMissingBlockIndexWithoutLock() const;
  std::deque<int32_t> getAllMissingBlockIndexes() const;
  void completeBlock(int32_t blockIndex);
  void cancelBlock(int32_t blockIndex);

  int32_t countCompleteBlock() const;

  bool hasBlock(int32_t blockIndex) const;

  /**
   * Returns true if all blocks of this piece have been downloaded, otherwise
   * returns false.
   */
  bool pieceComplete() const;

  int32_t countBlock() const;

  int32_t getBlockLength(int32_t index) const;

  int32_t getBlockLength() const;

  int32_t getIndex() const { return index; }

  void setIndex(int32_t index) { this->index = index; }

  int32_t getLength() const { return length; }

  void setLength(int32_t index) { this->length = length; }

  const unsigned char* getBitfield() const;

  void setBitfield(const unsigned char* bitfield, int32_t len);

  int32_t getBitfieldLength() const;

  void clearAllBlock();
  void setAllBlock();

  std::string toString() const;

  bool isBlockUsed(int32_t index) const;

  void addSubPiece(const SharedHandle<Piece>& subPiece);

  SharedHandle<Piece> getSubPiece(int32_t blockIndex);
  
  void removeSubPiece(int32_t blockIndex);

  std::deque<SharedHandle<Piece> >::iterator
  getSubPieceIterator(int32_t blockIndex);

  bool isRangeComplete(int32_t offset, int32_t length);

  // Calculates completed length, taking into account SubPieces
  int32_t getCompletedLength();

  /**
   * Loses current bitfield state.
   */
  void reconfigure(int32_t length);
};

typedef SharedHandle<Piece> PieceHandle;
typedef std::deque<PieceHandle> Pieces;

} // namespace aria2

#endif // _D_PIECE_H_
