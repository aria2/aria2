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
#include "SleepCommand.h"
#include "Util.h"

#define WAIT_SEC 5

SleepCommand::SleepCommand(int cuid, DownloadEngine* e, Command* nextCommand):
  Command(cuid), engine(e), nextCommand(nextCommand) {
  gettimeofday(&checkPoint, NULL);  
}

SleepCommand::~SleepCommand() {
  if(nextCommand != NULL) {
    delete(nextCommand);
  }
}

bool SleepCommand::execute() {
  struct timeval now;
  gettimeofday(&now, NULL);
  if(Util::difftv(now, checkPoint) >= WAIT_SEC*1000000) {
    engine->commands.push(nextCommand);
    nextCommand = NULL;
    return true;
  } else {
    engine->commands.push(this);
    return false;
  }
}

