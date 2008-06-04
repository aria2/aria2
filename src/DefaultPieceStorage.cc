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
#include "DownloadContext.h"
#include "Piece.h"
#include "Peer.h"
#include "LogFactory.h"
#include "Logger.h"
#include "prefs.h"
#include "DirectDiskAdaptor.h"
#include "MultiDiskAdaptor.h"
#include "CopyDiskAdaptor.h"
#include "DiskWriter.h"
#include "BitfieldManFactory.h"
#include "BitfieldMan.h"
#include "message.h"
#include "DefaultDiskWriterFactory.h"
#include "FileEntry.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "a2functional.h"
#include "Option.h"
#include "StringFormat.h"
#include "RarestPieceSelector.h"
#include <numeric>
#include <algorithm>

namespace aria2 {

DefaultPieceStorage::DefaultPieceStorage(const DownloadContextHandle& downloadContext, const Option* option, bool randomPieceStatsOrdering):
  downloadContext(downloadContext),
  bitfieldMan(BitfieldManFactory::getFactoryInstance()->
	      createBitfieldMan(downloadContext->getPieceLength(),
				downloadContext->getTotalLength())),
  _diskWriterFactory(new DefaultDiskWriterFactory()),
  endGamePieceNum(END_GAME_PIECE_NUM),
  logger(LogFactory::getInstance()),
  option(option),
  _pieceSelector(new RarestPieceSelector(downloadContext->getNumPieces(),
					 randomPieceStatsOrdering))
{}

DefaultPieceStorage::~DefaultPieceStorage() {
  delete bitfieldMan;
}

bool DefaultPieceStorage::hasMissingPiece(const PeerHandle& peer)
{
  return bitfieldMan->hasMissingPiece(peer->getBitfield(),
				      peer->getBitfieldLength());
}

bool DefaultPieceStorage::isEndGame()
{
  return bitfieldMan->countMissingBlock() <= endGamePieceNum;
}

bool DefaultPieceStorage::getMissingPieceIndex(size_t& index, const PeerHandle& peer)
{
  std::deque<size_t> indexes;
  bool r;
  if(isEndGame()) {
    r = bitfieldMan->getAllMissingIndexes(indexes, peer->getBitfield(),
					  peer->getBitfieldLength());
  } else {
    r = bitfieldMan->getAllMissingUnusedIndexes(indexes,
						peer->getBitfield(),
						peer->getBitfieldLength());
  }
  if(r) {
    // We assume indexes is sorted using comparator less.
    _pieceSelector->select(index, indexes);
    return true;
  } else {
    return false;
  }
}

PieceHandle DefaultPieceStorage::checkOutPiece(size_t index)
{
  bitfieldMan->setUseBit(index);

  PieceHandle piece = findUsedPiece(index);
  if(piece.isNull()) {
    piece.reset(new Piece(index, bitfieldMan->getBlockLength(index)));

#ifdef ENABLE_MESSAGE_DIGEST

    piece->setHashAlgo(downloadContext->getPieceHashAlgo());

#endif // ENABLE_MESSAGE_DIGEST

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
PieceHandle DefaultPieceStorage::getPiece(size_t index)
{
  PieceHandle piece;
  if(0 <= index && index <= bitfieldMan->getMaxIndex()) {
    piece = findUsedPiece(index);
    if(piece.isNull()) {
      piece.reset(new Piece(index, bitfieldMan->getBlockLength(index)));
      if(hasPiece(index)) {
	piece->setAllBlock();
      }
    }
  }
  return piece;
}

void DefaultPieceStorage::addUsedPiece(const PieceHandle& piece)
{
  std::deque<SharedHandle<Piece> >::iterator i =
    std::lower_bound(usedPieces.begin(), usedPieces.end(), piece);
  usedPieces.insert(i, piece);
  logger->debug("usedPieces.size()=%zu", usedPieces.size());
}

PieceHandle DefaultPieceStorage::findUsedPiece(size_t index) const
{
  SharedHandle<Piece> p(new Piece());
  p->setIndex(index);

  std::deque<SharedHandle<Piece> >::const_iterator i =
    std::lower_bound(usedPieces.begin(), usedPieces.end(), p);
  if(i != usedPieces.end() && (*i) == p) {
    return *i;
  } else {
    p.reset(0);
    return p;
  }
}

PieceHandle DefaultPieceStorage::getMissingPiece(const PeerHandle& peer)
{
  size_t index;
  if(getMissingPieceIndex(index, peer)) {
    return checkOutPiece(index);
  } else {
    return SharedHandle<Piece>();
  }
}

bool DefaultPieceStorage::getMissingFastPieceIndex(size_t& index,
						   const PeerHandle& peer)
{
  if(peer->isFastExtensionEnabled() && peer->countPeerAllowedIndexSet() > 0) {
    BitfieldMan tempBitfield(bitfieldMan->getBlockLength(),
			     bitfieldMan->getTotalLength());
    for(std::deque<size_t>::const_iterator itr = peer->getPeerAllowedIndexSet().begin();
	itr != peer->getPeerAllowedIndexSet().end(); itr++) {
      if(!bitfieldMan->isBitSet(index) && peer->hasPiece(*itr)) {
	tempBitfield.setBit(*itr);
      }
    }
    if(isEndGame()) {
      return bitfieldMan->getMissingIndex(index, tempBitfield.getBitfield(),
					  tempBitfield.getBitfieldLength());
    } else {
      return bitfieldMan->getMissingUnusedIndex(index,
						tempBitfield.getBitfield(),
						tempBitfield.getBitfieldLength());
    }
  }
  return false;
}

PieceHandle DefaultPieceStorage::getMissingFastPiece(const PeerHandle& peer)
{
  size_t index;
  if(getMissingFastPieceIndex(index, peer)) {
    return checkOutPiece(index);
  } else {
    return SharedHandle<Piece>();
  }
}

PieceHandle DefaultPieceStorage::getMissingPiece()
{
  size_t index;
  if(bitfieldMan->getSparseMissingUnusedIndex(index)) {
    return checkOutPiece(index);
  } else {
    return SharedHandle<Piece>();
  }
}

PieceHandle DefaultPieceStorage::getMissingPiece(size_t index)
{
  if(hasPiece(index) || isPieceUsed(index)) {
    return SharedHandle<Piece>();
  } else {
    return checkOutPiece(index);
  }
}

void DefaultPieceStorage::deleteUsedPiece(const PieceHandle& piece)
{
  if(piece.isNull()) {
    return;
  }
  std::deque<SharedHandle<Piece> >::iterator i = 
    std::lower_bound(usedPieces.begin(), usedPieces.end(), piece);
  if(i != usedPieces.end() && (*i) == piece) {
    usedPieces.erase(i);
  }
}

// void DefaultPieceStorage::reduceUsedPieces(size_t upperBound)
// {
//   size_t usedPiecesSize = usedPieces.size();
//   if(usedPiecesSize <= upperBound) {
//     return;
//   }
//   size_t delNum = usedPiecesSize-upperBound;
//   int fillRate = 10;
//   while(delNum && fillRate <= 15) {
//     delNum -= deleteUsedPiecesByFillRate(fillRate, delNum);
//     fillRate += 5;
//   }
// }

// size_t DefaultPieceStorage::deleteUsedPiecesByFillRate(int fillRate,
// 						       size_t delNum)
// {
//   size_t deleted = 0;
//   for(Pieces::iterator itr = usedPieces.begin();
//       itr != usedPieces.end() && deleted < delNum;) {
//     PieceHandle& piece = *itr;
//     if(!bitfieldMan->isUseBitSet(piece->getIndex()) &&
//        piece->countCompleteBlock() <= piece->countBlock()*(fillRate/100.0)) {
//       logger->info(MSG_DELETING_USED_PIECE,
// 		    piece->getIndex(),
// 		    (piece->countCompleteBlock()*100)/piece->countBlock(),
// 		    fillRate);
//       itr = usedPieces.erase(itr);
//       ++deleted;
//     } else {
//       ++itr;
//     }
//   }
//   return deleted;
// }

void DefaultPieceStorage::completePiece(const PieceHandle& piece)
{
  if(piece.isNull()) {
    return;
  }
  deleteUsedPiece(piece);
//   if(!isEndGame()) {
//     reduceUsedPieces(100);
//   }
  if(allDownloadFinished()) {
    return;
  }
  bitfieldMan->setBit(piece->getIndex());
  bitfieldMan->unsetUseBit(piece->getIndex());
  addPieceStats(piece->getIndex());
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

bool DefaultPieceStorage::isSelectiveDownloadingMode()
{
  return bitfieldMan->isFilterEnabled();
}

void DefaultPieceStorage::finishSelectiveDownloadingMode()
{
  bitfieldMan->clearFilter();
  diskAdaptor->addAllDownloadEntry();
}

// not unittested
void DefaultPieceStorage::cancelPiece(const PieceHandle& piece)
{
  if(piece.isNull()) {
    return;
  }
  bitfieldMan->unsetUseBit(piece->getIndex());
  if(!isEndGame()) {
    if(piece->getCompletedLength() == 0) {
      deleteUsedPiece(piece);
    }
  }
}

bool DefaultPieceStorage::hasPiece(size_t index)
{
  return bitfieldMan->isBitSet(index);
}

bool DefaultPieceStorage::isPieceUsed(size_t index)
{
  return bitfieldMan->isUseBitSet(index);
}

uint64_t DefaultPieceStorage::getTotalLength()
{
  return bitfieldMan->getTotalLength();
}

uint64_t DefaultPieceStorage::getFilteredTotalLength()
{
  return bitfieldMan->getFilteredTotalLength();
}

uint64_t DefaultPieceStorage::getCompletedLength()
{
  return bitfieldMan->getCompletedLength()+getInFlightPieceCompletedLength();
}

uint64_t DefaultPieceStorage::getFilteredCompletedLength()
{
  return bitfieldMan->getFilteredCompletedLength()+getInFlightPieceCompletedLength();
}

size_t DefaultPieceStorage::getInFlightPieceCompletedLength() const
{
  return std::accumulate(usedPieces.begin(), usedPieces.end(), 0, adopt2nd(std::plus<size_t>(), mem_fun_sh(&Piece::getCompletedLength)));
}

// not unittested
void DefaultPieceStorage::setFileFilter(const std::deque<std::string>& filePaths)
{
  if(downloadContext->getFileMode() != DownloadContext::MULTI || filePaths.empty()) {
    return;
  }
  diskAdaptor->removeAllDownloadEntry();
  for(std::deque<std::string>::const_iterator pitr = filePaths.begin();
      pitr != filePaths.end(); pitr++) {
    if(!diskAdaptor->addDownloadEntry(*pitr)) {
      throw DlAbortEx(StringFormat(EX_NO_SUCH_FILE_ENTRY, (*pitr).c_str()).str());
    }
    FileEntryHandle fileEntry = diskAdaptor->getFileEntryFromPath(*pitr);
    bitfieldMan->addFilter(fileEntry->getOffset(), fileEntry->getLength());
  }
  bitfieldMan->enableFilter();
}

void DefaultPieceStorage::setFileFilter(IntSequence seq)
{
  std::deque<int32_t> fileIndexes = seq.flush();
  // TODO Is sorting necessary?
  std::sort(fileIndexes.begin(), fileIndexes.end());
  fileIndexes.erase(std::unique(fileIndexes.begin(), fileIndexes.end()), fileIndexes.end());
  std::deque<std::string> filePaths;
  const FileEntries& entries = diskAdaptor->getFileEntries();
  for(size_t i = 0; i < entries.size(); i++) {
    if(std::find(fileIndexes.begin(), fileIndexes.end(), i+1) != fileIndexes.end()) {
      logger->debug("index=%d is %s", i+1, entries[i]->getPath().c_str());
      filePaths.push_back(entries[i]->getPath());
    }
  }
  setFileFilter(filePaths);
}

// not unittested
void DefaultPieceStorage::clearFileFilter()
{
  bitfieldMan->clearFilter();
  diskAdaptor->addAllDownloadEntry();
}

// not unittested
bool DefaultPieceStorage::downloadFinished()
{
  // TODO iterate all requested FileEntry and Call bitfieldMan->isBitSetOffsetRange()
  return bitfieldMan->isFilteredAllBitSet();
}

// not unittested
bool DefaultPieceStorage::allDownloadFinished()
{
  return bitfieldMan->isAllBitSet();
}

// not unittested
void DefaultPieceStorage::initStorage()
{
  if(downloadContext->getFileMode() == DownloadContext::SINGLE) {
    logger->debug("Instantiating DirectDiskAdaptor");
    DiskWriterHandle writer = _diskWriterFactory->newDiskWriter();
    writer->setDirectIOAllowed(option->getAsBool(PREF_ENABLE_DIRECT_IO));
    DirectDiskAdaptorHandle directDiskAdaptor(new DirectDiskAdaptor());
    directDiskAdaptor->setDiskWriter(writer);
    directDiskAdaptor->setTotalLength(downloadContext->getTotalLength());
    this->diskAdaptor = directDiskAdaptor;
  } else {
    // file mode == DownloadContext::MULTI
    if(option->getAsBool(PREF_DIRECT_FILE_MAPPING)) {
      logger->debug("Instantiating MultiDiskAdaptor");
      MultiDiskAdaptorHandle multiDiskAdaptor(new MultiDiskAdaptor());
      multiDiskAdaptor->setDirectIOAllowed(option->getAsBool(PREF_ENABLE_DIRECT_IO));
      multiDiskAdaptor->setPieceLength(downloadContext->getPieceLength());
      multiDiskAdaptor->setTopDir(downloadContext->getName());
      this->diskAdaptor = multiDiskAdaptor;
    } else {
      logger->debug("Instantiating CopyDiskAdaptor");
      DiskWriterHandle writer = _diskWriterFactory->newDiskWriter();
      writer->setDirectIOAllowed(option->getAsBool(PREF_ENABLE_DIRECT_IO));
      CopyDiskAdaptorHandle copyDiskAdaptor(new CopyDiskAdaptor());
      copyDiskAdaptor->setDiskWriter(writer);
      copyDiskAdaptor->setTempFilename(downloadContext->getName()+".a2tmp");
      copyDiskAdaptor->setTotalLength(downloadContext->getTotalLength());
      if(downloadContext->getFileMode() == DownloadContext::MULTI) {
	copyDiskAdaptor->setTopDir(downloadContext->getName());
      }
      this->diskAdaptor = copyDiskAdaptor;
    }
  }
  diskAdaptor->setStoreDir(downloadContext->getDir());
  diskAdaptor->setFileEntries(downloadContext->getFileEntries());
}

void DefaultPieceStorage::setBitfield(const unsigned char* bitfield,
				      size_t bitfieldLength)
{
  bitfieldMan->setBitfield(bitfield, bitfieldLength);
  addPieceStats(bitfield, bitfieldLength);
}

size_t DefaultPieceStorage::getBitfieldLength()
{
  return bitfieldMan->getBitfieldLength();
}

const unsigned char* DefaultPieceStorage::getBitfield()
{
  return bitfieldMan->getBitfield();
}

DiskAdaptorHandle DefaultPieceStorage::getDiskAdaptor() {
  return diskAdaptor;
}

size_t DefaultPieceStorage::getPieceLength(size_t index)
{
  return bitfieldMan->getBlockLength(index);
}

void DefaultPieceStorage::advertisePiece(int32_t cuid, size_t index)
{
  HaveEntry entry(cuid, index);
  haves.push_front(entry);
}

void
DefaultPieceStorage::getAdvertisedPieceIndexes(std::deque<size_t>& indexes,
					       int32_t myCuid,
					       const Time& lastCheckTime)
{
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
}

class FindElapsedHave
{
private:
  time_t elapsed;
public:
  FindElapsedHave(time_t elapsed):elapsed(elapsed) {}

  bool operator()(const HaveEntry& have) {
    if(have.getRegisteredTime().elapsed(elapsed)) {
      return true;
    } else {
      return false;
    }
  }
};
  
void DefaultPieceStorage::removeAdvertisedPiece(time_t elapsed)
{
  Haves::iterator itr =
    std::find_if(haves.begin(), haves.end(), FindElapsedHave(elapsed));
  if(itr != haves.end()) {
    logger->debug(MSG_REMOVED_HAVE_ENTRY, haves.end()-itr);
    haves.erase(itr, haves.end());
  }
}

void DefaultPieceStorage::markAllPiecesDone()
{
  bitfieldMan->setAllBit();
}

void DefaultPieceStorage::markPiecesDone(uint64_t length)
{
  if(length == bitfieldMan->getTotalLength()) {
    bitfieldMan->setAllBit();
  } else {
    size_t numPiece = length/bitfieldMan->getBlockLength();
    if(numPiece > 0) {
      bitfieldMan->setBitRange(0, numPiece-1);
    }
    size_t r = (length%bitfieldMan->getBlockLength())/Piece::BLOCK_LENGTH;
    if(r > 0) {
      PieceHandle p(new Piece(numPiece, bitfieldMan->getBlockLength(numPiece)));
      
      for(size_t i = 0; i < r; ++i) {
	p->completeBlock(i);
      }

#ifdef ENABLE_MESSAGE_DIGEST

      p->setHashAlgo(downloadContext->getPieceHashAlgo());

#endif // ENABLE_MESSAGE_DIGEST

      addUsedPiece(p);
    }
  }
}

void DefaultPieceStorage::markPieceMissing(size_t index)
{
  bitfieldMan->unsetBit(index);
}

void DefaultPieceStorage::addInFlightPiece(const Pieces& pieces)
{
  usedPieces.insert(usedPieces.end(), pieces.begin(), pieces.end());
  std::sort(usedPieces.begin(), usedPieces.end());
}

size_t DefaultPieceStorage::countInFlightPiece()
{
  return usedPieces.size();
}

void DefaultPieceStorage::getInFlightPieces(std::deque<SharedHandle<Piece> >& pieces)
{
  pieces.insert(pieces.end(), usedPieces.begin(), usedPieces.end());
}

void DefaultPieceStorage::setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory)
{
  _diskWriterFactory = diskWriterFactory;
}

void DefaultPieceStorage::addPieceStats(const unsigned char* bitfield,
					size_t bitfieldLength)
{
  _pieceSelector->addPieceStats(bitfield, bitfieldLength);
}

void DefaultPieceStorage::subtractPieceStats(const unsigned char* bitfield,
					     size_t bitfieldLength)
{
  _pieceSelector->subtractPieceStats(bitfield, bitfieldLength);
}

void DefaultPieceStorage::updatePieceStats(const unsigned char* newBitfield,
					   size_t newBitfieldLength,
					   const unsigned char* oldBitfield)
{
  _pieceSelector->updatePieceStats(newBitfield, newBitfieldLength,
				   oldBitfield);
}

void DefaultPieceStorage::addPieceStats(size_t index)
{
  _pieceSelector->addPieceStats(index);
}

} // namespace aria2
