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
#ifndef _D_PENDING_MESSAGE_H_
#define _D_PENDING_MESSAGE_H_

#include "common.h"
#include "PeerConnection.h"
#include <deque>

class PendingMessage {
private:
  int peerMessageId;
  int index;
  int begin;
  int length;
  int blockIndex;
  long long int pieceDataOffset;
  int leftPieceDataLength;
  bool inProgress;
  PeerConnection* peerConnection;
public:
  PendingMessage(int peerMessageId, PeerConnection* peerConnection):peerMessageId(peerMessageId), inProgress(false), peerConnection(peerConnection) {}
  ~PendingMessage() {}

  void setPeerMessageId(int peerMessageId) { this->peerMessageId = peerMessageId; }
  int getPeerMessageId() const { return peerMessageId; }

  void setPieceDataOffset(long long int offset) { this->pieceDataOffset = offset; }
  long long int getPieceDataOffset() const { return pieceDataOffset; }

  void setLeftPieceDataLength(int length) { this->leftPieceDataLength = length; }
  int getLeftPieceDataLength() const { return leftPieceDataLength; }

  void setInProgress(bool inprogress) { this->inProgress = inProgress; }
  bool isInProgress() const { return inProgress; }

  void setIndex(int index) { this->index = index; }
  int getIndex() const { return index; }
  void setBegin(int begin) { this->begin = begin; }
  int getBegin() const { return begin; }
  void setLength(int length) { this->length = length; }
  int getLength() const { return length; }
  void setBlockIndex(int blockIndex) { this->blockIndex = blockIndex; }
  int getBlockIndex() const { return blockIndex; }
  bool processMessage();

  static PendingMessage createRequestMessage(const Piece& piece, int blockIndex, PeerConnection* peerConnection);
  static PendingMessage createCancelMessage(int index, int begin, int length, PeerConnection* peerConnection);
  static PendingMessage createPieceMessage(int index, int begin, int length, int pieceLength, PeerConnection* peerConnection);
  static PendingMessage createHaveMessage(int index, PeerConnection* peerConnectioin);
};

typedef deque<PendingMessage> PendingMessages;

#endif // _D_PENDING_MESSAGE_H_
