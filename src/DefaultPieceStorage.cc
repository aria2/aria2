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
#include "DefaultPieceStorage.h"
#include "LogFactory.h"
#include "prefs.h"
#include "DirectDiskAdaptor.h"
#include "MultiDiskAdaptor.h"
#include "CopyDiskAdaptor.h"
#include "DefaultDiskWriter.h"
#include "MultiDiskWriter.h"
#include "PreAllocationDiskWriter.h"
#include "DlAbortEx.h"
#include "BitfieldManFactory.h"

DefaultPieceStorage::DefaultPieceStorage(BtContextHandle btContext, const Option* option):
  btContext(btContext),
  diskAdaptor(0),
  endGamePieceNum(END_GAME_PIECE_NUM),
  option(option)
{
  bitfieldMan =
    BitfieldManFactory::getNewFactory()->
    createBitfieldMan(btContext->getPieceLength(),
		      btContext->getTotalLength());
  logger = LogFactory::getInstance();
}

DefaultPieceStorage::~DefaultPieceStorage() {
  delete bitfieldMan;
  delete diskAdaptor;
}

bool DefaultPieceStorage::hasMissingPiece(const PeerHandle& peer) {
  return bitfieldMan->hasMissingPiece(peer->getBitfield(),
				      peer->getBitfieldLength());
}

bool DefaultPieceStorage::isEndGame() {
  return bitfieldMan->countMissingBlock() <= endGamePieceNum;
}

int DefaultPieceStorage::getMissingPieceIndex(const PeerHandle& peer) {
  int index = -1;
  if(isEndGame()) {
    index = bitfieldMan->getMissingIndex(peer->getBitfield(),
					 peer->getBitfieldLength());
  } else {
    index = bitfieldMan->getMissingUnusedIndex(peer->getBitfield(),
					       peer->getBitfieldLength());
  }
  return index;
}

PieceHandle DefaultPieceStorage::checkOutPiece(int index) {
  if(index == -1) {
    return 0;
  }
  bitfieldMan->setUseBit(index);

  PieceHandle piece = findUsedPiece(index);
  if(piece.isNull()) {
    piece = new Piece(index, bitfieldMan->getBlockLength(index));
    addUsedPiece(piece);
    return piece;
  } else {
    return piece;
  }
}

/**
 * Newly instantiated piece is not added to usedPieces.
 * Because it is waste of memory and there is no chance to use them later.
 */
PieceHandle DefaultPieceStorage::getPiece(int index) {
  if(0 <= index && index <= bitfieldMan->getMaxIndex()) {
    PieceHandle piece = findUsedPiece(index);
    if(piece.isNull()) {
      piece = new Piece(index, bitfieldMan->getBlockLength(index));
      if(hasPiece(index)) {
	piece->setAllBlock();
      }
    }
    return piece;
  } else {
    return 0;
  }
}

void DefaultPieceStorage::addUsedPiece(const PieceHandle& piece) {
  usedPieces.push_back(piece);
}

class FindPiece {
private:
  int index;
public:
  FindPiece(int index):index(index) {}

  bool operator()(const PieceHandle& piece) {
    return piece->getIndex() == index;
  }
};

PieceHandle DefaultPieceStorage::findUsedPiece(int index) const {
  Pieces::const_iterator itr = find_if(usedPieces.begin(),
				       usedPieces.end(),
				       FindPiece(index));
  if(itr == usedPieces.end()) {
    return 0;
  } else {
    return *itr;
  }
}

PieceHandle DefaultPieceStorage::getMissingPiece(const PeerHandle& peer) {
  int index = getMissingPieceIndex(peer);
  return checkOutPiece(index);
}

int DefaultPieceStorage::getMissingFastPieceIndex(const PeerHandle& peer) {
  int index = -1;
  if(peer->isFastExtensionEnabled() && peer->countFastSet() > 0) {
    BitfieldMan tempBitfield(bitfieldMan->getBlockLength(),
			     bitfieldMan->getTotalLength());
    for(Integers::const_iterator itr = peer->getFastSet().begin();
	itr != peer->getFastSet().end(); itr++) {
      if(!bitfieldMan->isBitSet(index) && peer->hasPiece(*itr)) {
	tempBitfield.setBit(*itr);
      }
    }
    if(isEndGame()) {
      index = bitfieldMan->getMissingIndex(tempBitfield.getBitfield(),
					   tempBitfield.getBitfieldLength());
    } else {
      index = bitfieldMan->getMissingUnusedIndex(tempBitfield.getBitfield(),
						 tempBitfield.getBitfieldLength());
    }
  }
  return index;
}

PieceHandle DefaultPieceStorage::getMissingFastPiece(const PeerHandle& peer) {
  int index = getMissingFastPieceIndex(peer);
  return checkOutPiece(index);
}

void DefaultPieceStorage::deleteUsedPiece(const PieceHandle& piece) {
  if(piece.isNull()) {
    return;
  }
  Pieces::iterator itr = find(usedPieces.begin(), usedPieces.end(), piece);
  if(itr != usedPieces.end()) {
    usedPieces.erase(itr);
  }
}

void DefaultPieceStorage::reduceUsedPieces(int delMax) {
  int toDelete = usedPieces.size()-delMax;
  if(toDelete <= 0) {
    return;
  }
  int fillRate = 10;
  while(fillRate < 50) {
    int deleted = deleteUsedPiecesByFillRate(fillRate, toDelete);
    if(deleted == 0) {
      break;
    }
    toDelete -= deleted;
    fillRate += 10;
  }
}

int DefaultPieceStorage::deleteUsedPiecesByFillRate(int fillRate,
							   int toDelete) {
  int deleted = 0;
  for(Pieces::iterator itr = usedPieces.begin();
      itr != usedPieces.end() && deleted < toDelete;) {
    PieceHandle& piece = *itr;
    if(!bitfieldMan->isUseBitSet(piece->getIndex()) &&
       piece->countCompleteBlock() <= piece->countBlock()*(fillRate/100.0)) {
      logger->debug("Deleting used piece index=%d, fillRate(%%)=%d<=%d",
		    piece->getIndex(),
		    (piece->countCompleteBlock()*100)/piece->countBlock(),
		    fillRate);
      itr = usedPieces.erase(itr);
      deleted++;
    } else {
      itr++;
    }
  }
  return deleted;
}

void DefaultPieceStorage::completePiece(const PieceHandle& piece) {
  if(piece.isNull()) {
    return;
  }
  deleteUsedPiece(piece);
  if(!isEndGame()) {
    reduceUsedPieces(100);
  }
  if(downloadFinished()) {
    return;
  }
  bitfieldMan->setBit(piece->getIndex());
  bitfieldMan->unsetUseBit(piece->getIndex());
  if(downloadFinished()) {
    diskAdaptor->onDownloadComplete();
    if(isSelectiveDownloadingMode()) {
      logger->notice(_("Download of selected files was complete."));
      finishSelectiveDownloadingMode();
    } else {
      logger->info(_("The download was complete."));
    }
  }
}

bool DefaultPieceStorage::isSelectiveDownloadingMode() {
  return bitfieldMan->isFilterEnabled();
}

void DefaultPieceStorage::finishSelectiveDownloadingMode() {
  bitfieldMan->clearFilter();
  diskAdaptor->addAllDownloadEntry();
}

// not unittested
void DefaultPieceStorage::cancelPiece(const PieceHandle& piece) {
  if(piece.isNull()) {
    return;
  }
  bitfieldMan->unsetUseBit(piece->getIndex());
  if(!isEndGame()) {
    if(piece->countCompleteBlock() == 0) {
      deleteUsedPiece(piece);
    }
  }
}

bool DefaultPieceStorage::hasPiece(int index) {
  return bitfieldMan->isBitSet(index);
}

long long int DefaultPieceStorage::getTotalLength() {
  return bitfieldMan->getTotalLength();
}

long long int DefaultPieceStorage::getFilteredTotalLength() {
  return bitfieldMan->getFilteredTotalLength();
}

long long int DefaultPieceStorage::getCompletedLength() {
  return bitfieldMan->getCompletedLength();
}

long long int DefaultPieceStorage::getFilteredCompletedLength() {
  return bitfieldMan->getFilteredCompletedLength();
}

// not unittested
void DefaultPieceStorage::setFileFilter(const Strings& filePaths) {
  if(btContext->getFileMode() != BtContext::MULTI || filePaths.empty()) {
    return;
  }
  diskAdaptor->removeAllDownloadEntry();
  for(Strings::const_iterator pitr = filePaths.begin();
      pitr != filePaths.end(); pitr++) {
    if(!diskAdaptor->addDownloadEntry(*pitr)) {
      throw new DlAbortEx("No such file entry %s", (*pitr).c_str());
    }
    FileEntryHandle fileEntry = diskAdaptor->getFileEntryFromPath(*pitr);
    bitfieldMan->addFilter(fileEntry->getOffset(), fileEntry->getLength());
  }
  bitfieldMan->enableFilter();
}

void DefaultPieceStorage::setFileFilter(const Integers& fileIndexes) {
  Strings filePaths;
  const FileEntries& entries = diskAdaptor->getFileEntries();
  for(int i = 0; i < (int)entries.size(); i++) {
    if(find(fileIndexes.begin(), fileIndexes.end(), i+1) != fileIndexes.end()) {
      logger->debug("index=%d is %s", i+1, entries.at(i)->getPath().c_str());
      filePaths.push_back(entries.at(i)->getPath());
    }
  }
  setFileFilter(filePaths);
}

// not unittested
void DefaultPieceStorage::clearFileFilter() {
  bitfieldMan->clearFilter();
  diskAdaptor->addAllDownloadEntry();
}

// not unittested
bool DefaultPieceStorage::downloadFinished() {
  return bitfieldMan->isAllBitSet();
}

// not unittested
void DefaultPieceStorage::initStorage() {
  if(diskAdaptor) {
    delete diskAdaptor;
    diskAdaptor = 0;
  }
  if(option->get(PREF_DIRECT_FILE_MAPPING) == V_TRUE) {
    if(btContext->getFileMode() == BtContext::SINGLE) {
      diskAdaptor = new DirectDiskAdaptor(new DefaultDiskWriter(btContext->getTotalLength()));
    } else {
      diskAdaptor = new MultiDiskAdaptor(new MultiDiskWriter(btContext->getPieceLength()));
      ((MultiDiskAdaptor*)diskAdaptor)->setTopDir(btContext->getName());
    }
  } else {
    diskAdaptor = new CopyDiskAdaptor(new PreAllocationDiskWriter(btContext->getTotalLength()));
    ((CopyDiskAdaptor*)diskAdaptor)->setTempFilename(btContext->getName()+".a2tmp");
    if(btContext->getFileMode() == BtContext::MULTI) {
      ((CopyDiskAdaptor*)diskAdaptor)->setTopDir(btContext->getName());
    }
  }
  string storeDir = option->get(PREF_DIR);
  if(storeDir == "") {
    storeDir = ".";
  }
  diskAdaptor->setStoreDir(storeDir);
  diskAdaptor->setFileEntries(btContext->getFileEntries());
}

void DefaultPieceStorage::setBitfield(const unsigned char* bitfield,
				      int bitfieldLength) {
  bitfieldMan->setBitfield(bitfield, bitfieldLength);
}
  
int DefaultPieceStorage::getBitfieldLength() {
  return bitfieldMan->getBitfieldLength();
}

const unsigned char* DefaultPieceStorage::getBitfield() {
  return bitfieldMan->getBitfield();
}

DiskAdaptor* DefaultPieceStorage::getDiskAdaptor() {
  return diskAdaptor;
}

int DefaultPieceStorage::getPieceLength(int index) {
  return bitfieldMan->getBlockLength(index);
}

void DefaultPieceStorage::advertisePiece(int cuid, int index) {
  HaveEntry entry(cuid, index);
  haves.push_front(entry);
}

Integers DefaultPieceStorage::getAdvertisedPieceIndexes(int myCuid,
							const Time& lastCheckTime) {
  Integers indexes;
  for(Haves::const_iterator itr = haves.begin(); itr != haves.end(); itr++) {
    const Haves::value_type& have = *itr;
    if(have.getCuid() == myCuid) {
      continue;
    }
    if(lastCheckTime.isNewer(have.getRegisteredTime())) {
      break;
    }
    indexes.push_back(have.getIndex());
  }
  return indexes;
}

class FindElapsedHave
{
private:
  int elapsed;
public:
  FindElapsedHave(int elapsed):elapsed(elapsed) {}

  bool operator()(const HaveEntry& have) {
    if(have.getRegisteredTime().elapsed(elapsed)) {
      return true;
    } else {
      return false;
    }
  }
};
  
void DefaultPieceStorage::removeAdvertisedPiece(int elapsed) {
  Haves::iterator itr =
    find_if(haves.begin(), haves.end(), FindElapsedHave(elapsed));
  if(itr != haves.end()) {
    logger->debug("Removed %d have entries.", haves.end()-itr);
    haves.erase(itr, haves.end());
  }
}
