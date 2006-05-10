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

int PeerMessageUtil::getId(const char* msg) {
  return (int)msg[0];
}

int PeerMessageUtil::getIntParam(const char* msg, int offset) {
  int nParam;
  memcpy(&nParam, msg+offset, 4);
  return ntohl(nParam);
}

int PeerMessageUtil::getShortIntParam(const char* msg, int offset) {
  short int nParam;
  memcpy(&nParam, msg+offset, 2);
  return ntohs(nParam);
}

ChokeMessage* PeerMessageUtil::createChokeMessage(const char* msg, int len) {
  if(len != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "choke", len, 1);
  }
  ChokeMessage* chokeMessage = new ChokeMessage();
  return chokeMessage;
}

UnchokeMessage* PeerMessageUtil::createUnchokeMessage(const char* msg, int len) {
  if(len != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "unchoke", len, 1);
  }
  UnchokeMessage* unchokeMessage = new UnchokeMessage();
  return unchokeMessage;
}

InterestedMessage* PeerMessageUtil::createInterestedMessage(const char* msg, int len) {
  if(len != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "interested", len, 1);
  }
  InterestedMessage* interestedMessage = new InterestedMessage();
  return interestedMessage;
}

NotInterestedMessage* PeerMessageUtil::createNotInterestedMessage(const char* msg, int len) {
  if(len != 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "not interested", len, 1);
  }
  NotInterestedMessage* notInterestedMessage = new NotInterestedMessage();
  return notInterestedMessage;
}

HaveMessage* PeerMessageUtil::createHaveMessage(const char* msg, int len) {
  if(len != 5) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "have", len, 5);
  }
  HaveMessage* haveMessage = new HaveMessage();
  haveMessage->setIndex(getIntParam(msg, 1));
  return haveMessage;
}

BitfieldMessage* PeerMessageUtil::createBitfieldMessage(const char* msg, int len) {
  if(len <= 1) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be greater than %d", "bitfield", len, 1);
  }
  BitfieldMessage* bitfieldMessage = new BitfieldMessage();
  bitfieldMessage->setBitfield((unsigned char*)msg+1, len-1);
  return bitfieldMessage;
}

RequestMessage* PeerMessageUtil::createRequestMessage(const char* msg, int len) {
  if(len != 13) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "request", len, 13);
  }
  RequestMessage* requestMessage = new RequestMessage();
  requestMessage->setIndex(getIntParam(msg, 1));
  requestMessage->setBegin(getIntParam(msg, 5));
  requestMessage->setLength(getIntParam(msg, 9));
  return requestMessage;
}

CancelMessage* PeerMessageUtil::createCancelMessage(const char* msg, int len) {
  if(len != 13) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "cancel", len, 13);
  }
  CancelMessage* cancelMessage = new CancelMessage();
  cancelMessage->setIndex(getIntParam(msg, 1));
  cancelMessage->setBegin(getIntParam(msg, 5));
  cancelMessage->setLength(getIntParam(msg, 9));
  return cancelMessage;
}

PieceMessage* PeerMessageUtil::createPieceMessage(const char* msg, int len) {
  if(len <= 9) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be greater than %d", "piece", len, 9);
  }
  PieceMessage* pieceMessage = new PieceMessage();
  pieceMessage->setIndex(getIntParam(msg, 1));
  pieceMessage->setBegin(getIntParam(msg, 5));
  pieceMessage->setBlock(msg+9, len-9);
  return pieceMessage;
}

PortMessage* PeerMessageUtil::createPortMessage(const char* msg, int len) {
  if(len != 3) {
    throw new DlAbortEx("invalid payload size for %s, size = %d. It should be %d", "port", len, 3);
  }
  PortMessage* portMessage = new PortMessage();
  portMessage->setPort(getShortIntParam(msg, 1));
  return portMessage;
}

RequestMessage* PeerMessageUtil::createRequestMessage(int index,
						       int begin,
						       int length,
						       int blockIndex) {
  RequestMessage* msg = new RequestMessage();
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setLength(length);
  msg->setBlockIndex(blockIndex);
  return msg;
}

CancelMessage* PeerMessageUtil::createCancelMessage(int index, int begin, int length) {
  CancelMessage* msg = new CancelMessage();
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setLength(length);
  return msg;
}

PieceMessage* PeerMessageUtil::createPieceMessage(int index, int begin, int length) {
  PieceMessage* msg = new PieceMessage();
  msg->setIndex(index);
  msg->setBegin(begin);
  msg->setBlockLength(length);
  return msg;  
}

HaveMessage* PeerMessageUtil::createHaveMessage(int index) {
  HaveMessage* msg = new HaveMessage();
  msg->setIndex(index);
  return msg;
}

ChokeMessage* PeerMessageUtil::createChokeMessage() {
  ChokeMessage* msg = new ChokeMessage();
  return msg;
}

UnchokeMessage* PeerMessageUtil::createUnchokeMessage() {
  UnchokeMessage* msg = new UnchokeMessage();
  return msg;
}

InterestedMessage* PeerMessageUtil::createInterestedMessage() {
  InterestedMessage* msg = new InterestedMessage();
  return msg;
}

NotInterestedMessage* PeerMessageUtil::createNotInterestedMessage() {
  NotInterestedMessage* msg = new NotInterestedMessage();
  return msg;
}

BitfieldMessage* PeerMessageUtil::createBitfieldMessage() {
  BitfieldMessage* msg = new BitfieldMessage();
  return msg;
}

KeepAliveMessage* PeerMessageUtil::createKeepAliveMessage() {
  KeepAliveMessage* msg = new KeepAliveMessage();
  return msg;
}

void PeerMessageUtil::checkIndex(int index, int pieces) {
  if(!(0 <= index && index < pieces)) {
    throw new DlAbortEx("invalid index = %d", index);
  }
}

void PeerMessageUtil::checkBegin(int begin, int pieceLength) {
  if(!(0 <= begin && begin < pieceLength)) {
    throw new DlAbortEx("invalid begin = %d", begin);
  }  
}

void PeerMessageUtil::checkLength(int length) {
  if(length > MAX_BLOCK_LENGTH) {
    throw new DlAbortEx("too large length %d > %dKB", length,
			MAX_BLOCK_LENGTH/1024);
  }
  if(!Util::isPowerOf(length, 2)) {
    throw new DlAbortEx("invalid length %d, which is not power of 2",
			length);
  }
}

void PeerMessageUtil::checkRange(int begin, int length, int pieceLength) {
  if(!(0 <= begin && 0 < length)) {
    throw new DlAbortEx("invalid range, begin = %d, length = %d",
			begin, length);
  }
  int end = begin+length;
  if(!(0 < end && end <= pieceLength)) {
    throw new DlAbortEx("invalid range, begin = %d, length = %d",
			begin, length);
  }
}

void PeerMessageUtil::checkBitfield(const unsigned char* bitfield, int bitfieldLength, int pieces) {
  if(!(bitfieldLength == BITFIELD_LEN_FROM_PIECES(pieces))) {
    throw new DlAbortEx("invalid bitfield length = %d",
			bitfieldLength);
  }
  char lastbyte = bitfield[bitfieldLength-1];
  for(int i = 0; i < 8-pieces%8 && pieces%8 != 0; i++) {
    if(!(((lastbyte >> i) & 1) == 0)) {
      throw new DlAbortEx("invalid bitfield");
    }
  }
}

HandshakeMessage* PeerMessageUtil::createHandshakeMessage(const char* msg, int length) {
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

