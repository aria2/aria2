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
#include "TorrentDownloadEngine.h"
#include "Util.h"

TorrentDownloadEngine::TorrentDownloadEngine():filenameFixed(false),
					       torrentMan(NULL) {}

TorrentDownloadEngine::~TorrentDownloadEngine() {
  if(torrentMan != NULL) {
    delete torrentMan;
  }
}

void TorrentDownloadEngine::onEndOfRun() {
  torrentMan->diskAdaptor->closeFile();
  if(torrentMan->downloadComplete()) {
    torrentMan->remove();
  } else {
    torrentMan->save();
  }
}

void TorrentDownloadEngine::initStatistics() {
  downloadSpeed = 0;
  uploadSpeed = 0;
  cp.reset();
  lastCalcStat.reset();
  startup.reset();
  eta = 0;
  avgSpeed = 0;
  downloadLength = 0;
  totalLength = 0;
  if(torrentMan->isSelectiveDownloadingMode()) {
    selectedDownloadLengthDiff = torrentMan->getDownloadLength()-torrentMan->getCompletedLength();
    selectedTotalLength = torrentMan->getSelectedTotalLength();
  }
}

int TorrentDownloadEngine::calculateSpeed(long long int length, int elapsed) {
  int nowSpeed = (int)(length/elapsed);
  return nowSpeed;
}

void TorrentDownloadEngine::calculateStat() {
  TransferStat stat = torrentMan->calculateStat();
  downloadSpeed = stat.downloadSpeed;
  uploadSpeed = stat.uploadSpeed;
  avgSpeed = calculateSpeed(stat.sessionDownloadLength, startup.difference());
  if(avgSpeed < 0) {
    avgSpeed = 0;
  } else if(avgSpeed != 0) {
    eta = (totalLength-downloadLength)/avgSpeed;
  }
}

void TorrentDownloadEngine::calculateStatistics() {
  if(torrentMan->isSelectiveDownloadingMode()) {
    downloadLength = torrentMan->getDownloadLength()-selectedDownloadLengthDiff;
    totalLength = selectedTotalLength;
  } else {
    downloadLength = torrentMan->getDownloadLength();
    totalLength = torrentMan->getTotalLength();
  }
  
  Time now;
  if(now.getTimeInMillis()-lastCalcStat.getTimeInMillis() >= 1000) {
    calculateStat();
    lastCalcStat = now;
  }
  if(cp.difference() >= 1) {
    sendStatistics();
    cp.reset();
  }
}
