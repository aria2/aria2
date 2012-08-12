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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <numeric>
#include <algorithm>

#include "DownloadContext.h"
#include "Piece.h"
#include "Peer.h"
#include "LogFactory.h"
#include "Logger.h"
#include "prefs.h"
#include "DirectDiskAdaptor.h"
#include "MultiDiskAdaptor.h"
#include "DiskWriter.h"
#include "BitfieldMan.h"
#include "message.h"
#include "DefaultDiskWriterFactory.h"
#include "FileEntry.h"
#include "DlAbortEx.h"
#include "util.h"
#include "a2functional.h"
#include "Option.h"
#include "fmt.h"
#include "RarestPieceSelector.h"
#include "DefaultStreamPieceSelector.h"
#include "InorderStreamPieceSelector.h"
#include "GeomStreamPieceSelector.h"
#include "array_fun.h"
#include "PieceStatMan.h"
#include "wallclock.h"
#include "bitfield.h"
#include "SingletonHolder.h"
#include "Notifier.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

DefaultPieceStorage::DefaultPieceStorage
(const SharedHandle<DownloadContext>& downloadContext, const Option* option)
 : downloadContext_(downloadContext),
   bitfieldMan_(new BitfieldMan(downloadContext->getPieceLength(),
                                downloadContext->getTotalLength())),
   diskWriterFactory_(new DefaultDiskWriterFactory()),
   endGame_(false),
   endGamePieceNum_(END_GAME_PIECE_NUM),
   option_(option),
   pieceStatMan_(new PieceStatMan(downloadContext->getNumPieces(), true)),
   pieceSelector_(new RarestPieceSelector(pieceStatMan_))
{
  const std::string& pieceSelectorOpt =
    option_->get(PREF_STREAM_PIECE_SELECTOR);
  if(pieceSelectorOpt.empty() || pieceSelectorOpt == A2_V_DEFAULT) {
    streamPieceSelector_.reset(new DefaultStreamPieceSelector(bitfieldMan_));
  } else if(pieceSelectorOpt == V_INORDER) {
    streamPieceSelector_.reset(new InorderStreamPieceSelector(bitfieldMan_));
  } else if(pieceSelectorOpt == A2_V_GEOM) {
    streamPieceSelector_.reset(new GeomStreamPieceSelector(bitfieldMan_, 1.5));
  }
}

DefaultPieceStorage::~DefaultPieceStorage()
{
  delete bitfieldMan_;
}

SharedHandle<Piece> DefaultPieceStorage::checkOutPiece
(size_t index, cuid_t cuid)
{
  bitfieldMan_->setUseBit(index);

  SharedHandle<Piece> piece = findUsedPiece(index);
  if(!piece) {
    piece.reset(new Piece(index, bitfieldMan_->getBlockLength(index)));
#ifdef ENABLE_MESSAGE_DIGEST

    piece->setHashType(downloadContext_->getPieceHashType());

#endif // ENABLE_MESSAGE_DIGEST

    addUsedPiece(piece);
  }
  piece->addUser(cuid);
  return piece;
}

/**
 * Newly instantiated piece is not added to usedPieces.
 * Because it is waste of memory and there is no chance to use them later.
 */
SharedHandle<Piece> DefaultPieceStorage::getPiece(size_t index)
{
  SharedHandle<Piece> piece;
  if(index <= bitfieldMan_->getMaxIndex()) {
    piece = findUsedPiece(index);
    if(!piece) {
      piece.reset(new Piece(index, bitfieldMan_->getBlockLength(index)));
      if(hasPiece(index)) {
        piece->setAllBlock();
      }
    }
  }
  return piece;
}

void DefaultPieceStorage::addUsedPiece(const SharedHandle<Piece>& piece)
{
  usedPieces_.insert(piece);
  A2_LOG_DEBUG(fmt("usedPieces_.size()=%lu",
                   static_cast<unsigned long>(usedPieces_.size())));
}

SharedHandle<Piece> DefaultPieceStorage::findUsedPiece(size_t index) const
{
  SharedHandle<Piece> p(new Piece());
  p->setIndex(index);

  UsedPieceSet::iterator i = usedPieces_.find(p);
  if(i == usedPieces_.end()) {
    p.reset();
    return p;
  } else {
    return *i;
  }
}

#ifdef ENABLE_BITTORRENT

bool DefaultPieceStorage::hasMissingPiece(const SharedHandle<Peer>& peer)
{
  return bitfieldMan_->hasMissingPiece(peer->getBitfield(),
                                       peer->getBitfieldLength());
}

void DefaultPieceStorage::getMissingPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const unsigned char* bitfield,
 size_t length,
 cuid_t cuid)
{
  const size_t mislen = bitfieldMan_->getBitfieldLength();
  array_ptr<unsigned char> misbitfield(new unsigned char[mislen]);
  size_t blocks = bitfieldMan_->countBlock();
  size_t misBlock = 0;
  if(isEndGame()) {
    bool r = bitfieldMan_->getAllMissingIndexes
      (misbitfield, mislen, bitfield, length);
    if(!r) {
      return;
    }
    std::vector<size_t> indexes;
    for(size_t i = 0; i < blocks; ++i) {
      if(bitfield::test(misbitfield, blocks, i)) {
        indexes.push_back(i);
      }
    }
    std::random_shuffle(indexes.begin(), indexes.end());
    for(std::vector<size_t>::const_iterator i = indexes.begin(),
          eoi = indexes.end(); i != eoi && misBlock < minMissingBlocks; ++i) {
      SharedHandle<Piece> piece = checkOutPiece(*i, cuid);
      if(piece->getUsedBySegment()) {
        // We don't share piece downloaded via HTTP/FTP
        piece->removeUser(cuid);
      } else {
        pieces.push_back(piece);
        misBlock += piece->countMissingBlock();
      }
    }
  } else {
    bool r = bitfieldMan_->getAllMissingUnusedIndexes
      (misbitfield, mislen, bitfield, length);
    if(!r) {
      return;
    }
    while(misBlock < minMissingBlocks) {
      size_t index;
      if(pieceSelector_->select(index, misbitfield, blocks)) {
        pieces.push_back(checkOutPiece(index, cuid));
        bitfield::flipBit(misbitfield, blocks, index);
        misBlock += pieces.back()->countMissingBlock();
      } else {
        break;
      }
    }
  }
}

namespace {
void unsetExcludedIndexes(BitfieldMan& bitfield,
                          const std::vector<size_t>& excludedIndexes)
{
  std::for_each(excludedIndexes.begin(), excludedIndexes.end(),
                std::bind1st(std::mem_fun(&BitfieldMan::unsetBit), &bitfield));
}
} // namespace

void DefaultPieceStorage::createFastIndexBitfield
(BitfieldMan& bitfield, const SharedHandle<Peer>& peer)
{
  for(std::set<size_t>::const_iterator itr =
        peer->getPeerAllowedIndexSet().begin(),
        eoi = peer->getPeerAllowedIndexSet().end(); itr != eoi; ++itr) {
    if(!bitfieldMan_->isBitSet(*itr) && peer->hasPiece(*itr)) {
      bitfield.setBit(*itr);
    }
  }
}

void DefaultPieceStorage::getMissingPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  getMissingPiece(pieces, minMissingBlocks,
                  peer->getBitfield(), peer->getBitfieldLength(),
                  cuid);
}


void DefaultPieceStorage::getMissingPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  BitfieldMan tempBitfield(bitfieldMan_->getBlockLength(),
                           bitfieldMan_->getTotalLength());
  tempBitfield.setBitfield(peer->getBitfield(), peer->getBitfieldLength());
  unsetExcludedIndexes(tempBitfield, excludedIndexes);
  getMissingPiece(pieces, minMissingBlocks,
                  tempBitfield.getBitfield(), tempBitfield.getBitfieldLength(),
                  cuid);
}

void DefaultPieceStorage::getMissingFastPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  if(peer->isFastExtensionEnabled() && peer->countPeerAllowedIndexSet() > 0) {
    BitfieldMan tempBitfield(bitfieldMan_->getBlockLength(),
                             bitfieldMan_->getTotalLength());
    createFastIndexBitfield(tempBitfield, peer);
    getMissingPiece(pieces, minMissingBlocks,
                    tempBitfield.getBitfield(),
                    tempBitfield.getBitfieldLength(),
                    cuid);
  }
}

void DefaultPieceStorage::getMissingFastPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  if(peer->isFastExtensionEnabled() && peer->countPeerAllowedIndexSet() > 0) {
    BitfieldMan tempBitfield(bitfieldMan_->getBlockLength(),
                             bitfieldMan_->getTotalLength());
    createFastIndexBitfield(tempBitfield, peer);
    unsetExcludedIndexes(tempBitfield, excludedIndexes);
    getMissingPiece(pieces, minMissingBlocks,
                    tempBitfield.getBitfield(),
                    tempBitfield.getBitfieldLength(),
                    cuid);
  }
}

SharedHandle<Piece>
DefaultPieceStorage::getMissingPiece
(const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  std::vector<SharedHandle<Piece> > pieces;
  getMissingPiece(pieces, 1, peer, cuid);
  if(pieces.empty()) {
    return SharedHandle<Piece>();
  } else {
    return pieces.front();
  }
}

SharedHandle<Piece> DefaultPieceStorage::getMissingPiece
(const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  std::vector<SharedHandle<Piece> > pieces;
  getMissingPiece(pieces, 1, peer, excludedIndexes, cuid);
  if(pieces.empty()) {
    return SharedHandle<Piece>();
  } else {
    return pieces.front();
  }
}

SharedHandle<Piece> DefaultPieceStorage::getMissingFastPiece
(const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  std::vector<SharedHandle<Piece> > pieces;
  getMissingFastPiece(pieces, 1, peer, cuid);
  if(pieces.empty()) {
    return SharedHandle<Piece>();
  } else {
    return pieces.front();
  }
}

SharedHandle<Piece> DefaultPieceStorage::getMissingFastPiece
(const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  std::vector<SharedHandle<Piece> > pieces;
  getMissingFastPiece(pieces, 1, peer, excludedIndexes, cuid);
  if(pieces.empty()) {
    return SharedHandle<Piece>();
  } else {
    return pieces.front();
  }
}

#endif // ENABLE_BITTORRENT

bool DefaultPieceStorage::hasMissingUnusedPiece()
{
  size_t index;
  return bitfieldMan_->getFirstMissingUnusedIndex(index);
}

SharedHandle<Piece> DefaultPieceStorage::getMissingPiece
(size_t minSplitSize,
 const unsigned char* ignoreBitfield,
 size_t length,
 cuid_t cuid)
{
  size_t index;
  if(streamPieceSelector_->select
     (index, minSplitSize, ignoreBitfield, length)) {
    return checkOutPiece(index, cuid);
  } else {
    return SharedHandle<Piece>();
  }
}

SharedHandle<Piece> DefaultPieceStorage::getMissingPiece
(size_t index,
 cuid_t cuid)
{
  if(hasPiece(index) || isPieceUsed(index)) {
    return SharedHandle<Piece>();
  } else {
    return checkOutPiece(index, cuid);
  }
}

void DefaultPieceStorage::deleteUsedPiece(const SharedHandle<Piece>& piece)
{
  if(!piece) {
    return;
  }
  usedPieces_.erase(piece);
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
//                                                     size_t delNum)
// {
//   size_t deleted = 0;
//   for(Pieces::iterator itr = usedPieces.begin();
//       itr != usedPieces.end() && deleted < delNum;) {
//     SharedHandle<Piece>& piece = *itr;
//     if(!bitfieldMan->isUseBitSet(piece->getIndex()) &&
//        piece->countCompleteBlock() <= piece->countBlock()*(fillRate/100.0)) {
//       logger->info(MSG_DELETING_USED_PIECE,
//                  piece->getIndex(),
//                  (piece->countCompleteBlock()*100)/piece->countBlock(),
//                  fillRate);
//       itr = usedPieces.erase(itr);
//       ++deleted;
//     } else {
//       ++itr;
//     }
//   }
//   return deleted;
// }

void DefaultPieceStorage::completePiece(const SharedHandle<Piece>& piece)
{
  if(!piece) {
    return;
  }
  deleteUsedPiece(piece);
  //   if(!isEndGame()) {
  //     reduceUsedPieces(100);
  //   }
  if(allDownloadFinished()) {
    return;
  }
  bitfieldMan_->setBit(piece->getIndex());
  bitfieldMan_->unsetUseBit(piece->getIndex());
  addPieceStats(piece->getIndex());
  if(downloadFinished()) {
    downloadContext_->resetDownloadStopTime();
    if(isSelectiveDownloadingMode()) {
      A2_LOG_NOTICE(MSG_SELECTIVE_DOWNLOAD_COMPLETED);
      // following line was commented out in order to stop sending request
      // message after user-specified files were downloaded.
      //finishSelectiveDownloadingMode();
    } else {
      A2_LOG_INFO(MSG_DOWNLOAD_COMPLETED);
    }
#ifdef ENABLE_BITTORRENT
    if(downloadContext_->hasAttribute(bittorrent::BITTORRENT)) {
      SharedHandle<TorrentAttribute> torrentAttrs =
        bittorrent::getTorrentAttrs(downloadContext_);
      if(!torrentAttrs->metadata.empty()) {
        util::executeHookByOptName(downloadContext_->getOwnerRequestGroup(),
                                   option_, PREF_ON_BT_DOWNLOAD_COMPLETE);
        SingletonHolder<Notifier>::instance()->
          notifyDownloadEvent(Notifier::ON_BT_DOWNLOAD_COMPLETE,
                              downloadContext_->getOwnerRequestGroup());
      }
    }
#endif // ENABLE_BITTORRENT
  }
}

bool DefaultPieceStorage::isSelectiveDownloadingMode()
{
  return bitfieldMan_->isFilterEnabled();
}

// not unittested
void DefaultPieceStorage::cancelPiece
(const SharedHandle<Piece>& piece, cuid_t cuid)
{
  if(!piece) {
    return;
  }
  piece->removeUser(cuid);
  if(!piece->getUsed()) {
    bitfieldMan_->unsetUseBit(piece->getIndex());
  }
  if(!isEndGame()) {
    if(piece->getCompletedLength() == 0) {
      deleteUsedPiece(piece);
    }
  }
}

bool DefaultPieceStorage::hasPiece(size_t index)
{
  return bitfieldMan_->isBitSet(index);
}

bool DefaultPieceStorage::isPieceUsed(size_t index)
{
  return bitfieldMan_->isUseBitSet(index);
}

int64_t DefaultPieceStorage::getTotalLength()
{
  return bitfieldMan_->getTotalLength();
}

int64_t DefaultPieceStorage::getFilteredTotalLength()
{
  return bitfieldMan_->getFilteredTotalLength();
}

int64_t DefaultPieceStorage::getCompletedLength()
{
  int64_t completedLength =
    bitfieldMan_->getCompletedLength()+getInFlightPieceCompletedLength();
  int64_t totalLength = getTotalLength();
  if(completedLength > totalLength) {
    completedLength = totalLength;
  }
  return completedLength;
}

int64_t DefaultPieceStorage::getFilteredCompletedLength()
{
  return bitfieldMan_->getFilteredCompletedLength()+
    getInFlightPieceFilteredCompletedLength();
}

int64_t DefaultPieceStorage::getInFlightPieceCompletedLength() const
{
  int64_t len = 0;
  for(UsedPieceSet::const_iterator i = usedPieces_.begin(),
        eoi = usedPieces_.end(); i != eoi; ++i) {
    len += (*i)->getCompletedLength();
  }
  return len;
}

int64_t DefaultPieceStorage::getInFlightPieceFilteredCompletedLength() const
{
  int64_t len = 0;
  for(UsedPieceSet::const_iterator i = usedPieces_.begin(),
        eoi = usedPieces_.end(); i != eoi; ++i) {
    if(bitfieldMan_->isFilterBitSet((*i)->getIndex())) {
      len += (*i)->getCompletedLength();
    }
  }
  return len;
}

// not unittested
void DefaultPieceStorage::setupFileFilter()
{
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    downloadContext_->getFileEntries();
  bool allSelected = true;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        fileEntries.begin(), eoi = fileEntries.end();
      i != eoi; ++i) {
    if(!(*i)->isRequested()) {
      allSelected = false;
      break;
    }
  }
  if(allSelected) {
    return;
  }
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        fileEntries.begin(), eoi = fileEntries.end(); i != eoi; ++i) {
    if((*i)->isRequested()) {
      bitfieldMan_->addFilter((*i)->getOffset(), (*i)->getLength());
    }
  }
  bitfieldMan_->enableFilter();
}

// not unittested
void DefaultPieceStorage::clearFileFilter()
{
  bitfieldMan_->clearFilter();
}

// not unittested
bool DefaultPieceStorage::downloadFinished()
{
  // TODO iterate all requested FileEntry and Call
  // bitfieldMan->isBitSetOffsetRange()
  return bitfieldMan_->isFilteredAllBitSet();
}

// not unittested
bool DefaultPieceStorage::allDownloadFinished()
{
  return bitfieldMan_->isAllBitSet();
}

// not unittested
void DefaultPieceStorage::initStorage()
{
  if(downloadContext_->getFileEntries().size() == 1) {
    A2_LOG_DEBUG("Instantiating DirectDiskAdaptor");
    DirectDiskAdaptorHandle directDiskAdaptor(new DirectDiskAdaptor());
    directDiskAdaptor->setTotalLength(downloadContext_->getTotalLength());
    directDiskAdaptor->setFileEntries
      (downloadContext_->getFileEntries().begin(),
       downloadContext_->getFileEntries().end());

    DiskWriterHandle writer =
      diskWriterFactory_->newDiskWriter(directDiskAdaptor->getFilePath());
    directDiskAdaptor->setDiskWriter(writer);
    diskAdaptor_ = directDiskAdaptor;
  } else {
    A2_LOG_DEBUG("Instantiating MultiDiskAdaptor");
    MultiDiskAdaptorHandle multiDiskAdaptor(new MultiDiskAdaptor());
    multiDiskAdaptor->setFileEntries(downloadContext_->getFileEntries().begin(),
                                     downloadContext_->getFileEntries().end());
    multiDiskAdaptor->setPieceLength(downloadContext_->getPieceLength());
    multiDiskAdaptor->setMaxOpenFiles
      (option_->getAsInt(PREF_BT_MAX_OPEN_FILES));
    diskAdaptor_ = multiDiskAdaptor;
  }
  if(option_->get(PREF_FILE_ALLOCATION) == V_FALLOC) {
    diskAdaptor_->setFileAllocationMethod(DiskAdaptor::FILE_ALLOC_FALLOC);
  } else if(option_->get(PREF_FILE_ALLOCATION) == V_TRUNC) {
    diskAdaptor_->setFileAllocationMethod(DiskAdaptor::FILE_ALLOC_TRUNC);
  }
}

void DefaultPieceStorage::setBitfield(const unsigned char* bitfield,
                                      size_t bitfieldLength)
{
  bitfieldMan_->setBitfield(bitfield, bitfieldLength);
  addPieceStats(bitfield, bitfieldLength);
}

size_t DefaultPieceStorage::getBitfieldLength()
{
  return bitfieldMan_->getBitfieldLength();
}

const unsigned char* DefaultPieceStorage::getBitfield()
{
  return bitfieldMan_->getBitfield();
}

DiskAdaptorHandle DefaultPieceStorage::getDiskAdaptor() {
  return diskAdaptor_;
}

int32_t DefaultPieceStorage::getPieceLength(size_t index)
{
  return bitfieldMan_->getBlockLength(index);
}

void DefaultPieceStorage::advertisePiece(cuid_t cuid, size_t index)
{
  HaveEntry entry(cuid, index, global::wallclock());
  haves_.push_front(entry);
}

void
DefaultPieceStorage::getAdvertisedPieceIndexes(std::vector<size_t>& indexes,
                                               cuid_t myCuid,
                                               const Timer& lastCheckTime)
{
  for(std::deque<HaveEntry>::const_iterator itr = haves_.begin(),
        eoi = haves_.end(); itr != eoi; ++itr) {
    const HaveEntry& have = *itr;
    if(have.getCuid() == myCuid) {
      continue;
    }
    if(lastCheckTime > have.getRegisteredTime()) {
      break;
    }
    indexes.push_back(have.getIndex());
  }
}

namespace {
class FindElapsedHave
{
private:
  time_t elapsed;
public:
  FindElapsedHave(time_t elapsed):elapsed(elapsed) {}

  bool operator()(const HaveEntry& have) {
    if(have.getRegisteredTime().difference(global::wallclock()) >= elapsed) {
      return true;
    } else {
      return false;
    }
  }
};
} // namespace

void DefaultPieceStorage::removeAdvertisedPiece(time_t elapsed)
{
  std::deque<HaveEntry>::iterator itr =
    std::find_if(haves_.begin(), haves_.end(), FindElapsedHave(elapsed));
  if(itr != haves_.end()) {
    A2_LOG_DEBUG(fmt(MSG_REMOVED_HAVE_ENTRY,
                     static_cast<unsigned long>(haves_.end()-itr)));
    haves_.erase(itr, haves_.end());
  }
}

void DefaultPieceStorage::markAllPiecesDone()
{
  bitfieldMan_->setAllBit();
}

void DefaultPieceStorage::markPiecesDone(int64_t length)
{
  if(length == bitfieldMan_->getTotalLength()) {
    bitfieldMan_->setAllBit();
  } else if(length == 0) {
    // TODO this would go to markAllPiecesUndone()
    bitfieldMan_->clearAllBit();
    usedPieces_.clear();
  } else {
    size_t numPiece = length/bitfieldMan_->getBlockLength();
    if(numPiece > 0) {
      bitfieldMan_->setBitRange(0, numPiece-1);
    }
    size_t r = (length%bitfieldMan_->getBlockLength())/Piece::BLOCK_LENGTH;
    if(r > 0) {
      SharedHandle<Piece> p
        (new Piece(numPiece, bitfieldMan_->getBlockLength(numPiece)));
      
      for(size_t i = 0; i < r; ++i) {
        p->completeBlock(i);
      }

#ifdef ENABLE_MESSAGE_DIGEST

      p->setHashType(downloadContext_->getPieceHashType());

#endif // ENABLE_MESSAGE_DIGEST

      addUsedPiece(p);
    }
  }
}

void DefaultPieceStorage::markPieceMissing(size_t index)
{
  bitfieldMan_->unsetBit(index);
}

void DefaultPieceStorage::addInFlightPiece
(const std::vector<SharedHandle<Piece> >& pieces)
{
  usedPieces_.insert(pieces.begin(), pieces.end());
}

size_t DefaultPieceStorage::countInFlightPiece()
{
  return usedPieces_.size();
}

void DefaultPieceStorage::getInFlightPieces
(std::vector<SharedHandle<Piece> >& pieces)
{
  pieces.insert(pieces.end(), usedPieces_.begin(), usedPieces_.end());
}

void DefaultPieceStorage::setDiskWriterFactory
(const DiskWriterFactoryHandle& diskWriterFactory)
{
  diskWriterFactory_ = diskWriterFactory;
}

void DefaultPieceStorage::addPieceStats(const unsigned char* bitfield,
                                        size_t bitfieldLength)
{
  pieceStatMan_->addPieceStats(bitfield, bitfieldLength);
}

void DefaultPieceStorage::subtractPieceStats(const unsigned char* bitfield,
                                             size_t bitfieldLength)
{
  pieceStatMan_->subtractPieceStats(bitfield, bitfieldLength);
}

void DefaultPieceStorage::updatePieceStats(const unsigned char* newBitfield,
                                           size_t newBitfieldLength,
                                           const unsigned char* oldBitfield)
{
  pieceStatMan_->updatePieceStats(newBitfield, newBitfieldLength,
                                  oldBitfield);
}

void DefaultPieceStorage::addPieceStats(size_t index)
{
  pieceStatMan_->addPieceStats(index);
}

size_t DefaultPieceStorage::getNextUsedIndex(size_t index)
{
  for(size_t i = index+1; i < bitfieldMan_->countBlock(); ++i) {
    if(bitfieldMan_->isUseBitSet(i) || bitfieldMan_->isBitSet(i)) {
      return i;
    }
  }
  return bitfieldMan_->countBlock();
}

void DefaultPieceStorage::onDownloadIncomplete()
{
  streamPieceSelector_->onBitfieldInit();
}

} // namespace aria2
