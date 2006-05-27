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
#include "SimplePeerMessage.h"
#include "message.h"
#include "PeerInteraction.h"

SimplePeerMessage::SimplePeerMessage():leftDataLength(0) {}

SimplePeerMessage::~SimplePeerMessage() {}

void SimplePeerMessage::send() {
  if(sendPredicate() || inProgress) {
    const char* msg = getMessage();
    int msgLength = getMessageLength();
    if(!inProgress) {
      logger->info(MSG_SEND_PEER_MESSAGE,
		   cuid, peer->ipaddr.c_str(), peer->port, toString().c_str());
      leftDataLength = getMessageLength();
    }
    inProgress = false;
    int writtenLength = peerInteraction->getPeerConnection()->sendMessage(msg+msgLength-leftDataLength, leftDataLength);
    if(writtenLength == leftDataLength) {
      onSendComplete();
    } else {
      leftDataLength -= writtenLength;
      inProgress = true;
    }
  }
}
