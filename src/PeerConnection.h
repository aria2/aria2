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
#ifndef _D_PEER_CONNECTION_H_
#define _D_PEER_CONNECTION_H_

#include "Option.h"
#include "Socket.h"
#include "Logger.h"
#include "TorrentMan.h"
#include "PeerMessage.h"
#include "common.h"

// we assume maximum length of incoming message is "piece" message with 16KB
// data. Messages beyond that size are dropped.
#define MAX_PAYLOAD_LEN (9+16*1024)

class PeerConnection {
private:
  int cuid;
  SocketHandle socket;
  const Option* option;
  const Logger* logger;

  char resbuf[MAX_PAYLOAD_LEN];
  int resbufLength;
  int currentPayloadLength;
  char lenbuf[4];
  int lenbufLength;

public:
  PeerConnection(int cuid, const SocketHandle& socket, const Option* op);
  ~PeerConnection();
  
  // Returns the number of bytes written
  int sendMessage(const char* msg, int length);

  bool receiveMessage(char* msg, int& length);
  /**
   * Returns true if a handshake message is fully received, otherwise returns
   * false.
   * In both cases, 'msg' is filled with received bytes and the filled length
   * is assigned to 'length'.
   */
  bool receiveHandshake(char* msg, int& length);
};

#endif // _D_PEER_CONNECTION_H_
