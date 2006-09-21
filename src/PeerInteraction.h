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
#ifndef _D_PEER_INTERACTION_H_
#define _D_PEER_INTERACTION_H_

#include "common.h"
#include "PeerConnection.h"
#include "RequestSlot.h"
#include "SharedHandle.h"
#include "PeerMessageFactory.h"

#define REQUEST_TIME_OUT 60
#define ALLOWED_FAST_SET_SIZE 10

typedef deque<RequestSlot> RequestSlots;
typedef deque<PeerMessageHandle> MessageQueue;

class PeerInteraction {
private:
  int cuid;
  RequestSlots requestSlots;
  MessageQueue messageQueue;
  const Option* option;
  TorrentMan* torrentMan;
  PeerConnection* peerConnection;
  PeerHandle peer;
  Pieces pieces;
  PeerMessageFactory* peerMessageFactory;
  // allowed fast piece indexes that local client has sent
  Integers fastSet;
  bool quickReplied;
  const Logger* logger;

  void getNewPieceAndSendInterest(int pieceNum);
  int countRequestSlot() const;
public:
  PeerInteraction(int cuid,
		  const PeerHandle& peer,
		  const SocketHandle& socket,
		  const Option* op,
		  TorrentMan* torrentMan);
  ~PeerInteraction();

  void addMessage(const PeerMessageHandle& peerMessage);
  void addRequestSlot(const RequestSlot& requestSlot);
  void rejectPieceMessageInQueue(int index, int begin, int length);
  void rejectAllPieceMessageInQueue();
  void onChoked();
  void abortPiece(Piece& piece);
  void abortAllPieces();

  bool isSendingMessageInProgress() const;
  void deleteRequestSlot(const RequestSlot& requestSlot);
  void checkRequestSlot();
  RequestSlot getCorrespondingRequestSlot(int index, int begin, int length) const;
  bool isInRequestSlot(int index, int blockIndex) const;

  int countMessageInQueue() const;

  TorrentMan* getTorrentMan() const { return torrentMan; }
  PeerConnection* getPeerConnection() const { return peerConnection; }
  // If this object has nullPiece, then return false, otherwise true
  bool hasDownloadPiece(int index) const;
  // If the piece which this object has is nullPiece, then throws an exception.
  // So before calling this function, call hasDownloadPiece and make sure
  // this has valid piece, not nullPiece.
  Piece& getDownloadPiece(int index);
  
  bool isInFastSet(int index) const;
  void addFastSetIndex(int index);

  void syncPiece();
  void updatePiece();
  void addRequests();
  void sendMessages();
  void sendHandshake();
  void sendBitfield();
  void sendAllowedFast();

  PeerMessageHandle receiveMessage();
  PeerMessageHandle receiveHandshake(bool quickReply = false);

  const PeerMessageFactory* getPeerMessageFactory() const {
    return peerMessageFactory;
  }
};

#endif // _D_PEER_INTERACTION_H_
