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
#include "RequestMessage.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "Util.h"

void RequestMessage::receivedAction() {
  TorrentMan* torrentMan = peerInteraction->getTorrentMan();
  if(torrentMan->hasPiece(index)) {
    peerInteraction->addMessage(peerInteraction->createPieceMessage(index, begin, length));
    torrentMan->addUploadLength(length);
    torrentMan->addDeltaUploadLength(length);
  }
}

void RequestMessage::send() {
  if(!peer->peerChoking) {
    peerInteraction->getPeerConnection()->sendRequest(index, begin, length);
  }
}

void RequestMessage::check() const {
  PeerMessageUtil::checkIndex(index, pieces);
  PeerMessageUtil::checkBegin(begin, pieceLength);
  PeerMessageUtil::checkLength(length);
  PeerMessageUtil::checkRange(begin, length, pieceLength);
}

string RequestMessage::toString() const {
  return "request index="+Util::itos(index)+", begin="+Util::itos(begin)+
    ", length="+Util::itos(length);
}
