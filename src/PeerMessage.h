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
#ifndef _D_PEER_MESSAGE_H_
#define _D_PEER_MESSAGE_H_

#include "common.h"
#include "Logger.h"
#include "Peer.h"
#include "Piece.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtRegistry.h"
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
  BtContextHandle btContext;
  PeerStorageHandle peerStorage;
  PieceStorageHandle pieceStorage;
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

  void setBtContext(const BtContextHandle& btContext) {
    this->btContext = btContext;
    pieceStorage = PIECE_STORAGE(btContext);
    peerStorage = PEER_STORAGE(btContext);
  }

  virtual int getId() const = 0;
  virtual void receivedAction() = 0;
  virtual void send() = 0;
  virtual void check() const {}
  virtual string toString() const = 0;
  virtual void onPush() {}
  virtual void onChoked() {}
  virtual void onCanceled(int index, int begin, int blockLength) {}
  virtual void onAbortPiece(const PieceHandle& piece) {}
};

typedef SharedHandle<PeerMessage> PeerMessageHandle;

#endif // _D_PEER_MESSAGE_H_
