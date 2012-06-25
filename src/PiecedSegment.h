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
#ifndef D_PIECED_SEGMENT_H
#define D_PIECED_SEGMENT_H

#include "Segment.h"

namespace aria2 {

class PiecedSegment:public Segment {
private:
  SharedHandle<Piece> piece_;
  /**
   * Piece class has length property but it is a actual length of piece.
   * The last piece likely have shorter length than the other length.
   */
  int32_t pieceLength_;
  int32_t writtenLength_;

public:
  PiecedSegment(int32_t pieceLength, const SharedHandle<Piece>& piece);

  virtual ~PiecedSegment();

  virtual bool complete() const;

  virtual size_t getIndex() const;

  virtual int64_t getPosition() const;

  virtual int64_t getPositionToWrite() const;

  virtual int32_t getLength() const;

  virtual int32_t getSegmentLength() const
  {
    return pieceLength_;
  }

  virtual int32_t getWrittenLength() const
  {
    return writtenLength_;
  }

  virtual void updateWrittenLength(int32_t bytes);

#ifdef ENABLE_MESSAGE_DIGEST

  // `begin' is a offset inside this segment.
  virtual bool updateHash
  (int32_t begin,
   const unsigned char* data,
   size_t dataLength);

  virtual bool isHashCalculated() const;

  virtual std::string getDigest();

#endif // ENABLE_MESSAGE_DIGEST

  virtual void clear();

  virtual SharedHandle<Piece> getPiece() const;
};

} // namespace aria2

#endif // D_PIECED_SEGMENT_H

