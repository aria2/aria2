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

int8_t PeerMessageUtil::getId(const unsigned char* msg) {
  return msg[0];
}

int32_t PeerMessageUtil::getIntParam(const unsigned char* msg, int32_t offset) {
  int32_t nParam;
  memcpy(&nParam, msg+offset, sizeof(int32_t));
  return ntohl(nParam);
}

int16_t PeerMessageUtil::getShortIntParam(const unsigned char* msg, int32_t offset) {
  int16_t nParam;
  memcpy(&nParam, msg+offset, sizeof(int16_t));
  return ntohs(nParam);
}

void PeerMessageUtil::checkIndex(int32_t index, int32_t pieces) {
  if(!(0 <= index && index < (int32_t)pieces)) {
    throw new DlAbortEx("Invalid index: %d", index);
  }
}

void PeerMessageUtil::checkBegin(int32_t begin, int32_t pieceLength) {
  if(!(0 <= begin && begin < (int32_t)pieceLength)) {
    throw new DlAbortEx("Invalid begin: %d", begin);
  }  
}

void PeerMessageUtil::checkLength(int32_t length) {
  if(length > MAX_BLOCK_LENGTH) {
    throw new DlAbortEx("Length too long: %d > %dKB", length,
			MAX_BLOCK_LENGTH/1024);
  }
  if(length <= 0) {
    throw new DlAbortEx("Invalid length: %d", length);
  }
  if(!Util::isPowerOf(length, 2)) {
    throw new DlAbortEx("Invalid length: %d It is not power of 2",
			length);
  }
}

void PeerMessageUtil::checkRange(int32_t begin, int32_t length, int32_t pieceLength) {
  if(!(0 <= begin && 0 < length)) {
    throw new DlAbortEx("Invalid range: begin=%d, length=%d",
			begin, length);
  }
  int32_t end = begin+length;
  if(!(0 < end && end <= pieceLength)) {
    throw new DlAbortEx("Invalid range: begin=%d, length=%d",
			begin, length);
  }
}

void PeerMessageUtil::checkBitfield(const unsigned char* bitfield,
				    int32_t bitfieldLength,
				    int32_t pieces) {
  if(!(bitfieldLength == BITFIELD_LEN_FROM_PIECES(pieces))) {
    throw new DlAbortEx("Invalid bitfield length: %d",
			bitfieldLength);
  }
  char lastbyte = bitfield[bitfieldLength-1];
  for(int32_t i = 0; i < 8-pieces%8 && pieces%8 != 0; ++i) {
    if(!(((lastbyte >> i) & 1) == 0)) {
      throw new DlAbortEx("Invalid bitfield");
    }
  }
}

void PeerMessageUtil::setIntParam(unsigned char* dest, int32_t param) {
  int32_t nParam = htonl(param);
  memcpy(dest, &nParam, sizeof(int32_t));
}

void PeerMessageUtil::setShortIntParam(unsigned char* dest, int16_t param) {
  int16_t nParam = htons(param);
  memcpy(dest, &nParam, sizeof(int16_t));
}

void PeerMessageUtil::createPeerMessageString(unsigned char* msg,
					      int32_t msgLength,
					      int32_t payloadLength,
					      int8_t messageId) {
  assert(msgLength >= 5);
  memset(msg, 0, msgLength);
  setIntParam(msg, payloadLength);
  msg[4] = messageId;
}
