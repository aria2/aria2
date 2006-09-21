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
  if(length <= 0) {
    throw new DlAbortEx("invalid length %d", length);
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

void PeerMessageUtil::setIntParam(char* dest, int param) {
  int nParam = htonl(param);
  memcpy(dest, &nParam, 4);
}

void PeerMessageUtil::createPeerMessageString(char* msg, int msgLength,
					      int payloadLength,
					      int messageId) {
  assert(msgLength >= 5);
  memset(msg, 0, msgLength);
  setIntParam(msg, payloadLength);
  msg[4] = (char)messageId;
}
