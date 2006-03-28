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
#ifndef _D_SEND_MESSAGE_QUEUE_H_
#define _D_SEND_MESSAGE_QUEUE_H_

#include "common.h"
#include "RequestSlotMan.h"

class SendMessageQueue {
private:
  int cuid;
  RequestSlotMan* requestSlotMan;
  PendingMessages pendingMessages;
  const Logger* logger;
public:
  SendMessageQueue(int cuid, PeerConnection* peerConnection,
		   TorrentMan* torrentMan, const Logger* logger);
  ~SendMessageQueue();

  void send();

  void addPendingMessage(const PendingMessage& pendingMessage);
  void deletePendingPieceMessage(const PeerMessage* cancelMessage);
  void deletePendingRequestMessage();
  
  void deleteRequestSlot(const RequestSlot& requestSlot);
  void deleteTimeoutRequestSlot(Piece& piece);
  void deleteCompletedRequestSlot(const Piece& piece);
  RequestSlot getCorrespoindingRequestSlot(const PeerMessage* pieceMessage) const;

  void cancelAllRequest();
  void cancelAllRequest(Piece& piece);

  int countPendingMessage() const;
  int countRequestSlot() const;
};

#endif // _D_SEND_MESSAGE_QUEUE_H_
