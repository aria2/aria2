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
#ifndef _D_PEER_MESSAGE_FACTORY_H_
#define _D_PEER_MESSAGE_FACTORY_H_

#include "common.h"
#include "PeerMessage.h"
#include "HandshakeMessage.h"

class PeerInteraction;

class PeerMessageFactory {
private:
  int cuid;
  PeerInteraction* peerInteraction;
  PeerHandle peer;

  void setPeerMessageCommonProperty(PeerMessageHandle& peerMessage) const;
public:
  PeerMessageFactory(int cuid,
		     PeerInteraction* peerInteraction,
		     const PeerHandle& peer);

  ~PeerMessageFactory();

  PeerMessageHandle
  createPeerMessage(const char* msg, int msgLength) const;

  PeerMessageHandle
  createHandshakeMessage(const char* msg,
			 int msgLength) const;

  PeerMessageHandle
  createHandshakeMessage(const unsigned char* infoHash,
			 const char* peerId) const;

  PeerMessageHandle createRequestMessage(const Piece& piece,
					 int blockIndex) const;
  PeerMessageHandle createCancelMessage(int index, int begin, int length) const;
  PeerMessageHandle createPieceMessage(int index, int begin, int length) const;
  PeerMessageHandle createHaveMessage(int index) const;
  PeerMessageHandle createChokeMessage() const;
  PeerMessageHandle createUnchokeMessage() const;
  PeerMessageHandle createInterestedMessage() const;
  PeerMessageHandle createNotInterestedMessage() const;
  PeerMessageHandle createBitfieldMessage() const;
  PeerMessageHandle createKeepAliveMessage() const;
  PeerMessageHandle createHaveAllMessage() const;
  PeerMessageHandle createHaveNoneMessage() const;
  PeerMessageHandle createRejectMessage(int index, int begin, int length) const;
  PeerMessageHandle createAllowedFastMessage(int index) const;
};

#endif // _D_PEER_MESSAGE_FACTORY_H_
