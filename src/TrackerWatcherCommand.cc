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
  Command* command = 0;
  if(e->torrentMan->trackers == 0 && e->torrentMan->isHalt()) {
    // Download is going to halt.
    // Check whether there are at least one tracker which can receive
    // "stopped" event.
    if(e->torrentMan->announceList.countStoppedAllowedTier()) {
      e->torrentMan->announceList.moveToStoppedAllowedTier();
      e->torrentMan->announceList.setEvent(AnnounceTier::STOPPED);
      command = createRequestCommand();
    } else {
      // We don't send "stopped" event since no tracker cares about it.
      return true;
    }
  } else if(e->torrentMan->trackers == 0 &&
	    e->torrentMan->downloadComplete() &&
	    e->torrentMan->announceList.countCompletedAllowedTier()) {
    // Send "completed" event to all trackers which can accept it.
    e->torrentMan->announceList.moveToCompletedAllowedTier();
    e->torrentMan->announceList.setEvent(AnnounceTier::COMPLETED);
    command = createRequestCommand();
  } else if(e->torrentMan->trackers == 0 &&
	    checkPoint.elapsed(interval)) {
    checkPoint.reset();
    // If download completed before "started" event is sent to a tracker,
    // we change the event to something else to prevent us from
    // sending "completed" event.
    if(e->torrentMan->downloadComplete() &&
       e->torrentMan->announceList.getEvent() == AnnounceTier::STARTED) {
      e->torrentMan->announceList.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
    }
    command = createRequestCommand();
  } else if(e->segmentMan->errors > 0) {
    e->torrentMan->trackerNumTry++;
    checkPoint.reset();
    // we assume the tracker request has failed.
    e->torrentMan->announceList.announceFailure();
    e->torrentMan->trackers = 0;
    e->segmentMan->init();
    if(e->torrentMan->trackerNumTry >= e->option->getAsInt(PREF_TRACKER_MAX_TRIES)) {
      // abort tracker request
      e->torrentMan->trackerNumTry = 0;
      if(e->torrentMan->isHalt()) {
	return true;
      }
    } else {
      // sleep a few seconds.
      command =
	new SleepCommand(cuid, e,
			 createRequestCommand(),
			 e->option->getAsInt(PREF_RETRY_WAIT));
    }
  }

  if(command) {
    e->commands.push_back(command);
    e->torrentMan->trackers++;
  }
  // updates interval with newest minInterval
  interval = e->torrentMan->minInterval;
  e->commands.push_back(this);
  return false;
}

Command* TrackerWatcherCommand::createRequestCommand() {
  int numWant = 50;
  if(e->torrentMan->connections >= MIN_PEERS || e->torrentMan->isHalt()) {
    numWant = 0;
  }
  string url = e->torrentMan->announceList.getAnnounce()+"?"+
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
  string event = e->torrentMan->announceList.getEventString();
  if(!event.empty()) {
    url += string("&")+"event="+event;
  }
  if(!e->torrentMan->trackerId.empty()) {
    url += string("&")+"trackerid="+e->torrentMan->trackerId;
  }
  RequestHandle req;
  req->setUrl(url);
  req->isTorrent = true;
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(e->torrentMan->getNewCuid(), req, e);
  logger->info("CUID#%d - Creating new tracker request command #%d", cuid,
	       command->getCuid());
  return command;
}
