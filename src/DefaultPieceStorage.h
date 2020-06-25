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
#ifndef D_DEFAULT_PIECE_STORAGE_H
#define D_DEFAULT_PIECE_STORAGE_H

#include "PieceStorage.h"

#include <deque>
#include <set>

#include "a2functional.h"

namespace aria2 {

class DownloadContext;
class BitfieldMan;
class Option;
class DiskWriterFactory;
class FileEntry;
class PieceStatMan;
class PieceSelector;
class StreamPieceSelector;

#define END_GAME_PIECE_NUM 20

struct HaveEntry {
  HaveEntry(uint64_t haveIndex, cuid_t cuid, size_t index, Timer registeredTime)
      : haveIndex(haveIndex),
        cuid(cuid),
        index(index),
        registeredTime(std::move(registeredTime))
  {
  }

  uint64_t haveIndex;
  cuid_t cuid;
  size_t index;
  Timer registeredTime;
};

class DefaultPieceStorage : public PieceStorage {
private:
  std::shared_ptr<DownloadContext> downloadContext_;
  std::unique_ptr<BitfieldMan> bitfieldMan_;
  std::shared_ptr<DiskAdaptor> diskAdaptor_;
  std::shared_ptr<DiskWriterFactory> diskWriterFactory_;
  typedef std::set<std::shared_ptr<Piece>, DerefLess<std::shared_ptr<Piece>>>
      UsedPieceSet;
  UsedPieceSet usedPieces_;

  bool endGame_;
  size_t endGamePieceNum_;
  const Option* option_;

  // The next unique index on HaveEntry, which is ever strictly
  // increasing sequence of integer.
  uint64_t nextHaveIndex_;
  std::deque<HaveEntry> haves_;

  std::shared_ptr<PieceStatMan> pieceStatMan_;

  std::unique_ptr<PieceSelector> pieceSelector_;
  std::unique_ptr<StreamPieceSelector> streamPieceSelector_;

  WrDiskCache* wrDiskCache_;
#ifdef ENABLE_BITTORRENT
  void getMissingPiece(std::vector<std::shared_ptr<Piece>>& pieces,
                       size_t minMissingBlocks, const unsigned char* bitfield,
                       size_t length, cuid_t cuid);

  void createFastIndexBitfield(BitfieldMan& bitfield,
                               const std::shared_ptr<Peer>& peer);
#endif // ENABLE_BITTORRENT

  std::shared_ptr<Piece> checkOutPiece(size_t index, cuid_t cuid);
  //   size_t deleteUsedPiecesByFillRate(int fillRate, size_t toDelete);
  //   void reduceUsedPieces(size_t upperBound);
  void deleteUsedPiece(const std::shared_ptr<Piece>& piece);
  std::shared_ptr<Piece> findUsedPiece(size_t index) const;

  // Returns the sum of completed length of in-flight pieces
  int64_t getInFlightPieceCompletedLength() const;
  // Returns the sum of completed length of in-flight pieces
  // intersecting filter ranges.
  int64_t getInFlightPieceFilteredCompletedLength() const;

public:
  // Setting randomPieceStatsOrdering to true means a piece is chosen in
  // random when more than 2 pieces has the same rarity.
  // If it is set to false, a piece whose index is smallest has the highest
  // priority.
  DefaultPieceStorage(const std::shared_ptr<DownloadContext>& downloadContext,
                      const Option* option);

  virtual ~DefaultPieceStorage();

#ifdef ENABLE_BITTORRENT

  virtual bool
  hasMissingPiece(const std::shared_ptr<Peer>& peer) CXX11_OVERRIDE;

  virtual void getMissingPiece(std::vector<std::shared_ptr<Piece>>& pieces,
                               size_t minMissingBlocks,
                               const std::shared_ptr<Peer>& peer,
                               cuid_t cuid) CXX11_OVERRIDE;

  virtual void getMissingPiece(std::vector<std::shared_ptr<Piece>>& pieces,
                               size_t minMissingBlocks,
                               const std::shared_ptr<Peer>& peer,
                               const std::vector<size_t>& excludedIndexes,
                               cuid_t cuid) CXX11_OVERRIDE;

  virtual void getMissingFastPiece(std::vector<std::shared_ptr<Piece>>& pieces,
                                   size_t minMissingBlocks,
                                   const std::shared_ptr<Peer>& peer,
                                   cuid_t cuid) CXX11_OVERRIDE;

  virtual void getMissingFastPiece(std::vector<std::shared_ptr<Piece>>& pieces,
                                   size_t minMissingBlocks,
                                   const std::shared_ptr<Peer>& peer,
                                   const std::vector<size_t>& excludedIndexes,
                                   cuid_t cuid) CXX11_OVERRIDE;

  virtual std::shared_ptr<Piece>
  getMissingPiece(const std::shared_ptr<Peer>& peer,
                  cuid_t cuid) CXX11_OVERRIDE;

  virtual std::shared_ptr<Piece>
  getMissingPiece(const std::shared_ptr<Peer>& peer,
                  const std::vector<size_t>& excludedIndexes,
                  cuid_t cuid) CXX11_OVERRIDE;

  std::shared_ptr<Piece> getMissingFastPiece(const std::shared_ptr<Peer>& peer,
                                             cuid_t cuid);

  std::shared_ptr<Piece>
  getMissingFastPiece(const std::shared_ptr<Peer>& peer,
                      const std::vector<size_t>& excludedIndexes, cuid_t cuid);

#endif // ENABLE_BITTORRENT

  virtual bool hasMissingUnusedPiece() CXX11_OVERRIDE;

  virtual std::shared_ptr<Piece>
  getMissingPiece(size_t minSplitSize, const unsigned char* ignoreBitfield,
                  size_t length, cuid_t cuid) CXX11_OVERRIDE;

  virtual std::shared_ptr<Piece> getMissingPiece(size_t index,
                                                 cuid_t cuid) CXX11_OVERRIDE;

  virtual std::shared_ptr<Piece> getPiece(size_t index) CXX11_OVERRIDE;

  virtual void
  completePiece(const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE;

  virtual void cancelPiece(const std::shared_ptr<Piece>& piece,
                           cuid_t cuid) CXX11_OVERRIDE;

  virtual bool hasPiece(size_t index) CXX11_OVERRIDE;

  virtual bool isPieceUsed(size_t index) CXX11_OVERRIDE;

  virtual int64_t getTotalLength() CXX11_OVERRIDE;

  virtual int64_t getFilteredTotalLength() CXX11_OVERRIDE;

  virtual int64_t getCompletedLength() CXX11_OVERRIDE;

  virtual int64_t getFilteredCompletedLength() CXX11_OVERRIDE;

  virtual void initStorage() CXX11_OVERRIDE;

  virtual void setupFileFilter() CXX11_OVERRIDE;

  virtual void clearFileFilter() CXX11_OVERRIDE;

  virtual bool downloadFinished() CXX11_OVERRIDE;

  virtual bool allDownloadFinished() CXX11_OVERRIDE;

  virtual void setBitfield(const unsigned char* bitfield,
                           size_t bitfieldLength) CXX11_OVERRIDE;

  virtual size_t getBitfieldLength() CXX11_OVERRIDE;

  virtual const unsigned char* getBitfield() CXX11_OVERRIDE;

  virtual void setEndGamePieceNum(size_t num) CXX11_OVERRIDE
  {
    endGamePieceNum_ = num;
  }

  size_t getEndGamePieceNum() const { return endGamePieceNum_; }

  virtual bool isSelectiveDownloadingMode() CXX11_OVERRIDE;

  virtual bool isEndGame() CXX11_OVERRIDE { return endGame_; }

  virtual void enterEndGame() CXX11_OVERRIDE { endGame_ = true; }

  virtual std::shared_ptr<DiskAdaptor> getDiskAdaptor() CXX11_OVERRIDE;

  virtual WrDiskCache* getWrDiskCache() CXX11_OVERRIDE;

  virtual void flushWrDiskCacheEntry(bool releaseEntries) CXX11_OVERRIDE;

  virtual int32_t getPieceLength(size_t index) CXX11_OVERRIDE;

  virtual void advertisePiece(cuid_t cuid, size_t index,
                              Timer registeredTime) CXX11_OVERRIDE;

  virtual uint64_t
  getAdvertisedPieceIndexes(std::vector<size_t>& indexes, cuid_t myCuid,
                            uint64_t lastHaveIndex) CXX11_OVERRIDE;

  virtual void removeAdvertisedPiece(const Timer& expiry) CXX11_OVERRIDE;

  virtual void markAllPiecesDone() CXX11_OVERRIDE;

  virtual void markPiecesDone(int64_t length) CXX11_OVERRIDE;

  virtual void markPieceMissing(size_t index) CXX11_OVERRIDE;

  virtual void addInFlightPiece(
      const std::vector<std::shared_ptr<Piece>>& pieces) CXX11_OVERRIDE;

  virtual size_t countInFlightPiece() CXX11_OVERRIDE;

  virtual void
  getInFlightPieces(std::vector<std::shared_ptr<Piece>>& pieces) CXX11_OVERRIDE;

  virtual void addPieceStats(size_t index) CXX11_OVERRIDE;

  virtual void addPieceStats(const unsigned char* bitfield,
                             size_t bitfieldLength) CXX11_OVERRIDE;

  virtual void subtractPieceStats(const unsigned char* bitfield,
                                  size_t bitfieldLength) CXX11_OVERRIDE;

  virtual void
  updatePieceStats(const unsigned char* newBitfield, size_t newBitfieldLength,
                   const unsigned char* oldBitfield) CXX11_OVERRIDE;

  virtual size_t getNextUsedIndex(size_t index) CXX11_OVERRIDE;

  virtual void onDownloadIncomplete() CXX11_OVERRIDE;

  /**
   * This method is made private for test purpose only.
   */
  void addUsedPiece(const std::shared_ptr<Piece>& piece);

  void setDiskWriterFactory(
      const std::shared_ptr<DiskWriterFactory>& diskWriterFactory);

  const std::shared_ptr<PieceStatMan>& getPieceStatMan() const
  {
    return pieceStatMan_;
  }

  void setPieceSelector(std::unique_ptr<PieceSelector> pieceSelector);

  const std::unique_ptr<PieceSelector>& getPieceSelector() const
  {
    return pieceSelector_;
  }

  std::unique_ptr<PieceSelector> popPieceSelector();

  void setWrDiskCache(WrDiskCache* wrDiskCache) { wrDiskCache_ = wrDiskCache; }
};

} // namespace aria2

#endif // D_DEFAULT_PIECE_STORAGE_H
