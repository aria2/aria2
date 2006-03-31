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
#include "TorrentConsoleDownloadEngine.h"
#include "Util.h"

TorrentConsoleDownloadEngine::TorrentConsoleDownloadEngine() {}

TorrentConsoleDownloadEngine::~TorrentConsoleDownloadEngine() {}

void TorrentConsoleDownloadEngine::printStatistics() {
  printf("\r                                                                             ");
  printf("\r");
  if(torrentMan->downloadComplete()) {
    printf("Download Completed ");
  } else {
    printf("%s/%sB %d%% DW:%.2f",
	   Util::llitos(torrentMan->getDownloadLength(), true).c_str(),
	   Util::llitos(torrentMan->getTotalLength(), true).c_str(),
	   (torrentMan->getTotalLength() == 0 ?
	    0 : (int)((torrentMan->getDownloadLength()*100)/torrentMan->getTotalLength())),
	   downloadSpeed/1000.0);
  }
  printf(" UP:%.2f(%s) %dpeers",
	 uploadSpeed/1000.0,
	 Util::llitos(torrentMan->getUploadLength(), true).c_str(),
	 torrentMan->connections);
  fflush(stdout);	 
}

void TorrentConsoleDownloadEngine::initStatistics() {
  downloadSpeed = 0;
  uploadSpeed = 0;
  lastElapsed = 0;
  gettimeofday(&cp[0], NULL);
  gettimeofday(&cp[1], NULL);
  sessionDownloadLengthArray[0] = 0;
  sessionDownloadLengthArray[1] = 0;
  sessionUploadLengthArray[0] = 0;
  sessionUploadLengthArray[1] = 0;
  currentCp = 0;
}

int TorrentConsoleDownloadEngine::calculateSpeed(long long int sessionLength, long long int elapsed) {
  int nowSpeed = (int)(sessionLength/(elapsed/1000000.0));
  return nowSpeed;
}

void TorrentConsoleDownloadEngine::calculateStatistics() {
  struct timeval now;
  gettimeofday(&now, NULL);
  long long int elapsed = Util::difftv(now, cp[currentCp]);

  sessionDownloadLengthArray[0] += torrentMan->getDeltaDownloadLength();
  sessionUploadLengthArray[0] += torrentMan->getDeltaUploadLength();
  sessionDownloadLengthArray[1] += torrentMan->getDeltaDownloadLength();
  sessionUploadLengthArray[1] += torrentMan->getDeltaUploadLength();

  downloadSpeed = calculateSpeed(sessionDownloadLengthArray[currentCp], elapsed);
  uploadSpeed = calculateSpeed(sessionUploadLengthArray[currentCp], elapsed);

  torrentMan->resetDeltaDownloadLength();
  torrentMan->resetDeltaUploadLength();

  if(elapsed-lastElapsed >= 1000000) {
    printStatistics();
    lastElapsed = elapsed;
  }

  if(elapsed > 15*1000000) {
    sessionDownloadLengthArray[currentCp] = 0;
    sessionUploadLengthArray[currentCp] = 0;
    cp[currentCp] = now;
    lastElapsed = 0;
    currentCp = currentCp ? 0 : 1;
  }
}
