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

#define MAX_BLOCK_LENGTH (128*1024)

class PeerMessageUtil {
private:
  PeerMessageUtil() {}
public:
  static int32_t getIntParam(const unsigned char* msg, int32_t offset);

  static int16_t getShortIntParam(const unsigned char* msg, int32_t offset);

  static void setIntParam(unsigned char* dest, int32_t param);

  static void setShortIntParam(unsigned char* dest, int16_t param);

  static int8_t getId(const unsigned char* msg);
  
  static void checkIndex(int32_t index, int32_t pieces);
  static void checkBegin(int32_t begin, int32_t pieceLength);
  static void checkLength(int32_t length);
  static void checkRange(int32_t begin, int32_t length, int32_t pieceLength);
  static void checkBitfield(const unsigned char* bitfield,
			    int32_t bitfieldLength,
			    int32_t pieces);

  static void createPeerMessageString(unsigned char* msg,
				      int32_t msgLength,
				      int32_t payloadLength,
				      int8_t messageId);

  /**
   * Creates compact tracker format(6bytes for ipv4 address and port)
   * and stores the results in compact.
   * compact must be at least 6 bytes and pre-allocated.
   * Returns true if creation is successful, otherwise returns false.
   * The example of failure reason is that addr is not numbers-and-dots
   * notation.
   */
  static bool createcompact(char* compact, const string& addr, uint16_t port);
};

#endif // _D_PEER_MESSAGE_UTIL_H_
