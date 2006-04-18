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
#include "CompactTrackerResponseProcessor.h"
#include "LogFactory.h"
#include "TorrentDownloadEngine.h"
#include "MetaFileUtil.h"
#include "DlAbortEx.h"
#include "message.h"
#include "PeerInitiateConnectionCommand.h"
#include <netinet/in.h>

CompactTrackerResponseProcessor::CompactTrackerResponseProcessor(ByteArrayDiskWriter* diskWriter, TorrentDownloadEngine* e, Request* req):
  diskWriter(diskWriter),
  e(e),
  req(req) {
  logger = LogFactory::getInstance();
}

CompactTrackerResponseProcessor::~CompactTrackerResponseProcessor() {}

void CompactTrackerResponseProcessor::resetTrackerResponse() {
  if(e->segmentMan->finished()) {
    diskWriter->reset();
    e->segmentMan->init();
  }
}

void CompactTrackerResponseProcessor::execute() {
  MetaEntry* entry = NULL;
  try {
    entry = MetaFileUtil::bdecoding(diskWriter->getByteArray(),
				    diskWriter->getByteArrayLength());
    Dictionary* response = (Dictionary*)entry;
    Data* failureReason = (Data*)response->get("failure reason");
    if(failureReason != NULL) {
      throw new DlAbortEx("Tracker returned failure reason: %s", failureReason->toString().c_str());
    }
    Data* warningMessage = (Data*)response->get("warning message");
    if(warningMessage != NULL) {
      logger->warn(MSG_TRACKER_WARNING_MESSAGE, warningMessage->toString().c_str());
    }
    Data* trackerId = (Data*)response->get("tracker id");
    if(trackerId != NULL) {
      e->torrentMan->trackerId = trackerId->toString();
      logger->debug("Tracker ID:%s", e->torrentMan->trackerId.c_str());
    }
    Data* interval = (Data*)response->get("interval");
    if(interval != NULL) {
      e->torrentMan->interval = interval->toInt();
      logger->debug("interval:%d", e->torrentMan->interval);
    }
    Data* minInterval = (Data*)response->get("min interval");
    if(minInterval != NULL) {
      e->torrentMan->minInterval = minInterval->toInt();
      logger->debug("min interval:%d", e->torrentMan->minInterval);
    }
    if(e->torrentMan->minInterval > e->torrentMan->interval) {
      e->torrentMan->minInterval = e->torrentMan->interval;
    }
    Data* complete = (Data*)response->get("complete");
    if(complete != NULL) {
      e->torrentMan->complete = complete->toInt();
      logger->debug("complete:%d", e->torrentMan->complete);
    }
    Data* incomplete = (Data*)response->get("incomplete");
    if(incomplete != NULL) {
      e->torrentMan->incomplete = incomplete->toInt();
      logger->debug("incomplete:%d", e->torrentMan->incomplete);
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
			      e->torrentMan->getTotalLength());
	if(e->torrentMan->addPeer(peer)) {
	  logger->debug("adding peer %s:%d", peer->ipaddr.c_str(), peer->port);
	} else {
	  delete peer;
	}
    }
    } else {
      logger->info("no peer list received.");
    }
    while(e->torrentMan->isPeerAvailable() &&
	  e->torrentMan->connections < MAX_PEER_UPDATE) {
      Peer* peer = e->torrentMan->getPeer();
      int newCuid =  e->torrentMan->getNewCuid();
      peer->cuid = newCuid;
      PeerInitiateConnectionCommand* command = new PeerInitiateConnectionCommand(newCuid, peer, e);
      e->commands.push(command);
      logger->debug("adding new command CUID#%d", newCuid);
    }
    if(req->getTrackerEvent() == Request::STARTED) {
      req->setTrackerEvent(Request::AUTO);
    }
  } catch(Exception* err) {
    logger->error("Error occurred while processing tracker response.", err);
    delete(err);
  }
  if(entry != NULL) {
    delete entry;
  }
  e->torrentMan->trackers = 0;
}

bool CompactTrackerResponseProcessor::isFeeded() const {
  return e->segmentMan->finished();
}
