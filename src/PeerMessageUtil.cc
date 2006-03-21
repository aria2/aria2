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
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"
#include "Util.h"
#include <netinet/in.h>

PeerMessage* PeerMessageUtil::createPeerMessage(const char* msg, int len) {
  PeerMessage* peerMessage;
  if(len == 0) {
    // keep-alive
    peerMessage = new PeerMessage();
    peerMessage->setId(PeerMessage::KEEP_ALIVE);
    return peerMessage;
  }
  int id = getId(msg);
  switch(id) {
  case PeerMessage::CHOKE:
  case PeerMessage::UNCHOKE:
  case PeerMessage::INTERESTED:
  case PeerMessage::NOT_INTERESTED:
    peerMessage = createBasicMessage(id, msg, len);
    break;
  case PeerMessage::HAVE:
    peerMessage = createHaveMessage(id, msg, len);
    break;
  case PeerMessage::BITFIELD:
    peerMessage = createBitfieldMessage(id, msg, len);
    break;
  case PeerMessage::REQUEST:
  case PeerMessage::CANCEL:
    peerMessage = createRequestCancelMessage(id, msg, len);
    break;
  case PeerMessage::PIECE:
    peerMessage = createPieceMessage(id, msg, len);
    break;
  default:
    throw new DlAbortEx("invalid message id. id = %d", id);
  }
  return peerMessage;
}

PeerMessage* PeerMessageUtil::createBasicMessage(int id, const char* msg, int len) {
  if(len != 1) {
    throw new DlAbortEx("invalid payload size for ID%d, size = %d. It should be %d", id, len, 1);
  }
  PeerMessage* peerMessage = new PeerMessage();
  peerMessage->setId(id);
  return peerMessage;
}

PeerMessage* PeerMessageUtil::createHaveMessage(int id, const char* msg, int len) {
  if(len != 5) {
    throw new DlAbortEx("invalid payload size for ID%d, size = %d. It should be %d", id, len, 5);
  }
  PeerMessage* peerMessage = new PeerMessage();
  peerMessage->setId(id);
  peerMessage->setIndex(getIntParam(msg, 1));
  return peerMessage;
}

PeerMessage* PeerMessageUtil::createBitfieldMessage(int id, const char* msg, int len) {
  if(len <= 1) {
    throw new DlAbortEx("invalid payload size for ID%d, size = %d. It should be greater than %d", id, len, 1);
  }
  PeerMessage* peerMessage = new PeerMessage();
  peerMessage->setId(id);
  peerMessage->setBitfield((unsigned char*)msg+1, len-1);
  return peerMessage;
}

PeerMessage* PeerMessageUtil::createRequestCancelMessage(int id, const char* msg, int len) {
  if(len != 13) {
    throw new DlAbortEx("invalid payload size for ID%d, size = %d. It should be %d", id, len, 13);
  }
  PeerMessage* peerMessage = new PeerMessage();
  peerMessage->setId(id);
  peerMessage->setIndex(getIntParam(msg, 1));
  peerMessage->setBegin(getIntParam(msg, 5));
  peerMessage->setLength(getIntParam(msg, 9));
  return peerMessage;
}

PeerMessage* PeerMessageUtil::createPieceMessage(int id, const char* msg, int len) {
  if(len <= 9) {
    throw new DlAbortEx("invalid payload size for ID%d, size = %d. It should be greater than %d", id, len, 9);
  }
  PeerMessage* peerMessage = new PeerMessage();
  peerMessage->setId(id);
  peerMessage->setIndex(getIntParam(msg, 1));
  peerMessage->setBegin(getIntParam(msg, 5));
  peerMessage->setBlock(msg+9, len-9);
  return peerMessage;
}

int PeerMessageUtil::getId(const char* msg) {
  return (int)msg[0];
}

int PeerMessageUtil::getIntParam(const char* msg, int offset) {
  int nParam;
  memcpy(&nParam, msg+offset, 4);
  return ntohl(nParam);
}

void PeerMessageUtil::checkIndex(const PeerMessage* message, int pieces) {
  if(!(0 <= message->getIndex() && message->getIndex() < pieces)) {
    throw new DlAbortEx("invalid index = %d", message->getIndex());
  }
}

void PeerMessageUtil::checkBegin(const PeerMessage* message, int pieceLength) {
  if(!(0 <= message->getBegin() && message->getBegin() < pieceLength)) {
    throw new DlAbortEx("invalid begin = %d", message->getBegin());
  }
}

void PeerMessageUtil::checkPieceOffset(const PeerMessage* message, int pieceLength, int pieces, long long int totalSize) {
  if(!(0 <= message->getBegin() && 0 < message->getLength())) {
    throw new DlAbortEx("invalid offset, begin = %d, length = %d", message->getBegin(), message->getLength());
  }
  int offset = message->getBegin()+message->getLength();
  int currentPieceLength;
  if(message->getIndex()+1 == pieces) {
    currentPieceLength = pieceLength-(pieces*pieceLength-totalSize);
  } else {
    currentPieceLength = pieceLength;
  }
  if(!(0 < offset && offset <= currentPieceLength)) {
    throw new DlAbortEx("invalid offset, begin = %d, length = %d", message->getBegin(), message->getLength());
  }
}

void PeerMessageUtil::checkLength(const PeerMessage* message) {
  if(message->getLength() > 128*1024) {
    throw new DlAbortEx("too large length %d > 128KB", message->getLength());
  }
}

void PeerMessageUtil::checkBitfield(const PeerMessage* message, int pieces) {
  if(!(message->getBitfieldLength() == BITFIELD_LEN_FROM_PIECES(pieces))) {
    throw new DlAbortEx("invalid bitfield length = %d", message->getBitfieldLength());
  }
  char lastbyte = message->getBitfield()[message->getBitfieldLength()-1];
  for(int i = 0; i < 8-pieces%8 && pieces%8 != 0; i++) {
    if(!(((lastbyte >> i) & 1) == 0)) {
      throw new DlAbortEx("invalid bitfield");
    }
  }
}

void PeerMessageUtil::checkIntegrity(const PeerMessage* message, int pieceLength, int pieces, long long int totalSize) {
  // 0 <= index < pieces
  // 0 <= begin < pieceLength
  // 0 < begin+length <= pieceLength
  // len of bitfield == pieces/8+(pieces%8 ? 1 : 0)
  // for(int i = 0; i < 8-pieces%8; i++) { ((lastbyteofbitfield >> i) & 1) == 0 }
  switch(message->getId()) {
  case PeerMessage::KEEP_ALIVE:
  case PeerMessage::CHOKE:
  case PeerMessage::UNCHOKE:
  case PeerMessage::INTERESTED:
  case PeerMessage::NOT_INTERESTED:
    break;
  case PeerMessage::HAVE:
    checkIndex(message, pieces);
    break;
  case PeerMessage::BITFIELD:
    checkBitfield(message, pieces);
    break;
  case PeerMessage::REQUEST:
  case PeerMessage::CANCEL:
    checkIndex(message, pieces);
    checkBegin(message, pieceLength);
    checkLength(message);
    checkPieceOffset(message, pieceLength, pieces, totalSize);
    break;
  case PeerMessage::PIECE:
    checkIndex(message, pieces);
    checkBegin(message, pieceLength);
    break;
  default:
    throw new DlAbortEx("invalid message id. id = %d", message->getId());
  }
}

HandshakeMessage* PeerMessageUtil::createHandshakeMessage(const char* msg) {
  HandshakeMessage* message = new HandshakeMessage();
  message->pstrlen = msg[0];
  char pstr[20];
  memcpy(pstr, &msg[1], sizeof(pstr)-1);
  pstr[sizeof(pstr)-1] = '\0';
  message->pstr = pstr;
  memcpy(message->infoHash, &msg[28], 20);
  memcpy(message->peerId, &msg[48], 20);
  return message;
}

void PeerMessageUtil::checkHandshake(const HandshakeMessage* message, const unsigned char* infoHash) {
  if(message->pstrlen != 19) {
    throw new DlAbortEx("invalid handshake pstrlen = %d", (int)message->pstrlen);
  }
  if(message->pstr != PSTR) {
    throw new DlAbortEx("invalid handshake pstr");
  }
  string myInfoHash = Util::toHex(infoHash, 20);
  string peerInfoHash = Util::toHex(message->infoHash, 20);
  if(myInfoHash != peerInfoHash) {
    throw new DlAbortEx("invalid handshake info hash: expected:%s, actual:%s",
			myInfoHash.c_str(), peerInfoHash.c_str());
  }
}
