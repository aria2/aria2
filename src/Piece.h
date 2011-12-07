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
#ifndef D_PIECE_H
#define D_PIECE_H

#include "common.h"

#include <stdint.h>
#include <vector>
#include <string>

#include "SharedHandle.h"
#include "Command.h"

namespace aria2 {

class BitfieldMan;

#ifdef ENABLE_MESSAGE_DIGEST

class MessageDigest;

#endif // ENABLE_MESSAGE_DIGEST

class Piece {
private:
  size_t index_;
  int32_t length_;
  int32_t blockLength_;
  BitfieldMan* bitfield_;
  std::vector<cuid_t> users_;
  bool usedBySegment_;
#ifdef ENABLE_MESSAGE_DIGEST

  int32_t nextBegin_;

  std::string hashType_;

  SharedHandle<MessageDigest> mdctx_;

#endif // ENABLE_MESSAGE_DIGEST

  Piece(const Piece& piece);

  Piece& operator=(const Piece& piece);  
public:

  static const int32_t BLOCK_LENGTH  = 16*1024;

  Piece();

  Piece(size_t index, int32_t length, int32_t blockLength = BLOCK_LENGTH);

  ~Piece();

  bool operator==(const Piece& piece) const
  {
    return index_ == piece.index_;
  }

  bool operator<(const Piece& piece) const
  {
    return index_ < piece.index_;
  }

  // TODO This function only used by unit tests
  bool getMissingUnusedBlockIndex(size_t& index) const;

  // Appends at most n missing unused block index to indexes. For all
  // i in retrieved indexes, call bitfield->setUseBit(i). This
  // function just append index to indexes and it doesn't remove
  // anything from it. Returns the number of indexes to retrieved.
  size_t getMissingUnusedBlockIndex
  (std::vector<size_t>& indexes, size_t n) const;

  bool getFirstMissingBlockIndexWithoutLock(size_t& index) const;
  bool getAllMissingBlockIndexes(unsigned char* misbitfield,
                                 size_t mislen) const;
  void completeBlock(size_t blockIndex);
  void cancelBlock(size_t blockIndex);

  size_t countCompleteBlock() const;

  size_t countMissingBlock() const;

  bool hasBlock(size_t blockIndex) const;

  /**
   * Returns true if all blocks of this piece have been downloaded, otherwise
   * returns false.
   */
  bool pieceComplete() const;

  size_t countBlock() const;

  int32_t getBlockLength(size_t index) const;

  int32_t getBlockLength() const;

  size_t getIndex() const { return index_; }

  void setIndex(size_t index) { index_ = index; }

  int32_t getLength() const { return length_; }

  void setLength(int32_t length) { length_ = length; }

  const unsigned char* getBitfield() const;

  void setBitfield(const unsigned char* bitfield, size_t len);

  size_t getBitfieldLength() const;

  void clearAllBlock();
  void setAllBlock();

  std::string toString() const;

  bool isBlockUsed(size_t index) const;

  // Calculates completed length
  int32_t getCompletedLength();

#ifdef ENABLE_MESSAGE_DIGEST

  void setHashType(const std::string& hashType);

  // Updates hash value. This function compares begin and private variable
  // nextBegin_ and only when they are equal, hash is updated eating data and
  // returns true. Otherwise returns false.
  bool updateHash(int32_t begin, const unsigned char* data, size_t dataLength);

  bool isHashCalculated() const;

  // Returns raw hash value, not hex digest, which is calculated
  // by updateHash().  Please note that this function returns hash
  // value only once. Second invocation without updateHash() returns
  // empty string.
  std::string getDigest();

  void destroyHashContext();

#endif // ENABLE_MESSAGE_DIGEST

  /**
   * Loses current bitfield state.
   */
  void reconfigure(int32_t length);

  void addUser(cuid_t cuid);
  void removeUser(cuid_t cuid);
  bool getUsed() const
  {
    return !users_.empty();
  }
  bool usedBy(cuid_t cuid) const;
  bool getUsedBySegment() const
  {
    return usedBySegment_;
  }
  void setUsedBySegment(bool f)
  {
    usedBySegment_ = f;
  }
};

} // namespace aria2

#endif // D_PIECE_H
