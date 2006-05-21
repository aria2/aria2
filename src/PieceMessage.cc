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
#include "PeerInteraction.h"
#include "Util.h"
#include "message.h"
#include "DlAbortEx.h"

void PieceMessage::setBlock(const char* block, int blockLength) {
  if(this->block != NULL) {
    delete [] this->block;
  }
  this->blockLength = blockLength;
  this->block = new char[this->blockLength];
  memcpy(this->block, block, this->blockLength);
}

PieceMessage* PieceMessage::create(const char* data, int dataLength) {
  if(dataLength <= 9) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be greater than %d", "piece", dataLength, 9);
  }
  int id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw new DlAbortEx("invalid ID=%d for %s. It should be %d.",
			id, "piece", ID);
  }
  PieceMessage* pieceMessage = new PieceMessage();
  pieceMessage->setIndex(PeerMessageUtil::getIntParam(data, 1));
  pieceMessage->setBegin(PeerMessageUtil::getIntParam(data, 5));
  pieceMessage->setBlock(data+9, dataLength-9);
  return pieceMessage;
}

void PieceMessage::receivedAction() {
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  RequestSlot slot = peerInteraction->getCorrespondingRequestSlot(index,
								  begin,
								  blockLength);
  peer->addPeerUpload(blockLength);
  if(!RequestSlot::isNull(slot) &&
     peerInteraction->hasDownloadPiece(slot.getIndex())) {
    //logger->debug("CUID#%d - Latency=%d", cuid, slot.getLatencyInMillis());
    peer->updateLatency(slot.getLatencyInMillis());
    Piece& piece = peerInteraction->getDownloadPiece(slot.getIndex());
    long long int offset =
      ((long long int)index)*torrentMan->pieceLength+begin;
    logger->debug("CUID#%d - Writing the block length=%d, offset=%lld",
		  cuid, blockLength, offset);      
    torrentMan->diskAdaptor->writeData(block,
				       blockLength,
				       offset);
    piece.completeBlock(slot.getBlockIndex());
    peerInteraction->deleteRequestSlot(slot);
    torrentMan->updatePiece(piece);
    logger->debug("CUID#%d - Setting piece block index=%d",
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

const char* PieceMessage::getMessageHeader() {
  if(!inProgress) {
    /**
     * len --- 9+blockLength, 4bytes
     * id --- 7, 1byte
     * index --- index, 4bytes
     * begin --- begin, 4bytes
     * total: 13bytes
     */
    PeerMessageUtil::createPeerMessageString(msgHeader, sizeof(msgHeader),
					     9+blockLength, ID);
    PeerMessageUtil::setIntParam(&msgHeader[5], index);
    PeerMessageUtil::setIntParam(&msgHeader[9], begin);
  }
  return msgHeader;
}

int PieceMessage::getMessageHeaderLength() {
  return sizeof(msgHeader);
}

void PieceMessage::send() {
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  PeerConnection* peerConnection = peerInteraction->getPeerConnection();
  if(!headerSent) {
    if(!inProgress) {
      logger->info(MSG_SEND_PEER_MESSAGE,
		   cuid, peer->ipaddr.c_str(), peer->port,
		   toString().c_str());
      getMessageHeader();
      leftDataLength = getMessageHeaderLength();
      inProgress = true;
    }
    int writtenLength
      = peerConnection->sendMessage(msgHeader+getMessageHeaderLength()-leftDataLength,
				    leftDataLength);
    if(writtenLength == leftDataLength) {
      headerSent = true;
      leftDataLength = blockLength;
    } else {
      leftDataLength -= writtenLength;
    }
  }
  if(headerSent) {
    inProgress = false;
    int pieceLength = torrentMan->pieceLength;
    long long int pieceDataOffset =
      ((long long int)index)*pieceLength+begin+blockLength-leftDataLength;
    int writtenLength =
      sendPieceData(pieceDataOffset, leftDataLength);
    peer->addPeerDownload(writtenLength);
    torrentMan->addUploadLength(writtenLength);
    torrentMan->addDeltaUploadLength(writtenLength);
    if(writtenLength != leftDataLength) {
      inProgress = true;
    }
    leftDataLength -= writtenLength;
  }
}

int PieceMessage::sendPieceData(long long int offset, int length) const {
  int BUF_SIZE = 256;
  char buf[BUF_SIZE];
  int iteration = length/BUF_SIZE;
  int writtenLength = 0;
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  PeerConnection* peerConnection = peerInteraction->getPeerConnection();
  for(int i = 0; i < iteration; i++) {
    if(torrentMan->diskAdaptor->readData(buf, BUF_SIZE, offset+i*BUF_SIZE) < BUF_SIZE) {
      throw new DlAbortEx("Failed to read data from disk.");
    }
    int ws = peerConnection->sendMessage(buf, BUF_SIZE);
    writtenLength += ws;
    if(ws != BUF_SIZE) {
      //logger->debug("CUID#%d - %d bytes written", cuid, writtenLength);
      return writtenLength;
    }
  }

  int rem = length%BUF_SIZE;
  if(rem > 0) {
    if(torrentMan->diskAdaptor->readData(buf, rem, offset+iteration*BUF_SIZE) < rem) {
      throw new DlAbortEx("Failed to read data from disk.");
    }
    int ws = peerConnection->sendMessage(buf, rem);
    writtenLength += ws;
  }
  //logger->debug("CUID#%d - %d bytes written", cuid, writtenLength);
  return writtenLength;
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
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  long long int offset =
    ((long long int)piece.getIndex())*torrentMan->pieceLength;
  return torrentMan->diskAdaptor->sha1Sum(offset, piece.getLength()) ==
    torrentMan->getPieceHash(piece.getIndex());
}

void PieceMessage::onGotNewPiece(Piece& piece) {
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  logger->info(MSG_GOT_NEW_PIECE, cuid, piece.getIndex());
  torrentMan->completePiece(piece);
  torrentMan->advertisePiece(cuid, piece.getIndex());
}

void PieceMessage::onGotWrongPiece(Piece& piece) {
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  logger->error(MSG_GOT_WRONG_PIECE, cuid, piece.getIndex());
  erasePieceOnDisk(piece);
  piece.clearAllBlock();
  torrentMan->updatePiece(piece);
}

void PieceMessage::erasePieceOnDisk(const Piece& piece) {
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
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

