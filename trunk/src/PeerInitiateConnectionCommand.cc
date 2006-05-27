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
#include "PeerInitiateConnectionCommand.h"
#include "PeerInteractionCommand.h"
#include "Util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"

PeerInitiateConnectionCommand::PeerInitiateConnectionCommand(int cuid,
							     Peer* peer,
							     TorrentDownloadEngine* e)
  :PeerAbstractCommand(cuid, peer, e) {}

PeerInitiateConnectionCommand::~PeerInitiateConnectionCommand() {}

bool PeerInitiateConnectionCommand::executeInternal() {
  socket = new Socket();
  // socket->establishConnection(...);

  Command* command;
  logger->info(MSG_CONNECTING_TO_SERVER, cuid, peer->ipaddr.c_str(),
	       peer->port);
  socket->establishConnection(peer->ipaddr, peer->port);
  command = new PeerInteractionCommand(cuid, peer, e, socket, PeerInteractionCommand::INITIATOR_SEND_HANDSHAKE);

  e->commands.push_back(command);
  return true;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInitiateConnectionCommand::prepareForNextPeer(int wait) {
  if(e->torrentMan->isPeerAvailable()) {
    Peer* peer = e->torrentMan->getPeer();
    int newCuid = e->torrentMan->getNewCuid();
    peer->cuid = newCuid;
    PeerInitiateConnectionCommand* command = new PeerInitiateConnectionCommand(newCuid, peer, e);
    e->commands.push_back(command);
  }
  return true;
}

bool PeerInitiateConnectionCommand::prepareForRetry(int wait) {
  PeerInitiateConnectionCommand* command = new PeerInitiateConnectionCommand(cuid, peer, e);
  e->commands.push_back(command);
  return true;
}
