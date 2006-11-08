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
#include "ConsoleDownloadEngine.h"
#include "Util.h"
#include <signal.h>

volatile sig_atomic_t haltRequested = 0;

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
  cp.reset();
  startup.reset();
  speed = 0;
  psize = 0;
  avgSpeed = 0;
  eta = 0;
  startupLength = 0;
  isStartupLengthSet = false;
}

void ConsoleDownloadEngine::calculateStatistics() {
  long long int dlSize = segmentMan->getDownloadLength();
  if(!isStartupLengthSet && dlSize > 0) {
    startupLength = dlSize;
    psize = dlSize;
    isStartupLengthSet = true;
  }
  int elapsed = cp.difference();
  if(elapsed >= 1) {
    int nspeed = (int)((dlSize-psize)/elapsed);
    speed = (nspeed+speed)/2;
    cp.reset();
    psize = dlSize;

    int elapsedFromStartup = startup.difference();
    if(elapsedFromStartup > 0) {
      avgSpeed = (int)((dlSize-startupLength)/elapsedFromStartup);
    }
    if(avgSpeed < 0) {
      avgSpeed = 0;
    } else if(avgSpeed != 0 && segmentMan->totalSize > 0) {
      eta = (segmentMan->totalSize-dlSize)/avgSpeed;
    }
    
    sendStatistics(dlSize, segmentMan->totalSize);
  }
}

void ConsoleDownloadEngine::onEndOfRun() {
  segmentMan->diskWriter->closeFile();
  if(segmentMan->finished()) {
    segmentMan->remove();
  } else {
    segmentMan->save();
  }
}

void ConsoleDownloadEngine::afterEachIteration() {
  if(haltRequested) {
    printf(_("\nstopping application...\n"));
    fflush(stdout);
    segmentMan->save();
    segmentMan->diskWriter->closeFile();
    printf(_("done\n"));
    exit(EXIT_SUCCESS);
  }
}
