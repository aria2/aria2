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
#include "BtRegistry.h"

TorrentDownloadEngine::TorrentDownloadEngine():
  filenameFixed(false),
  btContext(0),
  btRuntime(0),
  pieceStorage(0),
  peerStorage(0),
  btAnnounce(0),
  btProgressInfoFile(0) {}

TorrentDownloadEngine::~TorrentDownloadEngine() {
}

void TorrentDownloadEngine::setBtContext(const BtContextHandle& btContext) {
  this->btContext = btContext;
  btRuntime = BT_RUNTIME(btContext);
  pieceStorage = PIECE_STORAGE(btContext);
  peerStorage = PEER_STORAGE(btContext);
  btAnnounce = BT_ANNOUNCE(btContext);
  btProgressInfoFile = BT_PROGRESS_INFO_FILE(btContext);
}

void TorrentDownloadEngine::onEndOfRun() {
  pieceStorage->getDiskAdaptor()->closeFile();
  if(pieceStorage->downloadFinished()) {
    btProgressInfoFile->removeFile();
  } else {
    btProgressInfoFile->save();
  }
}

void TorrentDownloadEngine::initStatistics() {
  downloadSpeed = 0;
  uploadSpeed = 0;
  cp.reset();
  startup.reset();
  eta = 0;
  avgSpeed = 0;
  downloadLength = 0;
  uploadLength = 0;
  totalLength = 0;
}

int TorrentDownloadEngine::calculateSpeed(long long int length, int elapsed) {
  int nowSpeed = (int)(length/elapsed);
  return nowSpeed;
}

void TorrentDownloadEngine::calculateStat() {
  TransferStat stat = peerStorage->calculateStat();

  if(pieceStorage->isSelectiveDownloadingMode()) {
    downloadLength = pieceStorage->getFilteredCompletedLength();
    totalLength = pieceStorage->getFilteredTotalLength();
  } else {
    downloadLength = pieceStorage->getCompletedLength();
    totalLength = pieceStorage->getTotalLength();
  }
  uploadLength = stat.getSessionUploadLength()+
    btRuntime->getUploadLengthAtStartup();

  downloadSpeed = stat.getDownloadSpeed();
  uploadSpeed = stat.getUploadSpeed();
  avgSpeed = calculateSpeed(stat.getSessionDownloadLength(),
			    startup.difference());
  if(avgSpeed < 0) {
    avgSpeed = 0;
  } else if(avgSpeed != 0) {
    eta = (totalLength-downloadLength)/avgSpeed;
  }
}

void TorrentDownloadEngine::calculateStatistics() {
  if(cp.difference() >= 1) {
    calculateStat();
    sendStatistics();
    cp.reset();
  }
}
