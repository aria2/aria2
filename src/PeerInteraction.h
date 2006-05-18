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
#ifndef _D_PEER_INTERACTION_H_
#define _D_PEER_INTERACTION_H_

#include "common.h"
#include "PeerConnection.h"
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
#include "HaveAllMessage.h"
#include "HaveNoneMessage.h"
#include "RejectMessage.h"
#include "AllowedFastMessage.h"
#include "SuggestPieceMessage.h"
#include "RequestSlot.h"

#define REQUEST_TIME_OUT 120
#define ALLOWED_FAST_SET_SIZE 10

typedef deque<RequestSlot> RequestSlots;
typedef deque<PeerMessage*> MessageQueue;

class PeerInteraction {
private:
  int cuid;
  RequestSlots requestSlots;
  MessageQueue messageQueue;
  // upload speed limit(byte/sec)
  int uploadLimit;
  TorrentMan* torrentMan;
  PeerConnection* peerConnection;
  Peer* peer;
  Piece piece;
  // allowed fast piece indexes that local client has sent
  Integers fastSet;
  const Logger* logger;
  
  void getNewPieceAndSendInterest();
  PeerMessage* createPeerMessage(const char* msg, int msgLength);
  HandshakeMessage* createHandshakeMessage(const char* msg, int msgLength);
  void setPeerMessageCommonProperty(PeerMessage* peerMessage);
  int countRequestSlot() const;
public:
  PeerInteraction(int cuid,
		  const Socket* socket,
		  const Option* op,
		  TorrentMan* torrentMan,
		  Peer* peer);
  ~PeerInteraction();

  void addMessage(PeerMessage* peerMessage);
  void rejectPieceMessageInQueue(int index, int begin, int length);
  void rejectAllPieceMessageInQueue();
  void onChoked();
  void abortPiece();

  bool isSendingMessageInProgress() const;
  void deleteRequestSlot(const RequestSlot& requestSlot);
  void deleteTimeoutRequestSlot();
  void deleteCompletedRequestSlot();
  RequestSlot getCorrespondingRequestSlot(int index, int begin, int length) const;

  int countMessageInQueue() const;

  void setUploadLimit(int uploadLimit) { this->uploadLimit = uploadLimit; }
  int getUploadLimit() const { return this->uploadLimit; }

  TorrentMan* getTorrentMan() const { return torrentMan; }
  PeerConnection* getPeerConnection() const { return peerConnection; }
  // If this object has nullPiece, then return false, otherwise true
  bool hasDownloadPiece() const {
    return !Piece::isNull(piece);
  }
  // If the piece which this object has is nullPiece, then throws an exception.
  // So before calling this function, call hasDownloadPiece and make sure
  // this has valid piece, not nullPiece.
  Piece& getDownloadPiece();
  void setDownloadPiece(const Piece& piece) {
    this->piece = piece;
  }
  
  bool isInFastSet(int index) const;
  void addFastSetIndex(int index);

  void syncPiece();
  void addRequests();
  void sendMessages(int currentUploadSpeed);
  void sendHandshake();
  void sendBitfield();
  void sendAllowedFast();

  PeerMessage* receiveMessage();
  HandshakeMessage* receiveHandshake();

  RequestMessage* createRequestMessage(int blockIndex);
  CancelMessage* createCancelMessage(int index, int begin, int length);
  PieceMessage* createPieceMessage(int index, int begin, int length);
  HaveMessage* createHaveMessage(int index);
  ChokeMessage* createChokeMessage();
  UnchokeMessage* createUnchokeMessage();
  InterestedMessage* createInterestedMessage();
  NotInterestedMessage* createNotInterestedMessage();
  BitfieldMessage* createBitfieldMessage();
  KeepAliveMessage* createKeepAliveMessage();
  HaveAllMessage* createHaveAllMessage();
  HaveNoneMessage* createHaveNoneMessage();
  RejectMessage* createRejectMessage(int index, int begin, int length);
  AllowedFastMessage* createAllowedFastMessage(int index);
};

#endif // _D_PEER_INTERACTION_H_
