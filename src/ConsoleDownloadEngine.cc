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
#include "ConsoleDownloadEngine.h"
#include "Util.h"

ConsoleDownloadEngine::ConsoleDownloadEngine() {}

ConsoleDownloadEngine::~ConsoleDownloadEngine() {}

void ConsoleDownloadEngine::sendStatistics(long long int currentSize, long long int totalSize) {
  printf("\r                                                                             ");
  printf("\r");
  printf("%s/%s Bytes %d%% %s %.2f KB/s %d connections",
	 Util::llitos(currentSize, true).c_str(),
	 Util::llitos(totalSize, true).c_str(),
	 (totalSize == 0 ? 0 : (int)((currentSize*100)/totalSize)),
	 avgSpeed == 0 ? "-" : Util::secfmt(eta).c_str(),
	 speed/1024.0,
	 commands.size());
  fflush(stdout);
}

void ConsoleDownloadEngine::initStatistics() {
  cp.tv_sec = cp.tv_usec = 0;
  speed = 0;
  psize = 0;
  avgSpeed = 0;
  eta = 0;
  startupLength = 0;
  isStartupLengthSet = false;
  gettimeofday(&startup, NULL);
}

void ConsoleDownloadEngine::calculateStatistics() {
  long long int dlSize = segmentMan->getDownloadedSize();
  if(!isStartupLengthSet && dlSize > 0) {
    startupLength = dlSize;
    isStartupLengthSet = true;
  }
  struct timeval now;
  gettimeofday(&now, NULL);
  if(cp.tv_sec == 0 && cp.tv_usec == 0) {
    cp = now;
    psize = dlSize;
  } else {
    int elapsed = Util::difftvsec(now, cp);
    if(elapsed >= 1) {
      int nspeed = (int)((dlSize-psize)/elapsed);
      speed = (nspeed+speed)/2;
      cp = now;
      psize = dlSize;

      avgSpeed = (int)((dlSize-startupLength)/Util::difftvsec(now, startup));
      if(avgSpeed < 0) {
	avgSpeed = 0;
      } else if(avgSpeed != 0 && segmentMan->totalSize > 0) {
	eta = (segmentMan->totalSize-dlSize)/avgSpeed;
      }

      sendStatistics(dlSize, segmentMan->totalSize);
    }
  }
}

void ConsoleDownloadEngine::onEndOfRun() {
  diskWriter->closeFile();
  if(segmentMan->finished()) {
    segmentMan->remove();
  } else {
    segmentMan->save();
  }
}
