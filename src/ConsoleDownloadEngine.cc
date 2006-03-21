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
  cout << "\r                                                                            ";
  cout << "\rProgress " <<
    Util::llitos(currentSize, true) << " Bytes/" <<
    Util::llitos(totalSize, true) << " Bytes " <<
    (totalSize == 0 ? 0 : (currentSize*100)/totalSize) << "% " <<
    speed/1000.0 << "KB/s " <<
    "(" << commands.size() << " connections)" << flush;
}

void ConsoleDownloadEngine::initStatistics() {
  cp.tv_sec = cp.tv_usec = 0;
  speed = 0;
  psize = 0;
}

void ConsoleDownloadEngine::calculateStatistics() {
  long long int dlSize = segmentMan->getDownloadedSize();
  struct timeval now;
  gettimeofday(&now, NULL);
  if(cp.tv_sec == 0 && cp.tv_usec == 0) {
    cp = now;
    psize = dlSize;
  } else {
    long long int elapsed = Util::difftv(now, cp);
    if(elapsed >= 500000) {
      int nspeed = (int)((dlSize-psize)/(elapsed/1000000.0));
      speed = (nspeed+speed)/2;
      cp = now;
      psize = dlSize;
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
