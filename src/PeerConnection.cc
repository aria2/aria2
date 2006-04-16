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
#include "PeerConnection.h"
#include "message.h"
#include "DlAbortEx.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include <netinet/in.h>

PeerConnection::PeerConnection(int cuid, const Socket* socket,
			       const Option* op, const Logger* logger,
			       Peer* peer, TorrentMan* torrentMan)
  :cuid(cuid), socket(socket), option(op), logger(logger), peer(peer),
   torrentMan(torrentMan),
   resbufLength(0), currentPayloadLength(0), lenbufLength(0) {}

PeerConnection::~PeerConnection() {}

void PeerConnection::sendHandshake() const {
  /**
   * pstrlen --- '19', 1byte
   * pstr    --- "BitTorrent protocol", 19bytes
   * reserved --- \0 * 8, 8bytes
   * info_hash --- info_hash, 20bytes
   * peer_id --- peer_id, 20bytes
   * total: 68bytes
   */
  char msg[HANDSHAKE_MESSAGE_LENGTH];
  memset(msg, 0, sizeof(msg));
  msg[0] = 19;
  memcpy(&msg[1], PSTR, strlen(PSTR));
  memcpy(&msg[28], torrentMan->getInfoHash(), 20);
  memcpy(&msg[48], torrentMan->peerId.c_str(), 20);
  writeOutgoingMessageLog("handshake");
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendKeepAlive() const {
  /**
   * len --- 0, 4bytes
   * total: 4bytes
   */
  char msg[4];
  memset(msg, 0, sizeof(msg));
  writeOutgoingMessageLog("keep alive");
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::createNLengthMessage(char* msg, int msgLen, int payloadLen, int id) const {
  assert(msgLen >= 5);
  memset(msg, 0, msgLen);
  setIntParam(msg, payloadLen);
  msg[4] = (char)id;
}

void PeerConnection::setIntParam(char* dest, int param) const {
  int nParam = htonl(param);
  memcpy(dest, &nParam, 4);
}

void PeerConnection::writeOutgoingMessageLog(const char* msg) const {
  logger->info(MSG_SEND_PEER_MESSAGE, cuid, peer->ipaddr.c_str(), peer->port, msg);
}

void PeerConnection::writeOutgoingMessageLog(const char* msg, int index) const {
  logger->info(MSG_SEND_PEER_MESSAGE_WITH_INDEX, cuid, peer->ipaddr.c_str(), peer->port, msg, index);
}

void PeerConnection::writeOutgoingMessageLog(const char* msg, const unsigned char* bitfield, int bitfieldLength) const {
  logger->info(MSG_SEND_PEER_MESSAGE_WITH_BITFIELD, cuid, peer->ipaddr.c_str(), peer->port, msg, Util::toHex(bitfield, bitfieldLength).c_str());
}

void PeerConnection::writeOutgoingMessageLog(const char* msg, int index, int begin, int length) const {
  logger->info(MSG_SEND_PEER_MESSAGE_WITH_INDEX_BEGIN_LENGTH, cuid, peer->ipaddr.c_str(), peer->port, msg, index, begin, length);
}

void PeerConnection::sendChoke() const {
  /**
   * len --- 1, 4bytes
   * id --- 0, 1byte
   * total: 5bytes
   */
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 0);
  writeOutgoingMessageLog("choke");
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendUnchoke() const {
  /**
   * len --- 1, 4bytes
   * id --- 1, 1byte
   * total: 5bytes
   */
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 1);
  writeOutgoingMessageLog("unchoke");
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendInterested() const {
  /**
   * len --- 1, 4bytes
   * id --- 2, 1byte
   * total: 5bytes
   */
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 2);
  writeOutgoingMessageLog("interested");
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendNotInterested() const {
  /**
   * len --- 1, 4bytes
   * id --- 3, 1byte
   * total: 5bytes
   */
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 3);
  writeOutgoingMessageLog("not interested");
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendHave(int index) const {
  /**
   * len --- 5, 4bytes
   * id --- 4, 1byte
   * piece index --- index, 4bytes
   * total: 9bytes
   */
  char msg[9];
  createNLengthMessage(msg, sizeof(msg), 5, 4);
  setIntParam(&msg[5], index);
  writeOutgoingMessageLog("have", index);
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendBitfield() const {
  int len = torrentMan->getBitfieldLength();
  const unsigned char* bitfield = torrentMan->getBitfield();
  /**
   * len --- 1+len, 4bytes
   * id --- 5, 1byte
   * bitfield --- bitfield, len bytes
   * total: 5+len bytes
   */
  int msgLen = 5+len;
  char* msg = new char[msgLen];
  try {
    createNLengthMessage(msg, msgLen, 1+len, 5);
    writeOutgoingMessageLog("bitfield", bitfield, len);
    socket->writeData(msg, msgLen);
    delete [] msg;
  } catch(Exception* ex) {
    delete [] msg;
    throw;
  }
}
    
void PeerConnection::sendRequest(int index, int begin, int length) const {
  /**
   * len --- 13, 4bytes
   * id --- 6, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * length --- length, 4bytes
   * total: 17bytes
   */
  char msg[17];
  createNLengthMessage(msg, sizeof(msg), 13, 6);
  setIntParam(&msg[5], index);
  setIntParam(&msg[9], begin);
  setIntParam(&msg[13], length);
  writeOutgoingMessageLog("request", index, begin, length);
  socket->writeData(msg, sizeof(msg));
}

void PeerConnection::sendPiece(int index, int begin, int length) const {
  /**
   * len --- 9+length, 4bytes
   * id --- 7, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * sub total: 13bytes
   * additionally, 
   * block --- data, X bytes
   */
  char msg[13];
  createNLengthMessage(msg, sizeof(msg), 9+length, 7);
  setIntParam(&msg[5], index);
  setIntParam(&msg[9], begin);
  writeOutgoingMessageLog("piece", index, begin, length);
  socket->writeData(msg, sizeof(msg));
  int BUF_SIZE = 4096;
  char buf[BUF_SIZE];
  int iteration = length/BUF_SIZE;
  long long int pieceOffset = ((long long int)index*torrentMan->pieceLength)+begin;
  for(int i = 0; i < iteration; i++) {
    if(torrentMan->diskAdaptor->readData(buf, BUF_SIZE, pieceOffset+i*BUF_SIZE) < BUF_SIZE) {
      throw new DlAbortEx("piece reading failed.");
    }
    socket->writeData(buf, BUF_SIZE);
  }
  int rem = length%BUF_SIZE;
  if(rem > 0) {
    if(torrentMan->diskAdaptor->readData(buf, rem, pieceOffset+iteration*BUF_SIZE) < rem) {
      throw new DlAbortEx("piece reading failed.");
    }
    socket->writeData(buf, rem);
  }
}

void PeerConnection::sendPieceHeader(int index, int begin, int length) const {
  /**
   * len --- 9+length, 4bytes
   * id --- 7, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * sub total: 13bytes
   * additionally, 
   * block --- data, X bytes
   */
  char msg[13];
  createNLengthMessage(msg, sizeof(msg), 9+length, 7);
  setIntParam(&msg[5], index);
  setIntParam(&msg[9], begin);
  writeOutgoingMessageLog("piece", index, begin, length);
  socket->writeData(msg, sizeof(msg));
}

int PeerConnection::sendPieceData(long long int offset, int length) const {
  int BUF_SIZE = 256;
  char buf[BUF_SIZE];
  int iteration = length/BUF_SIZE;
  int writtenLength = 0;
  bool isWritable = true;
  for(int i = 0; i < iteration; i++) {
    isWritable = socket->isWritable(0);
    if(!isWritable) {
      return writtenLength;
    }
    if(torrentMan->diskAdaptor->readData(buf, BUF_SIZE, offset+i*BUF_SIZE) < BUF_SIZE) {
      throw new DlAbortEx("piece reading failed.");
    }
    socket->writeData(buf, BUF_SIZE);
    writtenLength += BUF_SIZE;
  }
  if(socket->isWritable(0)) {
    int rem = length%BUF_SIZE;
    if(rem > 0) {
      if(torrentMan->diskAdaptor->readData(buf, rem, offset+iteration*BUF_SIZE) < rem) {
	throw new DlAbortEx("piece reading failed.");
      }
      socket->writeData(buf, rem);
      writtenLength += rem;
    }
  }
  return writtenLength;
}


void PeerConnection::sendCancel(int index, int begin, int length) const {
  /**
   * len --- 13, 4bytes
   * id --- 8, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * length -- length, 4bytes
   * total: 17bytes
   */
  char msg[17];
  createNLengthMessage(msg, sizeof(msg), 13, 8);
  setIntParam(&msg[5], index);
  setIntParam(&msg[9], begin);
  setIntParam(&msg[13], length);
  writeOutgoingMessageLog("cancel", index, begin, length);
  socket->writeData(msg, sizeof(msg));
}

PeerMessage* PeerConnection::receiveMessage() {
  if(!socket->isReadable(0)) {
    return NULL;
  }
  if(resbufLength == 0 && lenbufLength != 4) {
    // read payload size, 4-byte integer
    int remain = 4-lenbufLength;
    int temp = remain;
    socket->readData(lenbuf+lenbufLength, temp);
    if(temp == 0) {
      // we got EOF
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    if(remain != temp) {
      // still 4-temp bytes to go
      lenbufLength += temp;
      return NULL;
    }
    //payloadLen = ntohl(nPayloadLen);
    int payloadLength = ntohl(*((int*)lenbuf));
    if(payloadLength > MAX_PAYLOAD_LEN) {
      throw new DlAbortEx("max payload length exceeded. length = %d",
			  payloadLength);
    }
    currentPayloadLength = payloadLength;
  }
  // we have currentPayloadLen-resbufLen bytes to read
  int remaining = currentPayloadLength-resbufLength;
  if(remaining > 0) {
    socket->readData(resbuf+resbufLength, remaining);
    if(remaining == 0) {
      // we got EOF
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    resbufLength += remaining;
    if(currentPayloadLength != resbufLength) {
      return NULL;
    }
  }
  // we got whole payload.
  resbufLength = 0;
  lenbufLength = 0;
  PeerMessage* peerMessage = PeerMessageUtil::createPeerMessage(resbuf,	currentPayloadLength);
  try {
    PeerMessageUtil::checkIntegrity(peerMessage, torrentMan->pieceLength,
				    torrentMan->pieces, torrentMan->getTotalLength());
  } catch(Exception* e) {
    delete peerMessage;
    throw;
  }
  return peerMessage;
}

HandshakeMessage* PeerConnection::receiveHandshake() {
  if(!socket->isReadable(0)) {
    return NULL;
  }
  int remain = HANDSHAKE_MESSAGE_LENGTH-resbufLength;
  int temp = remain;
  socket->readData(resbuf+resbufLength, temp);
  if(temp == 0) {
    // we got EOF
    throw new DlAbortEx(EX_EOF_FROM_PEER);
  }
  if(remain != temp) {
    resbufLength += temp;
    return NULL;
  }
  // we got whole handshake payload
  resbufLength = 0;
  HandshakeMessage* handshakeMessage = PeerMessageUtil::createHandshakeMessage(resbuf);
  try {
    PeerMessageUtil::checkHandshake(handshakeMessage, torrentMan->getInfoHash());
  } catch(Exception* e) {
    delete handshakeMessage;
    throw;
  }
  return handshakeMessage;
}
