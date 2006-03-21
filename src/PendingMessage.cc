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
#include "PendingMessage.h"

bool PendingMessage::processMessage() {
  bool retval = true;
  switch(peerMessageId) {
  case PeerMessage::HAVE:
    peerConnection->sendHave(index);
    break;
  case PeerMessage::BITFIELD:
    peerConnection->sendBitfield();
    break;
  case PeerMessage::UNCHOKE:
    if(peerConnection->getPeer()->amChocking) {
      peerConnection->sendUnchoke();
      peerConnection->getPeer()->amChocking = false;
    }
    break;
  case PeerMessage::CHOKE:
    if(!peerConnection->getPeer()->amChocking) {
      peerConnection->sendChoke();
      peerConnection->getPeer()->amChocking = true;
    }
    break;
  case PeerMessage::NOT_INTERESTED:
    if(peerConnection->getPeer()->amInterested) {
      peerConnection->sendNotInterested();
      peerConnection->getPeer()->amInterested = false;
    }
    break;
  case PeerMessage::INTERESTED:
    if(!peerConnection->getPeer()->amInterested) {
      peerConnection->sendInterested();
      peerConnection->getPeer()->amInterested = true;
    }
    break;
  case PeerMessage::PIECE:
    if(!peerConnection->getPeer()->amChocking) {
      if(!inProgress) {
	peerConnection->sendPieceHeader(index, begin, length);
	peerConnection->getPeer()->addPeerDownload(length);
      }
      int writtenLength = peerConnection->sendPieceData(pieceDataOffset, leftPieceDataLength);
      if(writtenLength != leftPieceDataLength) {
	inProgress = true;
	leftPieceDataLength -= writtenLength;
	pieceDataOffset += writtenLength;
	retval = false;
      }
    }
    break;
  case PeerMessage::REQUEST:
    peerConnection->sendRequest(index, begin, length);
    break;
  case PeerMessage::CANCEL:
    peerConnection->sendCancel(index, begin, length);
    break;
  default:
    break;
  }
  return retval;
}

PendingMessage PendingMessage::createRequestMessage(int index, int begin, int length, PeerConnection* peerConnection) {
  PendingMessage pendingMessage(PeerMessage::REQUEST, peerConnection);
  pendingMessage.setIndex(index);
  pendingMessage.setBegin(begin);
  pendingMessage.setLength(length);
  return pendingMessage;
}

PendingMessage PendingMessage::createCancelMessage(int index, int begin, int length, PeerConnection* peerConnection) {
  PendingMessage pendingMessage(PeerMessage::CANCEL, peerConnection);
  pendingMessage.setIndex(index);
  pendingMessage.setBegin(begin);
  pendingMessage.setLength(length);
  return pendingMessage;
}

PendingMessage PendingMessage::createPieceMessage(int index, int begin, int length, int pieceLength, PeerConnection* peerConnection) {
  PendingMessage pendingMessage(PeerMessage::PIECE, peerConnection);
  pendingMessage.setIndex(index);
  pendingMessage.setBegin(begin);
  pendingMessage.setLength(length);
  pendingMessage.setPieceDataOffset(((long long int)index)*pieceLength+begin);
  pendingMessage.setLeftPieceDataLength(length);
  return pendingMessage;
}

PendingMessage PendingMessage::createHaveMessage(int index, PeerConnection* peerConnection) {
  PendingMessage pendingMessage(PeerMessage::HAVE, peerConnection);
  pendingMessage.setIndex(index);
  return pendingMessage;
}

