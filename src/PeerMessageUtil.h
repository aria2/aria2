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
  static int getIntParam(const char* msg, int offset);
  static int getShortIntParam(const char* msg, int offset);
  static void setIntParam(char* dest, int param);

  static int getId(const char* msg);
  
  static void checkIndex(int index, int pieces);
  static void checkBegin(int begin, int pieceLength);
  static void checkLength(int length);
  static void checkRange(int begin, int length, int pieceLength);
  static void checkBitfield(const unsigned char* bitfield,
			    int bitfieldLength,
			    int pieces);

  static void checkHandshake(const HandshakeMessage* message,
			     const unsigned char* infoHash);

  static void createPeerMessageString(char* msg, int msgLength,
				      int payloadLength, int messageId);
};

#endif // _D_PEER_MESSAGE_UTIL_H_
