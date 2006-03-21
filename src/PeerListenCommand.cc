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
#include "PeerListenCommand.h"
#include "PeerInteractionCommand.h"

PeerListenCommand::PeerListenCommand(int cuid, TorrentDownloadEngine* e)
  :Command(cuid), e(e), socket(NULL) {
}

PeerListenCommand::~PeerListenCommand() {
  if(socket != NULL) {
    delete socket;
  }
}

int PeerListenCommand::bindPort(int portRangeStart, int portRangeEnd) {
  if(portRangeStart > portRangeEnd) {
    return -1;
  }
  for(int port = portRangeStart; port <= portRangeEnd; port++) {
    try {
      socket = new Socket();
      socket->beginListen(port);
      return port;
    } catch(Exception* ex) {
      e->logger->error("CUID#%d - an error occurred while binding port=%d",
		       cuid, port);
      delete socket;
      socket = NULL;
    }
  }
  return -1;
}

bool PeerListenCommand::execute() {
  if(e->torrentMan->downloadComplete()) {
    return true;
  }
  for(int i = 0; i < 3 && socket->isReadable(0); i++) {
    Socket* peerSocket = NULL;
    try {
      peerSocket = socket->acceptConnection();
      if(e->torrentMan->connections < MAX_PEERS) {
	pair<string, int> peerInfo;
	peerSocket->getPeerInfo(peerInfo);
	Peer* peer = new Peer(peerInfo.first, peerInfo.second,
			      e->torrentMan->pieceLength,
			      e->torrentMan->totalSize);
	if(e->torrentMan->addPeer(peer, true)) {
	  int newCuid =  e->torrentMan->getNewCuid();
	  peer->cuid = newCuid;
	  PeerInteractionCommand* command =
	    new PeerInteractionCommand(newCuid, peer, e, peerSocket,
				       PeerInteractionCommand::RECEIVER_WAIT_HANDSHAKE);
	  e->commands.push(command);
	  e->logger->debug("CUID#%d - incoming connection, adding new command CUID#%d", cuid, newCuid);
	} else {
	  delete peer;
	}
      }
      delete peerSocket;
    } catch(Exception* ex) {
      e->logger->error("CUID#%d - error in accepting connection", cuid, ex);
      delete ex;
      if(peerSocket != NULL) {
	delete peerSocket;
      }
    }		    
  }
  e->commands.push(this);
  return false;
}
