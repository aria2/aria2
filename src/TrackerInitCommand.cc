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
#include "TrackerInitCommand.h"
#include "Request.h"
#include "InitiateConnectionCommandFactory.h"
#include "TorrentMan.h"
#include "Util.h"

TrackerInitCommand::TrackerInitCommand(int cuid, Request* req, TorrentDownloadEngine* e):Command(cuid), req(req), e(e) {}

TrackerInitCommand::~TrackerInitCommand() {}

bool TrackerInitCommand::execute() {
  if(e->torrentMan->downloadComplete()) {
    if(req->getTrackerEvent() == Request::COMPLETED) {
      req->setTrackerEvent(Request::AFTER_COMPLETED);
    } else {
      req->setTrackerEvent(Request::COMPLETED);
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
    "uploaded="+Util::llitos(e->torrentMan->getUploadedSize())+"&"+
    "downloaded="+Util::llitos(e->torrentMan->getDownloadedSize())+"&"+
    "left="+(e->torrentMan->totalSize-e->torrentMan->getDownloadedSize() <= 0
	     ? "0" : Util::llitos(e->torrentMan->totalSize-e->torrentMan->getDownloadedSize()))+"&"+
    "compact=1";
  if(!event.empty()) {
    url += string("&")+"event="+event;
  }
  if(!e->torrentMan->trackerId.empty()) {
    url += string("&")+"trackerid="+e->torrentMan->trackerId;
  }
  req->setUrl(url);
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, e);
  e->commands.push(command);
  return true;
}

