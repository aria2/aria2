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
#include "LogFactory.h"
#include "MetaFileUtil.h"
#include "DlAbortEx.h"
#include "message.h"
#include "PeerInitiateConnectionCommand.h"
#include "SleepCommand.h"
#include "Util.h"
#include <netinet/in.h>

TrackerUpdateCommand::TrackerUpdateCommand(int cuid, TorrentDownloadEngine*e):Command(cuid), e(e) {
  logger = LogFactory::getInstance();
}

TrackerUpdateCommand::~TrackerUpdateCommand() {}

bool TrackerUpdateCommand::prepareForRetry() {
  e->commands.push_back(this);
  return false;
}

char* TrackerUpdateCommand::getTrackerResponse(int& trackerResponseLength) {
  int maxBufLength = 2048;
  char* buf = new char[maxBufLength];
  int bufLength = 0;
  char data[2048];
  try {
    while(1) {
      int dataLength = e->segmentMan->diskWriter->readData(data, sizeof(data), bufLength);
      if(bufLength+dataLength >= maxBufLength) {
	maxBufLength = Util::expandBuffer(&buf, bufLength, bufLength+dataLength);
      }
      memcpy(buf+bufLength, data, dataLength);
      bufLength += dataLength;
      if(dataLength != sizeof(data)) {
	break;
      }
    }
    trackerResponseLength = bufLength;
    return buf;
  } catch(Exception* e) {
    delete [] buf;
    throw;
  }
}

bool TrackerUpdateCommand::execute() {
  if(e->segmentMan->errors > 0 && e->torrentMan->isHalt()) {
    return true;
  }
  if(!e->segmentMan->finished()) {
    return prepareForRetry();
  }
  char* trackerResponse = NULL;
  int trackerResponseLength = 0;

  try {
    trackerResponse = getTrackerResponse(trackerResponseLength);
    SharedHandle<MetaEntry> entry(MetaFileUtil::bdecoding(trackerResponse,
							  trackerResponseLength));
    Dictionary* response = (Dictionary*)entry.get();
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
      logger->debug("CUID#%d - Tracker ID:%s",
		    cuid, e->torrentMan->trackerId.c_str());
    }
    Data* interval = (Data*)response->get("interval");
    if(interval != NULL) {
      e->torrentMan->interval = interval->toInt();
      logger->debug("CUID#%d - Interval:%d", cuid, e->torrentMan->interval);
    }
    Data* minInterval = (Data*)response->get("min interval");
    if(minInterval != NULL) {
      e->torrentMan->minInterval = minInterval->toInt();
      logger->debug("CUID#%d - Min interval:%d",
		    cuid, e->torrentMan->minInterval);
    }
    if(e->torrentMan->minInterval > e->torrentMan->interval) {
      e->torrentMan->minInterval = e->torrentMan->interval;
    }
    Data* complete = (Data*)response->get("complete");
    if(complete != NULL) {
      e->torrentMan->complete = complete->toInt();
      logger->debug("CUID#%d - Complete:%d", cuid, e->torrentMan->complete);
    }
    Data* incomplete = (Data*)response->get("incomplete");
    if(incomplete != NULL) {
      e->torrentMan->incomplete = incomplete->toInt();
      logger->debug("CUID#%d - Incomplete:%d",
		    cuid, e->torrentMan->incomplete);
    }
    if(!e->torrentMan->isHalt() &&
       e->torrentMan->connections < MIN_PEERS &&
       dynamic_cast<const Data*>(response->get("peers"))) {
      Data* peers = (Data*)response->get("peers");
      if(peers != NULL && peers->getLen() > 0) {
	for(int i = 0; i < peers->getLen(); i += 6) {
	  unsigned int ipaddr1 = (unsigned char)*(peers->getData()+i);
	  unsigned int ipaddr2 = (unsigned char)*(peers->getData()+i+1);
	  unsigned int ipaddr3 = (unsigned char)*(peers->getData()+i+2);
	  unsigned int ipaddr4 = (unsigned char)*(peers->getData()+i+3);
	  unsigned int port = ntohs(*(unsigned short int*)(peers->getData()+i+4));
	  char ipaddr[16];
	  
	  snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%d",
		   ipaddr1, ipaddr2, ipaddr3, ipaddr4);
	  PeerHandle peer =
	    PeerHandle(new Peer(ipaddr, port, e->torrentMan->pieceLength,
				e->torrentMan->getTotalLength()));
	  if(e->torrentMan->addPeer(peer)) {
	    logger->debug("CUID#%d - Adding peer %s:%d",
			  cuid, peer->ipaddr.c_str(), peer->port);
	  }
	}
      } else {
	logger->info("CUID#%d - No peer list received.", cuid);
      }
      while(e->torrentMan->isPeerAvailable() &&
	    e->torrentMan->connections < MIN_PEERS) {
	PeerHandle peer = e->torrentMan->getPeer();
	int newCuid =  e->torrentMan->getNewCuid();
	peer->cuid = newCuid;
	PeerInitiateConnectionCommand* command =
	  new PeerInitiateConnectionCommand(newCuid, peer, e);
	e->commands.push_back(command);
	logger->debug("CUID#%d - Adding new command CUID#%d", cuid, newCuid);
      }
    }
    if(e->torrentMan->req->getTrackerEvent() == Request::STARTED) {
      e->torrentMan->req->setTrackerEvent(Request::AUTO);
    }
  } catch(Exception* err) {
    logger->error("CUID#%d - Error occurred while processing tracker response.", cuid, err);
    delete err;
  }
  if(trackerResponse != NULL) {
    delete [] trackerResponse;
  }
  e->torrentMan->trackers = 0;

  e->segmentMan->init();

  if(e->torrentMan->isHalt()) {
    return true;
  } else {
    return prepareForRetry();
  }
}

