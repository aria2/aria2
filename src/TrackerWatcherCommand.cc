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
#include "TrackerWatcherCommand.h"
#include "InitiateConnectionCommandFactory.h"
#include "Util.h"
#include "SleepCommand.h"
#include "prefs.h"

TrackerWatcherCommand::TrackerWatcherCommand(int cuid,
					     TorrentDownloadEngine* e,
					     int interval):
  Command(cuid), e(e), interval(interval) {
  // to force requesting to a tracker first time.
  checkPoint.setTimeInSec(0);
}

TrackerWatcherCommand::~TrackerWatcherCommand() {}

bool TrackerWatcherCommand::execute() {
  if(e->segmentMan->errors > 0) {
    if(e->torrentMan->isHalt()) {
      return true;
    }
    // we assume the tracker request has failed.
    e->torrentMan->trackers = 0;
    e->segmentMan->init();
    // sleep a few seconds.
    SleepCommand* sleepCommand =
      new SleepCommand(cuid, e, this, e->option->getAsInt(PREF_RETRY_WAIT));
    e->commands.push_back(sleepCommand);
    return false;
  }
  if(e->torrentMan->trackers == 0 &&
     (e->torrentMan->isHalt() ||
      checkPoint.elapsed(interval))) {
    checkPoint.reset();
    e->torrentMan->req->resetTryCount();
    int numWant = 50;
    if(e->torrentMan->connections >= MIN_PEERS || e->torrentMan->isHalt()) {
      numWant = 0;
    }
    if(e->torrentMan->isHalt()) {
      e->torrentMan->req->setTrackerEvent(Request::STOPPED);
    } else if(e->torrentMan->downloadComplete()) {
      if(e->torrentMan->req->getTrackerEvent() == Request::COMPLETED) {
	e->torrentMan->req->setTrackerEvent(Request::AFTER_COMPLETED);
      } else {
	if(e->torrentMan->req->getTrackerEvent() == Request::STARTED) {
	  // in case where download had completed when aria2c started.
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
      "info_hash="+Util::torrentUrlencode(e->torrentMan->getInfoHash(), 20)+"&"+
      "peer_id="+e->torrentMan->peerId+"&"+
      "port="+Util::itos(e->torrentMan->getPort())+"&"+
      "uploaded="+Util::llitos(e->torrentMan->getSessionUploadLength())+"&"+
      "downloaded="+Util::llitos(e->torrentMan->getSessionDownloadLength())+"&"+
      "left="+(e->torrentMan->getTotalLength()-e->torrentMan->getDownloadLength() <= 0
	       ? "0" : Util::llitos(e->torrentMan->getTotalLength()-e->torrentMan->getDownloadLength()))+"&"+
      "compact=1"+"&"+
      "key="+e->torrentMan->key+"&"+
      "numwant="+Util::itos(numWant)+"&"+
      "no_peer_id=1";
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
    logger->info("CUID#%d - Creating new tracker request command #%d", cuid,
		 command->getCuid());
    if(e->torrentMan->isHalt()) {
      return true;
    }
  }
  interval = e->torrentMan->minInterval;
  e->commands.push_back(this);
  return false;
}
