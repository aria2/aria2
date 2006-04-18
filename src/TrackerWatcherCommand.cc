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

TrackerWatcherCommand::TrackerWatcherCommand(int cuid, Request* req,
					     TorrentDownloadEngine* e):
  Command(cuid), req(req), e(e) {
}

TrackerWatcherCommand::~TrackerWatcherCommand() {}

bool TrackerWatcherCommand::execute() {
  if(e->torrentMan->trackers == 0) {
    req->resetTryCount();
    
    if(e->torrentMan->downloadComplete()) {
      if(req->getTrackerEvent() == Request::COMPLETED) {
	req->setTrackerEvent(Request::AFTER_COMPLETED);
      } else {
	if(req->getTrackerEvent() == Request::STARTED) {
	  req->setTrackerEvent(Request::AFTER_COMPLETED);
	} else if(req->getTrackerEvent() != Request::AFTER_COMPLETED) {
	  req->setTrackerEvent(Request::COMPLETED);
	}
      }
    }
    string event;
    switch(req->getTrackerEvent()) {
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
      "key="+e->torrentMan->peerId;
    if(!event.empty()) {
      url += string("&")+"event="+event;
    }
    if(!e->torrentMan->trackerId.empty()) {
      url += string("&")+"trackerid="+e->torrentMan->trackerId;
    }
    req->setUrl(url);
    Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(e->torrentMan->getNewCuid(), req, e);
    e->commands.push(command);
    e->torrentMan->trackers++;
    logger->info("CUID#%d - creating new tracker request command #%d", cuid,
		 command->getCuid());
  }
  SleepCommand* slpCommand = new SleepCommand(cuid, e, this,
					      e->torrentMan->minInterval);
  e->commands.push(slpCommand);
  return false;
}
