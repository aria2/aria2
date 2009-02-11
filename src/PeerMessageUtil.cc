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

#include <cassert>
#include <cstring>

#include "DlAbortEx.h"
#include "a2netcompat.h"
#include "StringFormat.h"
#include "BtConstants.h"

namespace aria2 {

uint8_t PeerMessageUtil::getId(const unsigned char* msg) {
  return msg[0];
}

uint32_t PeerMessageUtil::getIntParam(const unsigned char* msg, size_t pos)
{
  uint32_t nParam;
  memcpy(&nParam, msg+pos, sizeof(nParam));
  return ntohl(nParam);
}

uint16_t PeerMessageUtil::getShortIntParam(const unsigned char* msg, size_t pos) {
  uint16_t nParam;
  memcpy(&nParam, msg+pos, sizeof(nParam));
  return ntohs(nParam);
}

void PeerMessageUtil::checkIndex(size_t index, size_t pieces) {
  if(!(index < pieces)) {
    throw DlAbortEx(StringFormat("Invalid index: %lu",
				 static_cast<unsigned long>(index)).str());
  }
}

void PeerMessageUtil::checkBegin(uint32_t begin, size_t pieceLength) {
  if(!(begin < pieceLength)) {
    throw DlAbortEx(StringFormat("Invalid begin: %u", begin).str());
  }  
}

void PeerMessageUtil::checkLength(size_t length) {
  if(length > MAX_BLOCK_LENGTH) {
    throw DlAbortEx(StringFormat("Length too long: %lu > %uKB",
				 static_cast<unsigned long>(length),
				 MAX_BLOCK_LENGTH/1024).str());
  }
  if(length == 0) {
    throw DlAbortEx(StringFormat("Invalid length: %lu",
				 static_cast<unsigned long>(length)).str());
  }
}

void PeerMessageUtil::checkRange(uint32_t begin, size_t length, size_t pieceLength) {
  if(!(0 < length)) {
    throw DlAbortEx(StringFormat("Invalid range: begin=%u, length=%lu",
				 begin,
				 static_cast<unsigned long>(length)).str());
  }
  uint32_t end = begin+length;
  if(!(end <= pieceLength)) {
    throw DlAbortEx(StringFormat("Invalid range: begin=%u, length=%lu",
				 begin,
				 static_cast<unsigned long>(length)).str());
  }
}

void PeerMessageUtil::checkBitfield(const unsigned char* bitfield,
				    size_t bitfieldLength,
				    size_t pieces) {
  if(!(bitfieldLength == (pieces+7)/8)) {
    throw DlAbortEx
      (StringFormat("Invalid bitfield length: %lu",
		    static_cast<unsigned long>(bitfieldLength)).str());
  }
  char lastbyte = bitfield[bitfieldLength-1];
  for(size_t i = 0; i < 8-pieces%8 && pieces%8 != 0; ++i) {
    if(!(((lastbyte >> i) & 1) == 0)) {
      throw DlAbortEx("Invalid bitfield");
    }
  }
}

void PeerMessageUtil::setIntParam(unsigned char* dest, uint32_t param) {
  uint32_t nParam = htonl(param);
  memcpy(dest, &nParam, sizeof(nParam));
}

void PeerMessageUtil::setShortIntParam(unsigned char* dest, uint16_t param) {
  uint16_t nParam = htons(param);
  memcpy(dest, &nParam, sizeof(nParam));
}

void PeerMessageUtil::createPeerMessageString(unsigned char* msg,
					      size_t msgLength,
					      size_t payloadLength,
					      uint8_t messageId) {
  assert(msgLength >= 5);
  memset(msg, 0, msgLength);
  setIntParam(msg, payloadLength);
  msg[4] = messageId;
}

bool
PeerMessageUtil::createcompact(unsigned char* compact, const std::string& addr, uint16_t port)
{
  struct addrinfo hints;
  struct addrinfo* res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // since compact peer format is ipv4 only.
  hints.ai_flags = AI_NUMERICHOST;
  if(getaddrinfo(addr.c_str(), 0, &hints, &res)) {
    return false;
  }
  struct sockaddr_in* in = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
  uint32_t* addrp = (uint32_t*)compact;
  *addrp = in->sin_addr.s_addr;
  uint16_t* portp = (uint16_t*)(compact+4);
  *portp = htons(port);
  freeaddrinfo(res);
  return true;
}

std::pair<std::string, uint16_t>
PeerMessageUtil::unpackcompact(const unsigned char* compact)
{
  struct sockaddr_in in;
  memset(&in, 0, sizeof(in));
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
  // For netbsd
  in.sin_len = sizeof(in);
#endif // HAVE_SOCKADDR_IN_SIN_LEN
  in.sin_family = AF_INET;
  in.sin_addr.s_addr = *reinterpret_cast<const uint32_t*>(compact);
  in.sin_port = 0;
  char host[NI_MAXHOST];
  int s;
  s = getnameinfo(reinterpret_cast<const struct sockaddr*>(&in), sizeof(in),
		  host, NI_MAXHOST, 0, NI_MAXSERV,
		  NI_NUMERICHOST);
  if(s) {
    return std::pair<std::string, uint16_t>();
  }
  uint16_t port = ntohs(*(uint16_t*)(compact+sizeof(uint32_t)));
  return std::pair<std::string, uint16_t>(host, port);
}

} // namespace aria2
