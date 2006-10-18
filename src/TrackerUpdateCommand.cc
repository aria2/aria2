/* <!-- copyright */
/*
 * aria2 - The high speed download utility
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
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
#include "DelegatingPeerListProcessor.h"

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
    const MetaEntry* peersEntry = response->get("peers");
    if(!e->torrentMan->isHalt() &&
       e->torrentMan->connections < MIN_PEERS &&
       peersEntry) {
      DelegatingPeerListProcessor proc(e->torrentMan->pieceLength,
				       e->torrentMan->getTotalLength());
      Peers peers = proc.extractPeer(peersEntry);
      e->torrentMan->addPeer(peers);

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
    if(!peersEntry) {
      logger->info("CUID#%d - No peer list received.", cuid);
    }
    e->torrentMan->announceList.announceSuccess();
    e->torrentMan->trackers = 0;
    e->segmentMan->init();
  } catch(Exception* err) {
    logger->error("CUID#%d - Error occurred while processing tracker response.", cuid, err);
    e->segmentMan->errors++;
    delete err;
  }
  if(trackerResponse) {
    delete [] trackerResponse;
  }
  if(e->torrentMan->isHalt()) {
    return true;
  } else {
    return prepareForRetry();
  }
}

