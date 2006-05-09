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
#include "PieceMessage.h"
#include "PeerMessageUtil.h"
#include "SendMessageQueue.h"
#include "Util.h"
#include "message.h"

void PieceMessage::setBlock(const char* block, int blockLength) {
  if(this->block != NULL) {
    delete [] this->block;
  }
  this->blockLength = blockLength;
  this->block = new char[this->blockLength];
  memcpy(this->block, block, this->blockLength);
}

void PieceMessage::receivedAction() {
  TorrentMan* torrentMan = sendMessageQueue->getTorrentMan();
  RequestSlot slot = sendMessageQueue->getCorrespondingRequestSlot(this);
  peer->addPeerUpload(blockLength);
  if(sendMessageQueue->hasDownloadPiece() &&
     !RequestSlot::isNull(slot)) {
    Piece& piece = sendMessageQueue->getDownloadPiece();
    long long int offset =
      ((long long int)index)*torrentMan->pieceLength+begin;
    logger->debug("CUID#%d - write block length = %d, offset=%lld",
		  cuid, blockLength, offset);      
    torrentMan->diskAdaptor->writeData(block,
				       blockLength,
				       offset);
    piece.completeBlock(slot.getBlockIndex());
    sendMessageQueue->deleteRequestSlot(slot);
    torrentMan->updatePiece(piece);
    logger->debug("CUID#%d - setting piece bit index=%d",
		  cuid, slot.getBlockIndex());
    torrentMan->addDeltaDownloadLength(blockLength);
    if(piece.pieceComplete()) {
      if(checkPieceHash(piece)) {
	onGotNewPiece(piece);
      } else {
	onGotWrongPiece(piece);
      }
    }
  }
}

void PieceMessage::send() {
  if((!peer->amChoking && peer->peerInterested) || inProgress) {
    PeerConnection* peerConnection = sendMessageQueue->getPeerConnection();
    if(!inProgress) {
      peerConnection->sendPieceHeader(index, begin, blockLength);
      peer->addPeerDownload(blockLength);
      leftPieceDataLength = blockLength;
    }
    inProgress = false;
    int pieceLength = sendMessageQueue->getTorrentMan()->pieceLength;
    long long int pieceDataOffset =
      ((long long int)index)*pieceLength+begin+blockLength-leftPieceDataLength;
    int writtenLength =
      peerConnection->sendPieceData(pieceDataOffset, leftPieceDataLength);
    if(writtenLength != leftPieceDataLength) {
      inProgress = true;
      leftPieceDataLength -= writtenLength;
    }
  }
}

void PieceMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
  PeerMessageUtil::checkBegin(begin, pieceLength);
}

string PieceMessage::toString() const {
  return "piece index="+Util::itos(index)+", begin="+Util::itos(begin)+
    ", length="+Util::itos(blockLength);
}

bool PieceMessage::checkPieceHash(const Piece& piece) {
  TorrentMan* torrentMan = sendMessageQueue->getTorrentMan();
  long long int offset =
    ((long long int)piece.getIndex())*torrentMan->pieceLength;
  return torrentMan->diskAdaptor->sha1Sum(offset, piece.getLength()) ==
    torrentMan->getPieceHash(piece.getIndex());
}

void PieceMessage::onGotNewPiece(Piece& piece) {
  TorrentMan* torrentMan = sendMessageQueue->getTorrentMan();
  logger->info(MSG_GOT_NEW_PIECE, cuid, piece.getIndex());
  torrentMan->completePiece(piece);
  torrentMan->advertisePiece(cuid, piece.getIndex());
  piece = Piece::nullPiece;
}

void PieceMessage::onGotWrongPiece(Piece& piece) {
  TorrentMan* torrentMan = sendMessageQueue->getTorrentMan();
  logger->error(MSG_GOT_WRONG_PIECE, cuid, piece.getIndex());
  erasePieceOnDisk(piece);
  piece.clearAllBlock();
  torrentMan->updatePiece(piece);
}

void PieceMessage::erasePieceOnDisk(const Piece& piece) {
  TorrentMan* torrentMan = sendMessageQueue->getTorrentMan();
  int BUFSIZE = 4096;
  char buf[BUFSIZE];
  memset(buf, 0, BUFSIZE);
  long long int offset = ((long long int)piece.getIndex())*torrentMan->pieceLength;
  for(int i = 0; i < piece.getLength()/BUFSIZE; i++) {
    torrentMan->diskAdaptor->writeData(buf, BUFSIZE, offset);
    offset += BUFSIZE;
  }
  int r = piece.getLength()%BUFSIZE;
  if(r > 0) {
    torrentMan->diskAdaptor->writeData(buf, r, offset);
  }
}

