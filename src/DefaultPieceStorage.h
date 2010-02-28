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
#ifndef _D_DEFAULT_PIECE_STORAGE_H_
#define _D_DEFAULT_PIECE_STORAGE_H_

#include "PieceStorage.h"

#include <deque>

namespace aria2 {

class DownloadContext;
class BitfieldMan;
class Logger;
class Option;
class DiskWriterFactory;
class FileEntry;
class PieceStatMan;
class PieceSelector;

#define END_GAME_PIECE_NUM 20

class HaveEntry {
private:
  int32_t cuid;
  size_t index;
  Time registeredTime;
public:
  HaveEntry(int32_t cuid, size_t index):
    cuid(cuid),
    index(index) {}

  int32_t getCuid() const { return cuid; }

  size_t getIndex() const { return index; }

  const Time& getRegisteredTime() const { return registeredTime; }
};

class DefaultPieceStorage : public PieceStorage {
private:
  SharedHandle<DownloadContext> downloadContext;
  BitfieldMan* bitfieldMan;
  SharedHandle<DiskAdaptor> diskAdaptor;
  SharedHandle<DiskWriterFactory> _diskWriterFactory;
  std::deque<SharedHandle<Piece> > usedPieces;

  size_t endGamePieceNum;
  Logger* logger;
  const Option* option;
  std::deque<HaveEntry> haves;

  SharedHandle<PieceStatMan> _pieceStatMan;

  SharedHandle<PieceSelector> _pieceSelector;

  bool getMissingPieceIndex(size_t& index,
                            const unsigned char* bitfield, size_t length);

  SharedHandle<Piece> getMissingPiece(const unsigned char* bitfield,
                                      size_t length);

  SharedHandle<Piece> getMissingPiece(const BitfieldMan& bitfield);

#ifdef ENABLE_BITTORRENT
  void createFastIndexBitfield(BitfieldMan& bitfield,
                               const SharedHandle<Peer>& peer);
#endif // ENABLE_BITTORRENT

  SharedHandle<Piece> checkOutPiece(size_t index);
  //   size_t deleteUsedPiecesByFillRate(int fillRate, size_t toDelete);
  //   void reduceUsedPieces(size_t upperBound);
  void deleteUsedPiece(const SharedHandle<Piece>& piece);
  SharedHandle<Piece> findUsedPiece(size_t index) const;

  size_t getInFlightPieceCompletedLength() const;

public:
  // Setting randomPieceStatsOrdering to true means a piece is chosen in
  // random when more than 2 pieces has the same rarity.
  // If it is set to false, a piece whose index is smallest has the highest
  // priority.
  DefaultPieceStorage(const SharedHandle<DownloadContext>& downloadContext,
                      const Option* option);
                      
  virtual ~DefaultPieceStorage();

#ifdef ENABLE_BITTORRENT

  virtual bool hasMissingPiece(const SharedHandle<Peer>& peer);

  virtual SharedHandle<Piece> getMissingPiece(const SharedHandle<Peer>& peer);

  virtual SharedHandle<Piece> getMissingFastPiece(const SharedHandle<Peer>& peer);

  virtual SharedHandle<Piece> getMissingPiece
  (const SharedHandle<Peer>& peer, const std::vector<size_t>& excludedIndexes);

  virtual SharedHandle<Piece> getMissingFastPiece
  (const SharedHandle<Peer>& peer, const std::vector<size_t>& excludedIndexes);

#endif // ENABLE_BITTORRENT

  virtual bool hasMissingUnusedPiece();

  virtual SharedHandle<Piece> getSparseMissingUnusedPiece
  (const unsigned char* ignoreBitfield, size_t length);

  virtual SharedHandle<Piece> getMissingPiece(size_t index);

  virtual SharedHandle<Piece> getPiece(size_t index);

  virtual void completePiece(const SharedHandle<Piece>& piece);

  virtual void cancelPiece(const SharedHandle<Piece>& piece);

  virtual bool hasPiece(size_t index);

  virtual bool isPieceUsed(size_t index);

  virtual uint64_t getTotalLength();

  virtual uint64_t getFilteredTotalLength();

  virtual uint64_t getCompletedLength();

  virtual uint64_t getFilteredCompletedLength();

  virtual void initStorage();

  virtual void setupFileFilter();
  
  virtual void clearFileFilter();

  virtual bool downloadFinished();

  virtual bool allDownloadFinished();

  virtual void setBitfield(const unsigned char* bitfield,
                           size_t bitfieldLength);
  
  virtual size_t getBitfieldLength();

  virtual const unsigned char* getBitfield();

  virtual void setEndGamePieceNum(size_t num) {
    endGamePieceNum = num;
  }

  size_t getEndGamePieceNum() const {
    return endGamePieceNum;
  }

  virtual bool isSelectiveDownloadingMode();

  virtual bool isEndGame();
  
  virtual SharedHandle<DiskAdaptor> getDiskAdaptor();

  virtual size_t getPieceLength(size_t index);

  virtual void advertisePiece(int32_t cuid, size_t index);

  virtual void
  getAdvertisedPieceIndexes(std::vector<size_t>& indexes,
                            int32_t myCuid, const Time& lastCheckTime);

  virtual void removeAdvertisedPiece(time_t elapsed);

  virtual void markAllPiecesDone();

  virtual void markPiecesDone(uint64_t length);

  virtual void markPieceMissing(size_t index);

  virtual void addInFlightPiece
  (const std::vector<SharedHandle<Piece> >& pieces);

  virtual size_t countInFlightPiece();

  virtual void getInFlightPieces
  (std::vector<SharedHandle<Piece> >& pieces);

  virtual void addPieceStats(size_t index);

  virtual void addPieceStats(const unsigned char* bitfield,
                             size_t bitfieldLength);
  
  virtual void subtractPieceStats(const unsigned char* bitfield,
                                  size_t bitfieldLength);

  virtual void updatePieceStats(const unsigned char* newBitfield,
                                size_t newBitfieldLength,
                                const unsigned char* oldBitfield);

  /**
   * This method is made private for test purpose only.
   */
  void addUsedPiece(const SharedHandle<Piece>& piece);

  void setDiskWriterFactory(const SharedHandle<DiskWriterFactory>& diskWriterFactory);

  const SharedHandle<PieceStatMan>& getPieceStatMan() const
  {
    return _pieceStatMan;
  }

  void setPieceSelector(const SharedHandle<PieceSelector>& pieceSelector)
  {
    _pieceSelector = pieceSelector;
  }

  SharedHandle<PieceSelector> getPieceSelector() const
  {
    return _pieceSelector;
  }
    
};

typedef SharedHandle<DefaultPieceStorage> DefaultPieceStorageHandle;

} // namespace aria2

#endif // _D_DEFAULT_PIECE_STORAGE_H_
