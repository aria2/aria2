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
#include "DlAbortEx.h"
#include "BitfieldManFactory.h"
#include "FileAllocationMonitor.h"
#include "DiskAdaptorWriter.h"
#include "ChunkChecksumValidator.h"
#include "message.h"

DefaultPieceStorage::DefaultPieceStorage(BtContextHandle btContext, const Option* option):
  btContext(btContext),
  diskAdaptor(0),
  endGamePieceNum(END_GAME_PIECE_NUM),
  option(option)
{
  bitfieldMan =
    BitfieldManFactory::getFactoryInstance()->
    createBitfieldMan(btContext->getPieceLength(),
		      btContext->getTotalLength());
  logger = LogFactory::getInstance();
}

DefaultPieceStorage::~DefaultPieceStorage() {
  delete bitfieldMan;
}

bool DefaultPieceStorage::hasMissingPiece(const PeerHandle& peer) {
  return bitfieldMan->hasMissingPiece(peer->getBitfield(),
				      peer->getBitfieldLength());
}

bool DefaultPieceStorage::isEndGame() {
  return bitfieldMan->countMissingBlock() <= endGamePieceNum;
}

int32_t DefaultPieceStorage::getMissingPieceIndex(const PeerHandle& peer) {
  int32_t index = -1;
  if(isEndGame()) {
    index = bitfieldMan->getMissingIndex(peer->getBitfield(),
					 peer->getBitfieldLength());
  } else {
    index = bitfieldMan->getMissingUnusedIndex(peer->getBitfield(),
					       peer->getBitfieldLength());
  }
  return index;
}

PieceHandle DefaultPieceStorage::checkOutPiece(int32_t index) {
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
PieceHandle DefaultPieceStorage::getPiece(int32_t index) {
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
  int32_t index;
public:
  FindPiece(int32_t index):index(index) {}

  bool operator()(const PieceHandle& piece) {
    return piece->getIndex() == index;
  }
};

PieceHandle DefaultPieceStorage::findUsedPiece(int32_t index) const {
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
  int32_t index = getMissingPieceIndex(peer);
  return checkOutPiece(index);
}

int32_t DefaultPieceStorage::getMissingFastPieceIndex(const PeerHandle& peer) {
  int32_t index = -1;
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
  int32_t index = getMissingFastPieceIndex(peer);
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

void DefaultPieceStorage::reduceUsedPieces(int32_t delMax) {
  int32_t toDelete = usedPieces.size()-delMax;
  if(toDelete <= 0) {
    return;
  }
  int32_t fillRate = 10;
  while(fillRate < 50) {
    int32_t deleted = deleteUsedPiecesByFillRate(fillRate, toDelete);
    if(deleted == 0) {
      break;
    }
    toDelete -= deleted;
    fillRate += 10;
  }
}

int32_t DefaultPieceStorage::deleteUsedPiecesByFillRate(int32_t fillRate,
							   int32_t toDelete) {
  int32_t deleted = 0;
  for(Pieces::iterator itr = usedPieces.begin();
      itr != usedPieces.end() && deleted < toDelete;) {
    PieceHandle& piece = *itr;
    if(!bitfieldMan->isUseBitSet(piece->getIndex()) &&
       piece->countCompleteBlock() <= piece->countBlock()*(fillRate/100.0)) {
      logger->debug(MSG_DELETING_USED_PIECE,
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
  if(allDownloadFinished()) {
    return;
  }
  bitfieldMan->setBit(piece->getIndex());
  bitfieldMan->unsetUseBit(piece->getIndex());
  if(downloadFinished()) {
    diskAdaptor->onDownloadComplete();
    if(isSelectiveDownloadingMode()) {
      logger->notice(MSG_SELECTIVE_DOWNLOAD_COMPLETED);
      // following line was commented out in order to stop sending request
      // message after user-specified files were downloaded.
      //finishSelectiveDownloadingMode();
    } else {
      logger->info(MSG_DOWNLOAD_COMPLETED);
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

bool DefaultPieceStorage::hasPiece(int32_t index) {
  return bitfieldMan->isBitSet(index);
}

int64_t DefaultPieceStorage::getTotalLength() {
  return bitfieldMan->getTotalLength();
}

int64_t DefaultPieceStorage::getFilteredTotalLength() {
  return bitfieldMan->getFilteredTotalLength();
}

int64_t DefaultPieceStorage::getCompletedLength() {
  return bitfieldMan->getCompletedLength();
}

int64_t DefaultPieceStorage::getFilteredCompletedLength() {
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
      throw new DlAbortEx(EX_NO_SUCH_FILE_ENTRY, (*pitr).c_str());
    }
    FileEntryHandle fileEntry = diskAdaptor->getFileEntryFromPath(*pitr);
    bitfieldMan->addFilter(fileEntry->getOffset(), fileEntry->getLength());
  }
  bitfieldMan->enableFilter();
}

void DefaultPieceStorage::setFileFilter(const Integers& fileIndexes) {
  Strings filePaths;
  const FileEntries& entries = diskAdaptor->getFileEntries();
  for(int32_t i = 0; i < (int32_t)entries.size(); i++) {
    if(find(fileIndexes.begin(), fileIndexes.end(), i+1) != fileIndexes.end()) {
      logger->debug("index=%d is %s", i+1, entries[i]->getPath().c_str());
      filePaths.push_back(entries[i]->getPath());
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
  return bitfieldMan->isFilteredAllBitSet();
}

// not unittested
bool DefaultPieceStorage::allDownloadFinished() {
  return bitfieldMan->isAllBitSet();
}

// not unittested
void DefaultPieceStorage::initStorage() {
  if(option->get(PREF_DIRECT_FILE_MAPPING) == V_TRUE) {
    if(btContext->getFileMode() == BtContext::SINGLE) {
      DefaultDiskWriterHandle writer = DefaultDiskWriter::createNewDiskWriter(option);
      DirectDiskAdaptorHandle directDiskAdaptor = new DirectDiskAdaptor();
      directDiskAdaptor->setDiskWriter(writer);
      directDiskAdaptor->setTotalLength(btContext->getTotalLength());
      this->diskAdaptor = directDiskAdaptor;
    } else {
      MultiDiskAdaptorHandle multiDiskAdaptor = new MultiDiskAdaptor();
      multiDiskAdaptor->setPieceLength(btContext->getPieceLength());
      multiDiskAdaptor->setTopDir(btContext->getName());
      multiDiskAdaptor->setOption(option);
      this->diskAdaptor = multiDiskAdaptor;
    }
  } else {
    DefaultDiskWriterHandle writer = DefaultDiskWriter::createNewDiskWriter(option);
    CopyDiskAdaptorHandle copyDiskAdaptor = new CopyDiskAdaptor();
    copyDiskAdaptor->setDiskWriter(writer);
    copyDiskAdaptor->setTempFilename(btContext->getName()+".a2tmp");
    copyDiskAdaptor->setTotalLength(btContext->getTotalLength());
    if(btContext->getFileMode() == BtContext::MULTI) {
      copyDiskAdaptor->setTopDir(btContext->getName());
    }
    this->diskAdaptor = copyDiskAdaptor;
  }
  string storeDir = option->get(PREF_DIR);
  if(storeDir == "") {
    storeDir = ".";
  }
  diskAdaptor->setStoreDir(storeDir);
  diskAdaptor->setFileEntries(btContext->getFileEntries());
}

void DefaultPieceStorage::setBitfield(const unsigned char* bitfield,
				      int32_t bitfieldLength) {
  bitfieldMan->setBitfield(bitfield, bitfieldLength);
}
  
int32_t DefaultPieceStorage::getBitfieldLength() {
  return bitfieldMan->getBitfieldLength();
}

const unsigned char* DefaultPieceStorage::getBitfield() {
  return bitfieldMan->getBitfield();
}

DiskAdaptorHandle DefaultPieceStorage::getDiskAdaptor() {
  return diskAdaptor;
}

int32_t DefaultPieceStorage::getPieceLength(int32_t index) {
  return bitfieldMan->getBlockLength(index);
}

void DefaultPieceStorage::advertisePiece(int32_t cuid, int32_t index) {
  HaveEntry entry(cuid, index);
  haves.push_front(entry);
}

Integers DefaultPieceStorage::getAdvertisedPieceIndexes(int32_t myCuid,
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
  int32_t elapsed;
public:
  FindElapsedHave(int32_t elapsed):elapsed(elapsed) {}

  bool operator()(const HaveEntry& have) {
    if(have.getRegisteredTime().elapsed(elapsed)) {
      return true;
    } else {
      return false;
    }
  }
};
  
void DefaultPieceStorage::removeAdvertisedPiece(int32_t elapsed) {
  Haves::iterator itr =
    find_if(haves.begin(), haves.end(), FindElapsedHave(elapsed));
  if(itr != haves.end()) {
    logger->debug(MSG_REMOVED_HAVE_ENTRY, haves.end()-itr);
    haves.erase(itr, haves.end());
  }
}

void DefaultPieceStorage::markAllPiecesDone()
{
  bitfieldMan->setAllBit();
}

void DefaultPieceStorage::checkIntegrity()
{
  logger->notice(MSG_VALIDATING_FILE,
		 diskAdaptor->getFilePath().c_str());
  ChunkChecksumValidator v;
  v.setDigestAlgo(DIGEST_ALGO_SHA1);
  v.setDiskWriter(new DiskAdaptorWriter(diskAdaptor));
  v.setFileAllocationMonitor(FileAllocationMonitorFactory::getFactory()->createNewMonitor());
  v.validate(bitfieldMan, btContext->getPieceHashes(),
	     btContext->getPieceLength());
}
