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
#include "RequestFactory.h"

TrackerWatcherCommand::TrackerWatcherCommand(int cuid,
					     TorrentDownloadEngine* e,
					     const BtContextHandle& btContext):
  BtContextAwareCommand(cuid, btContext), e(e)
{
}

TrackerWatcherCommand::~TrackerWatcherCommand() {}


bool TrackerWatcherCommand::execute() {
  if(btAnnounce->noMoreAnnounce()) {
    return true;
  }
  Command* command = createCommand();
  if(command) {
    e->commands.push_back(command);
  }
  e->commands.push_back(this);
  return false;
}

Command* TrackerWatcherCommand::createCommand() {
  Command* command = 0;
  if(btAnnounce->isAnnounceReady()) {
    command = createRequestCommand(btAnnounce->getAnnounceUrl());
    btAnnounce->announceStart(); // inside it, trackers++.
  } else if(e->segmentMan->errors > 0) {
    btAnnounce->announceFailure(); // inside it, trackers = 0.
    e->segmentMan->init();
    if(btAnnounce->isAllAnnounceFailed()) {
      btAnnounce->resetAnnounce();
      // sleep a few seconds.
      command =
	new SleepCommand(cuid, e,
			 createRequestCommand(btAnnounce->getAnnounceUrl()),
			 e->option->getAsInt(PREF_RETRY_WAIT));
    }
  }
  return command;
}

Command* TrackerWatcherCommand::createRequestCommand(const string& url)
{
  RequestHandle req = RequestFactorySingletonHolder::instance()->createRequest();
  req->setUrl(url);
  req->isTorrent = true;
  Command* command =
    InitiateConnectionCommandFactory::createInitiateConnectionCommand(btRuntime->getNewCuid(), req, e);
  logger->info("CUID#%d - Creating new tracker request command #%d", cuid,
	       command->getCuid());
  return command;
}
