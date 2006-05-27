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
#include "TorrentAutoSaveCommand.h"
#include "Util.h"

TorrentAutoSaveCommand::TorrentAutoSaveCommand(int cuid, TorrentDownloadEngine* e, int interval):Command(cuid), e(e), interval(interval) {
  checkPoint.tv_sec = 0;
  checkPoint.tv_usec = 0;
}

TorrentAutoSaveCommand::~TorrentAutoSaveCommand() {}

bool TorrentAutoSaveCommand::execute() {
  if(e->torrentMan->downloadComplete() || e->torrentMan->isHalt()) {
    return true;
  }
  struct timeval now;
  gettimeofday(&now, NULL);
  if(Util::difftvsec(now, checkPoint) >= interval) {
    checkPoint = now;
    e->torrentMan->save();
    if(e->torrentMan->isHalt()) {
      return true;
    }
  }
  e->commands.push_back(this);
  return false;
}
