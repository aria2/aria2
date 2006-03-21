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
#include "HandshakeMessage.h"

// we assume maximum length of incoming message is "piece" message with 16KB
// data. Messages beyond that size are dropped.
#define MAX_PAYLOAD_LEN (9+16*1024)

class PeerConnection {
private:
  int cuid;
  const Socket* socket;
  const Option* option;
  const Logger* logger;
  Peer* peer;
  TorrentMan* torrentMan;

  char resbuf[MAX_PAYLOAD_LEN];
  int resbufLength;
  int currentPayloadLength;
  char lenbuf[4];
  int lenbufLength;

  void createNLengthMessage(char* msg, int msgLen, int payloadLen, int id) const;
  void setIntParam(char* dest, int param) const;

  void writeOutgoingMessageLog(const char* msg) const;
  void writeOutgoingMessageLog(const char* msg, int index) const;
  void writeOutgoingMessageLog(const char* msg, const unsigned char* bitfield, int bitfieldLength) const;
  void writeOutgoingMessageLog(const char* msg, int index, int begin, int length) const;
public:
  PeerConnection(int cuid, const Socket* socket, const Option* op,
		 const Logger* logger, Peer* peer, TorrentMan* torrenMan);
  ~PeerConnection();

  void sendHandshake() const;
  void sendKeepAlive() const;
  void sendChoke() const;
  void sendUnchoke() const;
  void sendInterested() const;
  void sendNotInterested() const;
  void sendHave(int index) const;
  void sendBitfield() const;
  void sendRequest(int index, int begin, int length) const;
  void sendPiece(int index, int begin, int length) const;
  void sendPieceHeader(int index, int begin, int length) const;
  int sendPieceData(long long int offset, int length) const;
  void sendCancel(int index, int begin, int length) const;
  PeerMessage* receiveMessage();
  HandshakeMessage* receiveHandshake();

  Peer* getPeer() const { return peer; }
};

#endif // _D_PEER_CONNECTION_H_
