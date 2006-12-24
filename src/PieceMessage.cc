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
  RequestSlot slot = peerInteraction->getCorrespondingRequestSlot(index,
								  begin,
								  blockLength);
  peer->updateDownloadLength(blockLength);
  if(!RequestSlot::isNull(slot) &&
     peerInteraction->hasDownloadPiece(slot.getIndex())) {
    peer->snubbing = false;
    //logger->debug("CUID#%d - Latency=%d", cuid, slot.getLatencyInMillis());
    peer->updateLatency(slot.getLatencyInMillis());
    PieceHandle piece = peerInteraction->getDownloadPiece(slot.getIndex());
    long long int offset =
      ((long long int)index)*btContext->getPieceLength()+begin;
    logger->debug("CUID#%d - Writing the block length=%d, offset=%lld",
		  cuid, blockLength, offset);      
    pieceStorage->getDiskAdaptor()->writeData(block,
					      blockLength,
					      offset);
    piece->completeBlock(slot.getBlockIndex());
    peerInteraction->deleteRequestSlot(slot);
    //pieceStorage->updatePiece(piece);
    logger->debug("CUID#%d - Setting piece block index=%d",
		  cuid, slot.getBlockIndex());
    if(piece->pieceComplete()) {
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
  if(invalidate) {
    return;
  }
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
    long long int pieceDataOffset =
      ((long long int)index)*btContext->getPieceLength()+begin+blockLength-leftDataLength;
    int writtenLength =
      sendPieceData(pieceDataOffset, leftDataLength);
    peer->updateUploadLength(writtenLength);
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
  PeerConnection* peerConnection = peerInteraction->getPeerConnection();
  for(int i = 0; i < iteration; i++) {
    if(pieceStorage->getDiskAdaptor()->readData(buf, BUF_SIZE, offset+i*BUF_SIZE) < BUF_SIZE) {
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
    if(pieceStorage->getDiskAdaptor()->readData(buf, rem, offset+iteration*BUF_SIZE) < rem) {
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

bool PieceMessage::checkPieceHash(const PieceHandle& piece) {
  long long int offset =
    ((long long int)piece->getIndex())*btContext->getPieceLength();
  return pieceStorage->getDiskAdaptor()->sha1Sum(offset, piece->getLength()) ==
    btContext->getPieceHash(piece->getIndex());
}

void PieceMessage::onGotNewPiece(const PieceHandle& piece) {
  logger->info(MSG_GOT_NEW_PIECE, cuid, piece->getIndex());
  pieceStorage->completePiece(piece);
  pieceStorage->advertisePiece(cuid, piece->getIndex());
}

void PieceMessage::onGotWrongPiece(const PieceHandle& piece) {
  logger->error(MSG_GOT_WRONG_PIECE, cuid, piece->getIndex());
  erasePieceOnDisk(piece);
  piece->clearAllBlock();
  //pieceStorage->updatePiece(piece);
  peerInteraction->abortPiece(piece);
}

void PieceMessage::erasePieceOnDisk(const PieceHandle& piece) {
  int BUFSIZE = 4096;
  char buf[BUFSIZE];
  memset(buf, 0, BUFSIZE);
  long long int offset =
    ((long long int)piece->getIndex())*btContext->getPieceLength();
  for(int i = 0; i < piece->getLength()/BUFSIZE; i++) {
    pieceStorage->getDiskAdaptor()->writeData(buf, BUFSIZE, offset);
    offset += BUFSIZE;
  }
  int r = piece->getLength()%BUFSIZE;
  if(r > 0) {
    pieceStorage->getDiskAdaptor()->writeData(buf, r, offset);
  }
}

PeerMessageHandle PieceMessage::createRejectMessage(int index,
						    int begin,
						    int blockLength) const {
  return peerInteraction->getPeerMessageFactory()->
    createRejectMessage(index,
			begin,
			blockLength);
}

void PieceMessage::onChoked() {
  if(!invalidate &&
     !inProgress &&
     !peerInteraction->isInFastSet(index)) {
    logger->debug("CUID#%d - Reject piece message in queue because"
		  " the peer has been choked. index=%d, begin=%d, length=%d",
		  cuid,
		  index,
		  begin,
		  blockLength);

    if(peer->isFastExtensionEnabled()) {
      peerInteraction->addMessage(createRejectMessage(index,
						      begin,
						      blockLength));
    }
    invalidate = true;
  }
}

void PieceMessage::onCanceled(int index, int begin, int blockLength) {
  if(!invalidate &&
     !inProgress &&
     this->index == index &&
     this->begin == begin &&
     this->blockLength == blockLength) {
    logger->debug("CUID#%d - Reject piece message in queue because cancel"
		  " message received. index=%d, begin=%d, length=%d",
		  cuid, index, begin, blockLength);
    if(peer->isFastExtensionEnabled()) {
      peerInteraction->addMessage(createRejectMessage(index,
						      begin,
						      blockLength));
    }
    invalidate = true;
  }
}
