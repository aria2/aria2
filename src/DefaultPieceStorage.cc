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
#include "RandomStreamPieceSelector.h"
#include "GeomStreamPieceSelector.h"
#include "array_fun.h"
#include "PieceStatMan.h"
#include "wallclock.h"
#include "bitfield.h"
#include "SingletonHolder.h"
#include "Notifier.h"
#include "WrDiskCache.h"
#include "RequestGroup.h"
#include "SimpleRandomizer.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

DefaultPieceStorage::DefaultPieceStorage(
    const std::shared_ptr<DownloadContext>& downloadContext,
    const Option* option)
    : downloadContext_(downloadContext),
      bitfieldMan_(make_unique<BitfieldMan>(downloadContext->getPieceLength(),
                                            downloadContext->getTotalLength())),
      diskWriterFactory_(std::make_shared<DefaultDiskWriterFactory>()),
      endGame_(false),
      endGamePieceNum_(END_GAME_PIECE_NUM),
      option_(option),
      // The DefaultBtInteractive has the default value of
      // lastHaveIndex of 0, so we need to make nextHaveIndex_ more
      // than that.
      nextHaveIndex_(1),
      pieceStatMan_(std::make_shared<PieceStatMan>(
          downloadContext->getNumPieces(), true)),
      pieceSelector_(make_unique<RarestPieceSelector>(pieceStatMan_)),
      wrDiskCache_(nullptr)
{
  const std::string& pieceSelectorOpt =
      option_->get(PREF_STREAM_PIECE_SELECTOR);
  if (pieceSelectorOpt.empty() || pieceSelectorOpt == A2_V_DEFAULT) {
    streamPieceSelector_ =
        make_unique<DefaultStreamPieceSelector>(bitfieldMan_.get());
  }
  else if (pieceSelectorOpt == V_INORDER) {
    streamPieceSelector_ =
        make_unique<InorderStreamPieceSelector>(bitfieldMan_.get());
  }
  else if (pieceSelectorOpt == A2_V_RANDOM) {
    streamPieceSelector_ =
        make_unique<RandomStreamPieceSelector>(bitfieldMan_.get());
  }
  else if (pieceSelectorOpt == A2_V_GEOM) {
    streamPieceSelector_ =
        make_unique<GeomStreamPieceSelector>(bitfieldMan_.get(), 1.5);
  }
}

DefaultPieceStorage::~DefaultPieceStorage() = default;

std::shared_ptr<Piece> DefaultPieceStorage::checkOutPiece(size_t index,
                                                          cuid_t cuid)
{
  assert(!bitfieldMan_->isFilterEnabled() ||
         bitfieldMan_->isFilterBitSet(index));

  bitfieldMan_->setUseBit(index);

  std::shared_ptr<Piece> piece = findUsedPiece(index);
  if (!piece) {
    piece = std::make_shared<Piece>(index, bitfieldMan_->getBlockLength(index));
    piece->setHashType(downloadContext_->getPieceHashType());

    addUsedPiece(piece);
  }
  piece->addUser(cuid);
  RequestGroup* group = downloadContext_->getOwnerRequestGroup();
  if ((!group || !group->inMemoryDownload()) && wrDiskCache_ &&
      !piece->getWrDiskCacheEntry()) {
    // So, we rely on the fact that diskAdaptor_ is not reinitialized
    // in the session.
    piece->initWrCache(wrDiskCache_, diskAdaptor_);
  }
  return piece;
}

/**
 * Newly instantiated piece is not added to usedPieces.
 * Because it is waste of memory and there is no chance to use them later.
 */
std::shared_ptr<Piece> DefaultPieceStorage::getPiece(size_t index)
{
  std::shared_ptr<Piece> piece;
  if (index <= bitfieldMan_->getMaxIndex()) {
    piece = findUsedPiece(index);
    if (!piece) {
      piece =
          std::make_shared<Piece>(index, bitfieldMan_->getBlockLength(index));
      if (hasPiece(index)) {
        piece->setAllBlock();
      }
    }
  }
  return piece;
}

void DefaultPieceStorage::addUsedPiece(const std::shared_ptr<Piece>& piece)
{
  usedPieces_.insert(piece);
  A2_LOG_DEBUG(fmt("usedPieces_.size()=%lu",
                   static_cast<unsigned long>(usedPieces_.size())));
}

std::shared_ptr<Piece> DefaultPieceStorage::findUsedPiece(size_t index) const
{
  auto p = std::make_shared<Piece>();
  p->setIndex(index);

  auto i = usedPieces_.find(p);
  if (i == usedPieces_.end()) {
    p.reset();
    return p;
  }
  else {
    return *i;
  }
}

#ifdef ENABLE_BITTORRENT

bool DefaultPieceStorage::hasMissingPiece(const std::shared_ptr<Peer>& peer)
{
  return bitfieldMan_->hasMissingPiece(peer->getBitfield(),
                                       peer->getBitfieldLength());
}

void DefaultPieceStorage::getMissingPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const unsigned char* bitfield, size_t length, cuid_t cuid)
{
  const size_t mislen = bitfieldMan_->getBitfieldLength();
  auto misbitfield = make_unique<unsigned char[]>(mislen);
  size_t blocks = bitfieldMan_->countBlock();
  size_t misBlock = 0;
  if (isEndGame()) {
    bool r = bitfieldMan_->getAllMissingIndexes(misbitfield.get(), mislen,
                                                bitfield, length);
    if (!r) {
      return;
    }
    std::vector<size_t> indexes;
    for (size_t i = 0; i < blocks; ++i) {
      if (bitfield::test(misbitfield, blocks, i)) {
        indexes.push_back(i);
      }
    }
    std::shuffle(indexes.begin(), indexes.end(),
                 *SimpleRandomizer::getInstance());
    for (std::vector<size_t>::const_iterator i = indexes.begin(),
                                             eoi = indexes.end();
         i != eoi && misBlock < minMissingBlocks; ++i) {
      std::shared_ptr<Piece> piece = checkOutPiece(*i, cuid);
      if (piece->getUsedBySegment()) {
        // We don't share piece downloaded via HTTP/FTP
        piece->removeUser(cuid);
      }
      else {
        pieces.push_back(piece);
        misBlock += piece->countMissingBlock();
      }
    }
  }
  else {
    bool r = bitfieldMan_->getAllMissingUnusedIndexes(misbitfield.get(), mislen,
                                                      bitfield, length);
    if (!r) {
      return;
    }
    while (misBlock < minMissingBlocks) {
      size_t index;
      if (pieceSelector_->select(index, misbitfield.get(), blocks)) {
        pieces.push_back(checkOutPiece(index, cuid));
        bitfield::flipBit(misbitfield.get(), blocks, index);
        misBlock += pieces.back()->countMissingBlock();
      }
      else {
        break;
      }
    }
  }
}

namespace {
void unsetExcludedIndexes(BitfieldMan& bitfield,
                          const std::vector<size_t>& excludedIndexes)
{
  using namespace std::placeholders;
  std::for_each(excludedIndexes.begin(), excludedIndexes.end(),
                std::bind(&BitfieldMan::unsetBit, &bitfield, _1));
}
} // namespace

void DefaultPieceStorage::createFastIndexBitfield(
    BitfieldMan& bitfield, const std::shared_ptr<Peer>& peer)
{
  const auto& is = peer->getPeerAllowedIndexSet();
  for (const auto& i : is) {
    if (!bitfieldMan_->isBitSet(i) && peer->hasPiece(i)) {
      bitfield.setBit(i);
    }
  }
}

void DefaultPieceStorage::getMissingPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer, cuid_t cuid)
{
  getMissingPiece(pieces, minMissingBlocks, peer->getBitfield(),
                  peer->getBitfieldLength(), cuid);
}

void DefaultPieceStorage::getMissingPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer,
    const std::vector<size_t>& excludedIndexes, cuid_t cuid)
{
  BitfieldMan tempBitfield(bitfieldMan_->getBlockLength(),
                           bitfieldMan_->getTotalLength());
  tempBitfield.setBitfield(peer->getBitfield(), peer->getBitfieldLength());
  unsetExcludedIndexes(tempBitfield, excludedIndexes);
  getMissingPiece(pieces, minMissingBlocks, tempBitfield.getBitfield(),
                  tempBitfield.getBitfieldLength(), cuid);
}

void DefaultPieceStorage::getMissingFastPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer, cuid_t cuid)
{
  if (peer->isFastExtensionEnabled() && peer->countPeerAllowedIndexSet() > 0) {
    BitfieldMan tempBitfield(bitfieldMan_->getBlockLength(),
                             bitfieldMan_->getTotalLength());
    createFastIndexBitfield(tempBitfield, peer);
    getMissingPiece(pieces, minMissingBlocks, tempBitfield.getBitfield(),
                    tempBitfield.getBitfieldLength(), cuid);
  }
}

void DefaultPieceStorage::getMissingFastPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer,
    const std::vector<size_t>& excludedIndexes, cuid_t cuid)
{
  if (peer->isFastExtensionEnabled() && peer->countPeerAllowedIndexSet() > 0) {
    BitfieldMan tempBitfield(bitfieldMan_->getBlockLength(),
                             bitfieldMan_->getTotalLength());
    createFastIndexBitfield(tempBitfield, peer);
    unsetExcludedIndexes(tempBitfield, excludedIndexes);
    getMissingPiece(pieces, minMissingBlocks, tempBitfield.getBitfield(),
                    tempBitfield.getBitfieldLength(), cuid);
  }
}

std::shared_ptr<Piece>
DefaultPieceStorage::getMissingPiece(const std::shared_ptr<Peer>& peer,
                                     cuid_t cuid)
{
  std::vector<std::shared_ptr<Piece>> pieces;
  getMissingPiece(pieces, 1, peer, cuid);
  if (pieces.empty()) {
    return nullptr;
  }
  else {
    return pieces.front();
  }
}

std::shared_ptr<Piece>
DefaultPieceStorage::getMissingPiece(const std::shared_ptr<Peer>& peer,
                                     const std::vector<size_t>& excludedIndexes,
                                     cuid_t cuid)
{
  std::vector<std::shared_ptr<Piece>> pieces;
  getMissingPiece(pieces, 1, peer, excludedIndexes, cuid);
  if (pieces.empty()) {
    return nullptr;
  }
  else {
    return pieces.front();
  }
}

std::shared_ptr<Piece>
DefaultPieceStorage::getMissingFastPiece(const std::shared_ptr<Peer>& peer,
                                         cuid_t cuid)
{
  std::vector<std::shared_ptr<Piece>> pieces;
  getMissingFastPiece(pieces, 1, peer, cuid);
  if (pieces.empty()) {
    return nullptr;
  }
  else {
    return pieces.front();
  }
}

std::shared_ptr<Piece> DefaultPieceStorage::getMissingFastPiece(
    const std::shared_ptr<Peer>& peer,
    const std::vector<size_t>& excludedIndexes, cuid_t cuid)
{
  std::vector<std::shared_ptr<Piece>> pieces;
  getMissingFastPiece(pieces, 1, peer, excludedIndexes, cuid);
  if (pieces.empty()) {
    return nullptr;
  }
  else {
    return pieces.front();
  }
}

#endif // ENABLE_BITTORRENT

bool DefaultPieceStorage::hasMissingUnusedPiece()
{
  size_t index;
  return bitfieldMan_->getFirstMissingUnusedIndex(index);
}

std::shared_ptr<Piece>
DefaultPieceStorage::getMissingPiece(size_t minSplitSize,
                                     const unsigned char* ignoreBitfield,
                                     size_t length, cuid_t cuid)
{
  size_t index;
  if (streamPieceSelector_->select(index, minSplitSize, ignoreBitfield,
                                   length)) {
    return checkOutPiece(index, cuid);
  }
  else {
    return nullptr;
  }
}

std::shared_ptr<Piece> DefaultPieceStorage::getMissingPiece(size_t index,
                                                            cuid_t cuid)
{
  if (hasPiece(index) || isPieceUsed(index) ||
      (bitfieldMan_->isFilterEnabled() &&
       !bitfieldMan_->isFilterBitSet(index))) {
    return nullptr;
  }
  else {
    return checkOutPiece(index, cuid);
  }
}

void DefaultPieceStorage::deleteUsedPiece(const std::shared_ptr<Piece>& piece)
{
  if (!piece) {
    return;
  }
  usedPieces_.erase(piece);
  piece->releaseWrCache(wrDiskCache_);
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
//     std::shared_ptr<Piece>& piece = *itr;
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

void DefaultPieceStorage::completePiece(const std::shared_ptr<Piece>& piece)
{
  if (!piece) {
    return;
  }
  deleteUsedPiece(piece);
  //   if(!isEndGame()) {
  //     reduceUsedPieces(100);
  //   }
  if (allDownloadFinished()) {
    return;
  }
  bitfieldMan_->setBit(piece->getIndex());
  bitfieldMan_->unsetUseBit(piece->getIndex());
  addPieceStats(piece->getIndex());
  if (downloadFinished()) {
    downloadContext_->resetDownloadStopTime();
    if (isSelectiveDownloadingMode()) {
      A2_LOG_NOTICE(MSG_SELECTIVE_DOWNLOAD_COMPLETED);
      // following line was commented out in order to stop sending request
      // message after user-specified files were downloaded.
      // finishSelectiveDownloadingMode();
    }
    else {
      A2_LOG_INFO(MSG_DOWNLOAD_COMPLETED);
    }
#ifdef ENABLE_BITTORRENT
    if (downloadContext_->hasAttribute(CTX_ATTR_BT)) {
      if (!bittorrent::getTorrentAttrs(downloadContext_)->metadata.empty()) {
#  ifdef __MINGW32__
        // On Windows, if aria2 opens files with GENERIC_WRITE access
        // right, some programs cannot open them aria2 is seeding. To
        // avoid this situation, re-open the files with read-only
        // enabled.
        A2_LOG_INFO("Closing files and re-open them with read-only mode"
                    " enabled.");
        diskAdaptor_->closeFile();
        diskAdaptor_->enableReadOnly();
        diskAdaptor_->openFile();
#  endif // __MINGW32__
        auto group = downloadContext_->getOwnerRequestGroup();

        util::executeHookByOptName(group, option_,
                                   PREF_ON_BT_DOWNLOAD_COMPLETE);
        SingletonHolder<Notifier>::instance()->notifyDownloadEvent(
            EVENT_ON_BT_DOWNLOAD_COMPLETE, group);

        group->enableSeedOnly();
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
void DefaultPieceStorage::cancelPiece(const std::shared_ptr<Piece>& piece,
                                      cuid_t cuid)
{
  if (!piece) {
    return;
  }
  piece->removeUser(cuid);
  if (!piece->getUsed()) {
    bitfieldMan_->unsetUseBit(piece->getIndex());
  }
  if (!isEndGame()) {
    if (piece->getCompletedLength() == 0) {
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
      bitfieldMan_->getCompletedLength() + getInFlightPieceCompletedLength();
  int64_t totalLength = getTotalLength();
  if (completedLength > totalLength) {
    completedLength = totalLength;
  }
  return completedLength;
}

int64_t DefaultPieceStorage::getFilteredCompletedLength()
{
  return bitfieldMan_->getFilteredCompletedLength() +
         getInFlightPieceFilteredCompletedLength();
}

int64_t DefaultPieceStorage::getInFlightPieceCompletedLength() const
{
  int64_t len = 0;
  for (auto& elem : usedPieces_) {
    len += elem->getCompletedLength();
  }
  return len;
}

int64_t DefaultPieceStorage::getInFlightPieceFilteredCompletedLength() const
{
  int64_t len = 0;
  for (auto& elem : usedPieces_) {
    if (bitfieldMan_->isFilterBitSet(elem->getIndex())) {
      len += elem->getCompletedLength();
    }
  }
  return len;
}

// not unittested
void DefaultPieceStorage::setupFileFilter()
{
  const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
      downloadContext_->getFileEntries();
  bool allSelected = true;
  for (auto& e : fileEntries) {
    if (!e->isRequested()) {
      allSelected = false;
      break;
    }
  }
  if (allSelected) {
    return;
  }
  for (auto& e : fileEntries) {
    if (e->isRequested()) {
      bitfieldMan_->addFilter(e->getOffset(), e->getLength());
    }
  }
  bitfieldMan_->enableFilter();
}

// not unittested
void DefaultPieceStorage::clearFileFilter() { bitfieldMan_->clearFilter(); }

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
  if (downloadContext_->getFileEntries().size() == 1) {
    A2_LOG_DEBUG("Instantiating DirectDiskAdaptor");
    auto directDiskAdaptor = std::make_shared<DirectDiskAdaptor>();
    directDiskAdaptor->setTotalLength(downloadContext_->getTotalLength());
    directDiskAdaptor->setFileEntries(
        downloadContext_->getFileEntries().begin(),
        downloadContext_->getFileEntries().end());

    directDiskAdaptor->setDiskWriter(
        diskWriterFactory_->newDiskWriter(directDiskAdaptor->getFilePath()));
    diskAdaptor_ = std::move(directDiskAdaptor);
  }
  else {
    A2_LOG_DEBUG("Instantiating MultiDiskAdaptor");
    auto multiDiskAdaptor = std::make_shared<MultiDiskAdaptor>();
    multiDiskAdaptor->setFileEntries(downloadContext_->getFileEntries().begin(),
                                     downloadContext_->getFileEntries().end());
    multiDiskAdaptor->setPieceLength(downloadContext_->getPieceLength());
    diskAdaptor_ = std::move(multiDiskAdaptor);
  }
  if (option_->get(PREF_FILE_ALLOCATION) == V_FALLOC) {
    diskAdaptor_->setFileAllocationMethod(DiskAdaptor::FILE_ALLOC_FALLOC);
  }
  else if (option_->get(PREF_FILE_ALLOCATION) == V_TRUNC) {
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

std::shared_ptr<DiskAdaptor> DefaultPieceStorage::getDiskAdaptor()
{
  return diskAdaptor_;
}

WrDiskCache* DefaultPieceStorage::getWrDiskCache() { return wrDiskCache_; }

void DefaultPieceStorage::flushWrDiskCacheEntry()
{
  if (!wrDiskCache_) {
    return;
  }
  // UsedPieceSet is sorted by piece index. It means we can flush
  // cache by non-decreasing offset, which is good to reduce disk seek
  // unless the file is heavily fragmented.
  for (auto& piece : usedPieces_) {
    auto ce = piece->getWrDiskCacheEntry();
    if (ce) {
      piece->flushWrCache(wrDiskCache_);
      piece->releaseWrCache(wrDiskCache_);
    }
  }
}

int32_t DefaultPieceStorage::getPieceLength(size_t index)
{
  return bitfieldMan_->getBlockLength(index);
}

void DefaultPieceStorage::advertisePiece(cuid_t cuid, size_t index,
                                         Timer registeredTime)
{
  haves_.emplace_back(nextHaveIndex_++, cuid, index, std::move(registeredTime));
}

uint64_t DefaultPieceStorage::getAdvertisedPieceIndexes(
    std::vector<size_t>& indexes, cuid_t myCuid, uint64_t lastHaveIndex)
{
  auto it =
      std::upper_bound(std::begin(haves_), std::end(haves_), lastHaveIndex,
                       [](uint64_t lastHaveIndex, const HaveEntry& have) {
                         return lastHaveIndex < have.haveIndex;
                       });

  if (it == std::end(haves_)) {
    return lastHaveIndex;
  }

  for (; it != std::end(haves_); ++it) {
    indexes.push_back((*it).index);
  }

  return (*(std::end(haves_) - 1)).haveIndex;
}

void DefaultPieceStorage::removeAdvertisedPiece(const Timer& expiry)
{
  auto it = std::upper_bound(std::begin(haves_), std::end(haves_), expiry,
                             [](const Timer& expiry, const HaveEntry& have) {
                               return expiry < have.registeredTime;
                             });

  A2_LOG_DEBUG(
      fmt(MSG_REMOVED_HAVE_ENTRY,
          static_cast<unsigned long>(std::distance(std::begin(haves_), it))));

  haves_.erase(std::begin(haves_), it);
}

void DefaultPieceStorage::markAllPiecesDone() { bitfieldMan_->setAllBit(); }

void DefaultPieceStorage::markPiecesDone(int64_t length)
{
  if (length == bitfieldMan_->getTotalLength()) {
    bitfieldMan_->setAllBit();
  }
  else if (length == 0) {
    // TODO this would go to markAllPiecesUndone()
    bitfieldMan_->clearAllBit();
    usedPieces_.clear();
  }
  else {
    size_t numPiece = length / bitfieldMan_->getBlockLength();
    if (numPiece > 0) {
      bitfieldMan_->setBitRange(0, numPiece - 1);
    }
    size_t r = (length % bitfieldMan_->getBlockLength()) / Piece::BLOCK_LENGTH;
    if (r > 0) {
      auto p = std::make_shared<Piece>(numPiece,
                                       bitfieldMan_->getBlockLength(numPiece));

      for (size_t i = 0; i < r; ++i) {
        p->completeBlock(i);
      }

      p->setHashType(downloadContext_->getPieceHashType());

      addUsedPiece(p);
    }
  }
}

void DefaultPieceStorage::markPieceMissing(size_t index)
{
  bitfieldMan_->unsetBit(index);
}

void DefaultPieceStorage::addInFlightPiece(
    const std::vector<std::shared_ptr<Piece>>& pieces)
{
  usedPieces_.insert(pieces.begin(), pieces.end());
}

size_t DefaultPieceStorage::countInFlightPiece() { return usedPieces_.size(); }

void DefaultPieceStorage::getInFlightPieces(
    std::vector<std::shared_ptr<Piece>>& pieces)
{
  pieces.insert(pieces.end(), usedPieces_.begin(), usedPieces_.end());
}

void DefaultPieceStorage::setDiskWriterFactory(
    const std::shared_ptr<DiskWriterFactory>& diskWriterFactory)
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
  pieceStatMan_->updatePieceStats(newBitfield, newBitfieldLength, oldBitfield);
}

void DefaultPieceStorage::addPieceStats(size_t index)
{
  pieceStatMan_->addPieceStats(index);
}

size_t DefaultPieceStorage::getNextUsedIndex(size_t index)
{
  for (size_t i = index + 1; i < bitfieldMan_->countBlock(); ++i) {
    if (bitfieldMan_->isUseBitSet(i) || bitfieldMan_->isBitSet(i)) {
      return i;
    }
  }
  return bitfieldMan_->countBlock();
}

void DefaultPieceStorage::onDownloadIncomplete()
{
  streamPieceSelector_->onBitfieldInit();
}

void DefaultPieceStorage::setPieceSelector(
    std::unique_ptr<PieceSelector> pieceSelector)
{
  pieceSelector_ = std::move(pieceSelector);
}

std::unique_ptr<PieceSelector> DefaultPieceStorage::popPieceSelector()
{
  return std::move(pieceSelector_);
}

} // namespace aria2
