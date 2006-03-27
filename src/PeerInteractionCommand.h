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
#ifndef _D_PEER_INTERACTION_COMMAND_H_
#define _D_PEER_INTERACTION_COMMAND_H_

#include "PeerAbstractCommand.h"
#include "PeerConnection.h"
#include "PendingMessage.h"
#include "RequestSlotMan.h"

using namespace std;

class PeerInteractionCommand : public PeerAbstractCommand {
private:
  int sequence;
  PeerConnection* peerConnection;
  RequestSlotMan* requestSlotMan;
  PendingMessages pendingMessages;
  Piece piece;
  struct timeval keepAliveCheckPoint;
  struct timeval chokeCheckPoint;
  void receiveMessage();
  void checkLongTimePeerChoking();
  void syncPiece();
  void detectTimeoutAndDuplicateBlock();
  void decideChoking();
  void sendInterest();
  void sendMessages();
  void createRequestPendingMessage(int blockIndex);
  void deletePendingMessage(PeerMessage* cancelMessage);
  const RequestSlot& getRequestSlot(int index, int begin, int length) const;
  bool deleteRequestSlot(const RequestSlot& slot);
  void deleteAllRequestSlot();
  bool checkPieceHash(const Piece& piece);
  void erasePieceOnDisk(const Piece& piece);
  void keepAlive();
  Piece getNewPieceAndSendInterest();
  void onGotNewPiece();
  void onGotWrongPiece();
protected:
  bool executeInternal();
  bool prepareForRetry(int wait);
  bool prepareForNextPeer(int wait);
  void onAbort(Exception* ex);
  void beforeSocketCheck();
public:
  PeerInteractionCommand(int cuid, Peer* peer, TorrentDownloadEngine* e, Socket* s, int sequence);
  ~PeerInteractionCommand();

  enum Seq {
    INITIATOR_SEND_HANDSHAKE,
    INITIATOR_WAIT_HANDSHAKE,
    RECEIVER_SEND_HANDSHAKE,
    RECEIVER_WAIT_HANDSHAKE,
    WIRED};
};

#endif // _D_PEER_INTERACTION_COMMAND_H_
