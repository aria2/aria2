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
#ifndef _D_PEER_MESSAGE_H_
#define _D_PEER_MESSAGE_H_

#include "common.h"
#include "Logger.h"
#include "Peer.h"
#include "Piece.h"
#include "SharedHandle.h"
#include <string>

class PeerInteraction;

class PeerMessage {
protected:
  bool inProgress;
  bool invalidate;
  bool uploading;
  int cuid;
  PeerHandle peer;
  PeerInteraction* peerInteraction;
  const Logger* logger;
public:
  PeerMessage();

  virtual ~PeerMessage() {}

  bool isInProgress() const { return inProgress; }
  bool isInvalidate() const { return invalidate; }
  bool isUploading() const { return uploading; }

  int getCuid() const { return cuid; }
  void setCuid(int cuid) {
    this->cuid = cuid;
  }
  PeerHandle getPeer() const { return this->peer; }
  void setPeer(const PeerHandle& peer) {
    this->peer = peer;
  }
  PeerInteraction* getPeerInteraction() const { return peerInteraction; }
  void setPeerInteraction(PeerInteraction* peerInteraction) {
    this->peerInteraction = peerInteraction;
  }

  virtual int getId() const = 0;
  virtual void receivedAction() = 0;
  virtual void send() = 0;
  virtual void check() const {}
  virtual string toString() const = 0;
  virtual void onPush() {}
  virtual void onChoked() {}
  virtual void onCanceled(int index, int begin, int blockLength) {}
  virtual void onAbortPiece(const Piece& piece) {}
};

typedef SharedHandle<PeerMessage> PeerMessageHandle;

#endif // _D_PEER_MESSAGE_H_
