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
#ifndef _D_PIECE_MESSAGE_H_
#define _D_PIECE_MESSAGE_H_

#include "PeerMessage.h"
#include "TorrentMan.h"

class PieceMessage : public PeerMessage {
private:
  int index;
  int begin;
  char* block;
  int blockLength;
  int leftPieceDataLength;
  // for check
  int pieces;
  int pieceLength;

  bool checkPieceHash(const Piece& piece);
  void onGotNewPiece(Piece& piece);
  void onGotWrongPiece(Piece& piece);
  void erasePieceOnDisk(const Piece& piece);
public:
  PieceMessage():PeerMessage(),
		 index(0), begin(0), block(NULL), blockLength(0),
		 leftPieceDataLength(0),
		 pieces(0), pieceLength(0) {}

  virtual ~PieceMessage() {
    if(block != NULL) {
      delete []  block;
    }
  }

  enum ID {
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

  virtual int getId() const { return ID; }
  virtual void receivedAction();
  virtual void send();
  virtual void check() const;
  virtual string toString() const;
};

#endif // _D_PIECE_MESSAGE_H_
