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
#ifndef _D_PIECE_MESSAGE_H_
#define _D_PIECE_MESSAGE_H_

#include "PeerMessage.h"

class PieceMessage : public PeerMessage {
private:
  int index;
  int begin;
  char* block;
  int blockLength;
  int leftDataLength;
  bool headerSent;
  int pendingCount;
  // for check
  int pieces;
  int pieceLength;
  
  char msgHeader[13];

  bool checkPieceHash(const PieceHandle& piece);
  void onGotNewPiece(const PieceHandle& piece);
  void onGotWrongPiece(const PieceHandle& piece);
  void erasePieceOnDisk(const PieceHandle& piece);
  int sendPieceData(long long int offset, int length) const;
  PeerMessageHandle createRejectMessage(int index, int begin, int blockLength) const;
public:
  PieceMessage(int index = 0, int begin = 0, int blockLength = 0)
    :PeerMessage(),
     index(index),
     begin(begin),
     block(0),
     blockLength(blockLength),
     leftDataLength(0),
     headerSent(false),
     pendingCount(0),
     pieces(0),
     pieceLength(0)
  {
    uploading = true;
  }

  virtual ~PieceMessage() {
      delete []  block;
  }

  enum ID_t {
    ID = 7
  };

  int getIndex() const { return index; }
  void setIndex(int index) { this->index = index; }
  int getBegin() const { return begin; }
  void setBegin(int begin) { this->begin = begin; }
  const char* getBlock() const { return block; }
  void setBlock(const char* block, int blockLength);
  int getBlockLength() const { return blockLength; }
  void setBlockLength(int blockLength) { this->blockLength = blockLength; }

  void setPieces(int pieces) {
    this->pieces = pieces;
  }
  int getPieces() const { return pieces;}

  void setPieceLength(int pieceLength) {
    this->pieceLength = pieceLength;
  }
  int getPieceLength() const { return pieceLength;}

  void incrementPendingCount() { pendingCount++; }
  bool isPendingCountMax() const { return pendingCount > 2; }

  static PieceMessage* create(const char* data, int dataLength);

  virtual int getId() const { return ID; }
  virtual void receivedAction();
  virtual const char* getMessageHeader();
  virtual int getMessageHeaderLength();
  virtual void send();
  virtual void check() const;
  virtual string toString() const;
  virtual void onChoked();
  virtual void onCanceled(int index, int begin, int blockLength);
};

#endif // _D_PIECE_MESSAGE_H_
