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

#include "ChokeMessage.h"
#include "UnchokeMessage.h"
#include "InterestedMessage.h"
#include "NotInterestedMessage.h"
#include "HaveMessage.h"
#include "BitfieldMessage.h"
#include "RequestMessage.h"
#include "CancelMessage.h"
#include "PieceMessage.h"
#include "HandshakeMessage.h"
#include "KeepAliveMessage.h"
#include "PortMessage.h"
#include "PeerConnection.h"

#define MAX_BLOCK_LENGTH (128*1024)

class PeerMessageUtil {
private:
  PeerMessageUtil() {}

  static int getIntParam(const char* msg, int offset);
  static int getShortIntParam(const char* msg, int offset);
public:
  static int getId(const char* msg);

  static ChokeMessage* createChokeMessage(const char* msg, int len);
  static UnchokeMessage* createUnchokeMessage(const char* msg, int len);
  static InterestedMessage* createInterestedMessage(const char* msg, int len);
  static NotInterestedMessage* createNotInterestedMessage(const char* msg, int len);
  static HaveMessage* createHaveMessage(const char* msg, int len);
  static BitfieldMessage* createBitfieldMessage(const char* msg, int len);
  static RequestMessage* createRequestMessage(const char* msg, int len);
  static CancelMessage* createCancelMessage(const char* msg, int len);
  static PieceMessage* createPieceMessage(const char* msg, int len);
  static PortMessage* createPortMessage(const char* msg, int len);

  static void checkIndex(int index, int pieces);
  static void checkBegin(int begin, int pieceLength);
  static void checkLength(int length);
  static void checkRange(int begin, int length, int pieceLength);
  static void checkBitfield(const unsigned char* bitfield, int bitfieldLength, int pieces);

  static HandshakeMessage* createHandshakeMessage(const char* msg, int length);
  static void checkHandshake(const HandshakeMessage* message, const unsigned char* infoHash);
};

#endif // _D_PEER_MESSAGE_UTIL_H_
