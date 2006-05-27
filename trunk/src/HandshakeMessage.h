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
#ifndef _D_HANDSHAKE_MESSAGE_H_
#define _D_HANDSHAKE_MESSAGE_H_

#include "SimplePeerMessage.h"
#include "TorrentMan.h"

#define PSTR "BitTorrent protocol"
#define HANDSHAKE_MESSAGE_LENGTH 68

class HandshakeMessage : public SimplePeerMessage {
private:
  char msg[HANDSHAKE_MESSAGE_LENGTH];
public:
  char pstrlen;
  string pstr;
  unsigned char reserved[8];
  unsigned char infoHash[INFO_HASH_LENGTH];
  char peerId[PEER_ID_LENGTH];
public:
  HandshakeMessage();

  static HandshakeMessage* create(const char* data, int dataLength);

  virtual ~HandshakeMessage() {}

  virtual int getId() const { return 999; }
  virtual void receivedAction() {};
  virtual const char* getMessage();
  virtual int getMessageLength();
  virtual void check() const;
  virtual string toString() const;

  bool isFastExtensionSupported() const;
};

#endif // _D_HANDSHAKE_MESSAGE_H_
