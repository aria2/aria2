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
#include "TrackerUpdateCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "PeerListenCommand.h"
#include "Dictionary.h"
#include "Data.h"
#include "DlAbortEx.h"
#include "message.h"
#include <netinet/in.h>

TrackerUpdateCommand::TrackerUpdateCommand(int cuid, Request* req,
					   TorrentDownloadEngine* e,
					   MetaEntry* trackerResponse)
  :Command(cuid), req(req), e(e), trackerResponse(trackerResponse) {}

TrackerUpdateCommand::~TrackerUpdateCommand() {
  delete trackerResponse;
}

bool TrackerUpdateCommand::execute() {
  Dictionary* response = (Dictionary*)trackerResponse;
  Data* failureReason = (Data*)response->get("failure reason");
  if(failureReason != NULL) {
    throw new DlAbortEx("Tracker returned failure reason: %s", failureReason->toString().c_str());
  }
  Data* warningMessage = (Data*)response->get("warning message");
  if(warningMessage != NULL) {
    e->logger->warn(MSG_TRACKER_WARNING_MESSAGE, cuid, warningMessage->toString().c_str());
  }
  Data* trackerId = (Data*)response->get("tracker id");
  if(trackerId != NULL) {
    e->torrentMan->trackerId = trackerId->toString();
    e->logger->debug("CUID#%d - Tracker ID:%s", cuid, e->torrentMan->trackerId.c_str());
  }
  Data* interval = (Data*)response->get("interval");
  if(interval != NULL) {
    e->torrentMan->interval = interval->toInt();
    e->logger->debug("CUID#%d - interval:%d", cuid, e->torrentMan->interval);
  }
  Data* minInterval = (Data*)response->get("min interval");
  if(minInterval != NULL) {
    e->torrentMan->minInterval = minInterval->toInt();
    e->logger->debug("CUID#%d - min interval:%d", cuid, e->torrentMan->minInterval);
  }
  Data* complete = (Data*)response->get("complete");
  if(complete != NULL) {
    e->torrentMan->complete = complete->toInt();
    e->logger->debug("CUID#%d - complete:%d", cuid, e->torrentMan->complete);
  }
  Data* incomplete = (Data*)response->get("incomplete");
  if(incomplete != NULL) {
    e->torrentMan->incomplete = incomplete->toInt();
    e->logger->debug("CUID#%d - incomplete:%d", cuid, e->torrentMan->incomplete);
  } 
  Data* peers = (Data*)response->get("peers");
  if(peers != NULL) {
    for(int i = 0; i < peers->getLen(); i += 6) {
      unsigned int ipaddr1 = (unsigned char)*(peers->getData()+i);
      unsigned int ipaddr2 = (unsigned char)*(peers->getData()+i+1);
      unsigned int ipaddr3 = (unsigned char)*(peers->getData()+i+2);
      unsigned int ipaddr4 = (unsigned char)*(peers->getData()+i+3);
      unsigned int port = ntohs(*(unsigned short int*)(peers->getData()+i+4));
      char ipaddr[16];
      
      snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%d",
	       ipaddr1, ipaddr2, ipaddr3, ipaddr4);
      Peer* peer = new Peer(ipaddr, port, e->torrentMan->pieceLength,
			    e->torrentMan->totalSize);
      if(e->torrentMan->addPeer(peer)) {
	e->logger->debug("CUID#%d - adding peer %s:%d", cuid,
			 peer->ipaddr.c_str(), peer->port);
      } else {
	delete peer;
      }
    }
  } else {
    e->logger->info("CUID#%d - no peer list received.", cuid);
  }
  while(e->torrentMan->isPeerAvailable() &&
	e->torrentMan->connections < MAX_PEER_UPDATE) {
    Peer* peer = e->torrentMan->getPeer();
    int newCuid =  e->torrentMan->getNewCuid();
    peer->cuid = newCuid;
    PeerInitiateConnectionCommand* command = new PeerInitiateConnectionCommand(newCuid, peer, e);
    e->commands.push(command);
    e->logger->debug("CUID#%d - adding new command CUID#%d", cuid, newCuid);
  }
  if(req->getTrackerEvent() == Request::STARTED) {
    req->setTrackerEvent(Request::AUTO);
  }
  return true;
}
