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
#include "TorrentDownloadEngine.h"
#include "Util.h"

void TorrentDownloadEngine::onEndOfRun() {
  torrentMan->diskAdaptor->closeFile();
  if(filenameFixed && torrentMan->downloadComplete()) {
    torrentMan->remove();
  } else {
    torrentMan->save();
  }
}

void TorrentDownloadEngine::afterEachIteration() {
  if(!filenameFixed && torrentMan->downloadComplete()) {
    if(torrentMan->isSelectiveDownloadingMode()) {
      onSelectiveDownloadingCompletes();
    }
    logger->info("The download was complete.");
    torrentMan->onDownloadComplete();
    if(torrentMan->downloadComplete()) {
      filenameFixed = true;
    }
  }
}

void TorrentDownloadEngine::initStatistics() {
  downloadSpeed = 0;
  uploadSpeed = 0;
  lastElapsed = 0;
  gettimeofday(&cp[0], NULL);
  gettimeofday(&cp[1], NULL);
  gettimeofday(&startup, NULL);
  sessionDownloadLengthArray[0] = 0;
  sessionDownloadLengthArray[1] = 0;
  sessionUploadLengthArray[0] = 0;
  sessionUploadLengthArray[1] = 0;
  currentCp = 0;
  eta = 0;
  avgSpeed = 0;
  sessionDownloadLength = 0;
  downloadLength = 0;
  totalLength = 0;
  if(torrentMan->isSelectiveDownloadingMode()) {
    selectedDownloadLengthDiff = torrentMan->getDownloadLength()-torrentMan->getCompletedLength();
    selectedTotalLength = torrentMan->getSelectedTotalLength();
  }
}

int TorrentDownloadEngine::calculateSpeed(long long int sessionLength, int elapsed) {
  int nowSpeed = (int)(sessionLength/elapsed);
  return nowSpeed;
}

void TorrentDownloadEngine::calculateStatistics() {
  struct timeval now;
  gettimeofday(&now, NULL);
  int elapsed = Util::difftvsec(now, cp[currentCp]);

  sessionDownloadLengthArray[0] += torrentMan->getDeltaDownloadLength();
  sessionUploadLengthArray[0] += torrentMan->getDeltaUploadLength();
  sessionDownloadLengthArray[1] += torrentMan->getDeltaDownloadLength();
  sessionUploadLengthArray[1] += torrentMan->getDeltaUploadLength();

  sessionDownloadLength += torrentMan->getDeltaDownloadLength();


  torrentMan->resetDeltaDownloadLength();
  torrentMan->resetDeltaUploadLength();

  if(torrentMan->isSelectiveDownloadingMode()) {
    downloadLength = torrentMan->getDownloadLength()-selectedDownloadLengthDiff;
    totalLength = selectedTotalLength;
  } else {
    downloadLength = torrentMan->getDownloadLength();
    totalLength = torrentMan->getTotalLength();
  }
  
  downloadSpeed = calculateSpeed(sessionDownloadLengthArray[currentCp], elapsed);
  uploadSpeed = calculateSpeed(sessionUploadLengthArray[currentCp], elapsed);

  if(elapsed-lastElapsed >= 1) {
    avgSpeed = calculateSpeed(sessionDownloadLength,
			      Util::difftvsec(now, startup));
    if(avgSpeed < 0) {
      avgSpeed = 0;
    } else if(avgSpeed != 0) {
      eta = (totalLength-downloadLength)/avgSpeed;
    }

    sendStatistics();
    lastElapsed = elapsed;
  }

  if(elapsed > 15) {
    sessionDownloadLengthArray[currentCp] = 0;
    sessionUploadLengthArray[currentCp] = 0;
    cp[currentCp] = now;
    lastElapsed = 0;
    currentCp = currentCp ? 0 : 1;
  }
}
