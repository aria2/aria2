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
#include "TrackerWatcherCommand.h"
#include "SleepCommand.h"
#include "InitiateConnectionCommandFactory.h"
#include "Util.h"

TrackerWatcherCommand::TrackerWatcherCommand(int cuid,
					     TorrentDownloadEngine* e,
					     int interval):
  Command(cuid), e(e), interval(interval) {
  checkPoint.tv_sec = 0;
  checkPoint.tv_usec = 0;
}

TrackerWatcherCommand::~TrackerWatcherCommand() {}

bool TrackerWatcherCommand::execute() {
  struct timeval now;
  gettimeofday(&now, NULL);
  
  if(e->torrentMan->trackers == 0 &&
     Util::difftvsec(now, checkPoint) >= interval) {
    checkPoint = now;
    e->torrentMan->req->resetTryCount();
    int numWant = 50;
    if(e->torrentMan->connections >= MIN_PEERS) {
      numWant = 0;
    }    
    if(e->torrentMan->downloadComplete()) {
      if(e->torrentMan->req->getTrackerEvent() == Request::COMPLETED) {
	e->torrentMan->req->setTrackerEvent(Request::AFTER_COMPLETED);
      } else {
	if(e->torrentMan->req->getTrackerEvent() == Request::STARTED) {
	  e->torrentMan->req->setTrackerEvent(Request::AFTER_COMPLETED);
	} else if(e->torrentMan->req->getTrackerEvent() != Request::AFTER_COMPLETED) {
	  e->torrentMan->req->setTrackerEvent(Request::COMPLETED);
	}
      }
    }
    string event;
    switch(e->torrentMan->req->getTrackerEvent()) {
    case Request::STARTED:
      event = "started";
      break;
    case Request::STOPPED:
      event = "stopped";
      break;
    case Request::COMPLETED:
      event = "completed";
      break;
    }
    string url = e->torrentMan->announce+"?"+
      "info_hash="+Util::urlencode(e->torrentMan->getInfoHash(), 20)+"&"+
      "peer_id="+e->torrentMan->peerId+"&"+
      "port="+Util::itos(e->torrentMan->getPort())+"&"+
      "uploaded="+Util::llitos(e->torrentMan->getSessionUploadLength())+"&"+
      "downloaded="+Util::llitos(e->torrentMan->getSessionDownloadLength())+"&"+
      "left="+(e->torrentMan->getTotalLength()-e->torrentMan->getDownloadLength() <= 0
	       ? "0" : Util::llitos(e->torrentMan->getTotalLength()-e->torrentMan->getDownloadLength()))+"&"+
      "compact=1"+"&"+
      "key="+e->torrentMan->peerId+"&"+
      "numwant="+Util::itos(numWant);
    if(!event.empty()) {
      url += string("&")+"event="+event;
    }
    if(!e->torrentMan->trackerId.empty()) {
      url += string("&")+"trackerid="+e->torrentMan->trackerId;
    }
    e->torrentMan->req->setUrl(url);
    Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(e->torrentMan->getNewCuid(), e->torrentMan->req, e);
    e->commands.push_back(command);
    e->torrentMan->trackers++;
    logger->info("CUID#%d - creating new tracker request command #%d", cuid,
		 command->getCuid());
  }
  interval = e->torrentMan->minInterval;
  e->commands.push_back(this);
  return false;
}
