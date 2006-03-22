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
#ifndef _D_REQUEST_SLOT_MAN_H_
#define _D_REQUEST_SLOT_MAN_H_

#include "RequestSlot.h"
#include "common.h"
#include "PeerMessage.h"
#include "Logger.h"
#include "PeerConnection.h"
#include "PendingMessage.h"
#include "TorrentMan.h"
#include <deque>

#define DEFAULT_TIME_OUT 120

typedef deque<RequestSlot> RequestSlots;

class RequestSlotMan {
private:
  int cuid;
  RequestSlots requestSlots;
  int timeout;
  PendingMessages* pendingMessages;
  PeerConnection* peerConnection;
  TorrentMan* torrentMan;
  const Logger* logger;
public:
  RequestSlotMan(int cuid,
		 PendingMessages* pendingMessages,
		 PeerConnection* peerConnection,
		 TorrentMan* torrentMan,
		 const Logger* logger):cuid(cuid), timeout(DEFAULT_TIME_OUT),
  pendingMessages(pendingMessages), peerConnection(peerConnection),
  torrentMan(torrentMan), logger(logger) {}
  ~RequestSlotMan() {}

  void addRequestSlot(const RequestSlot& requestSlot);
  void deleteRequestSlot(const RequestSlot& requestSlot);
  void deleteAllRequestSlot(Piece& piece);

  void deleteTimedoutRequestSlot(Piece& piece);
  void deleteCompletedRequestSlot(const Piece& piece);

  RequestSlot getCorrespoindingRequestSlot(const PeerMessage* pieceMessage) const;
  bool isEmpty() { return requestSlots.empty(); }
  int countRequestSlot() const { return requestSlots.size(); }

  void setTimeout(int timeout) { this->timeout = timeout; }
  int getTimeout() const { return timeout; }
};

#endif // _D_REQUEST_SLOT_MAN_H_
