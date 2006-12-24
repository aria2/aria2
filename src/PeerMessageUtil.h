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
#ifndef _D_PEER_MESSAGE_UTIL_H_
#define _D_PEER_MESSAGE_UTIL_H_

#include "common.h"
#include "HandshakeMessage.h"

#define MAX_BLOCK_LENGTH (128*1024)

class PeerMessageUtil {
private:
  PeerMessageUtil() {}
public:
  static uint32_t getIntParam(const unsigned char* msg, int32_t offset);
  static uint32_t getIntParam(const char* msg, int32_t offset) {
    return getIntParam((const unsigned char*)msg, offset);
  }

  static uint16_t getShortIntParam(const unsigned char* msg, int32_t offset);
  static uint16_t getShortIntParam(const char* msg, int32_t offset) {
    return getShortIntParam((const unsigned char*)msg, offset);
  }

  static void setIntParam(unsigned char* dest, uint32_t param);
  static void setIntParam(char* dest, uint32_t param) {
    setIntParam((unsigned char*)dest, param);
  }

  static void setShortIntParam(unsigned char* dest, uint16_t param);
  static void setShortIntParam(char* dest, uint16_t param) {
    setShortIntParam((unsigned char*)dest, param);
  }

  static int32_t getId(const unsigned char* msg);
  static int32_t getId(const char* msg) {
    return getId((const unsigned char*)msg);
  }
  
  static void checkIndex(uint32_t index, uint32_t pieces);
  static void checkBegin(uint32_t begin, uint32_t pieceLength);
  static void checkLength(uint32_t length);
  static void checkRange(uint32_t begin, uint32_t length, uint32_t pieceLength);
  static void checkBitfield(const unsigned char* bitfield,
			    uint32_t bitfieldLength,
			    uint32_t pieces);
  static void checkBitfield(const char* bitfield,
			    uint32_t bitfieldLength,
			    uint32_t pieces) {
    checkBitfield((unsigned char*)bitfield, bitfieldLength, pieces);
  }

  static void checkHandshake(const HandshakeMessage* message,
			     const unsigned char* infoHash);

  static void createPeerMessageString(unsigned char* msg,
				      uint32_t msgLength,
				      uint32_t payloadLength,
				      int32_t messageId);
  static void createPeerMessageString(char* msg,
				      uint32_t msgLength,
				      uint32_t payloadLength,
				      int32_t messageId) {
    createPeerMessageString((unsigned char*)msg, msgLength, payloadLength, messageId);
  }
};

#endif // _D_PEER_MESSAGE_UTIL_H_
