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
#ifndef _D_PEER_MESSAGE_UTIL_H_
#define _D_PEER_MESSAGE_UTIL_H_

#include "PeerConnection.h"

class PeerMessageUtil {
private:
  PeerMessageUtil() {}

  static PeerMessage* createBasicMessage(int id, const char* msg, int len);
  static PeerMessage* createHaveMessage(int id, const char* msg, int len);
  static PeerMessage* createBitfieldMessage(int id, const char* msg, int len);
  static PeerMessage* createRequestCancelMessage(int id, const char* msg, int len);
  static PeerMessage* createPieceMessage(int id, const char* msg, int len);
  static int getId(const char* msg);
  static int getIntParam(const char* msg, int offset);

  static void checkIndex(const PeerMessage* message, int pieces);
  static void checkBegin(const PeerMessage* message, int pieceLength);
  static void checkLength(const PeerMessage* message);
  static void checkPieceOffset(const PeerMessage* message, int pieceLength, int pieces, long long int totalLength);
  static void checkBitfield(const PeerMessage* message, int pieces);
public:
  static PeerMessage* createPeerMessage(const char* msg, int len);
  static void checkIntegrity(const PeerMessage* message, int pieceLength, int pieces, long long int totalLength);
  static HandshakeMessage* createHandshakeMessage(const char* msg);
  static void checkHandshake(const HandshakeMessage* message, const unsigned char* infoHash);
};

#endif // _D_PEER_MESSAGE_UTIL_H_
