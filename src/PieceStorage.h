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
#ifndef D_PIECE_STORAGE_H
#define D_PIECE_STORAGE_H

#include "common.h"

#include <string>
#include <vector>

#include "SharedHandle.h"
#include "TimerA2.h"
#include "Command.h"

namespace aria2 {

class Piece;
#ifdef ENABLE_BITTORRENT
class Peer;
#endif // ENABLE_BITTORRENT
class DiskAdaptor;

class PieceStorage {
public:
  virtual ~PieceStorage() {}

#ifdef ENABLE_BITTORRENT
  /**
   * Returns true if the peer has a piece that localhost doesn't have.
   * Otherwise returns false.
   */
  virtual bool hasMissingPiece(const SharedHandle<Peer>& peer) = 0;

   // Stores pieces that the peer has but localhost doesn't.  Those
   // pieces will be marked "used" status in order to prevent other
   // command from get the same piece. But in end game mode, same
   // piece may be got by several commands. This function stores N
   // pieces where the sum of missing block of first N-1 pieces is
   // less than minMissingBlocks and the sum of missing block of N
   // pieces is greater than or equal to minMissingBlocks. If there is
   // M missing pieces left where M < N, M pieces are stored.
  virtual void getMissingPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   cuid_t cuid) = 0;

  // Same as getMissingPiece(pieces, minMissingBlocks, peer), but the
  // indexes in excludedIndexes are excluded.
  virtual void getMissingPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   const std::vector<size_t>& excludedIndexes,
   cuid_t cuid) = 0;
  
  // Stores pieces that the peer has but localhost doesn't.  Only
  // pieces that declared as "fast" are stored.  Those pieces stored
  // will be marked "used" status in order to prevent other command
  // from get the same piece. But in end game mode, same piece may be
  // got by several commands. This function stores N pieces where the
  // sum of missing block of first N-1 pieces is less than
  // minMissingBlocks and the sum of missing block of N pieces is
  // greater than or equal to minMissingBlocks. If there is M missing
  // pieces left where M < N, M pieces are stored.
  virtual void getMissingFastPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   cuid_t cuid) = 0;

  // Same as getMissingFastPiece(pieces, minMissingBlocks, peer), but
  // the indexes in excludedIndexes are excluded.
  virtual void getMissingFastPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   const std::vector<size_t>& excludedIndexes,
   cuid_t cuid) = 0;

  /**
   * Returns a piece that the peer has but localhost doesn't.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual SharedHandle<Piece>
  getMissingPiece(const SharedHandle<Peer>& peer, cuid_t cuid) = 0;

  /**
   * Same as getMissingPiece(const SharedHandle<Peer>& peer), but the indexes in
   * excludedIndexes are excluded.
   */
  virtual SharedHandle<Piece> getMissingPiece
  (const SharedHandle<Peer>& peer,
   const std::vector<size_t>& excludedIndexes,
   cuid_t cuid) = 0;
#endif // ENABLE_BITTORRENT

  // Returns true if there is at least one missing and unused piece.
  virtual bool hasMissingUnusedPiece() = 0;

  /**
   * Returns a missing piece if available. Otherwise returns 0;
   * If ignoreBitfield is set, indexes of true bit are excluded.
   */
  virtual SharedHandle<Piece> getMissingPiece
  (size_t minSplitSize,
   const unsigned char* ignoreBitfield,
   size_t length,
   cuid_t cuid) = 0;

  /**
   * Returns a missing piece whose index is index.
   * If a piece whose index is index is already acquired or currently used,
   * then returns 0.
   * Also returns 0 if any of missing piece is not available.
   */
  virtual SharedHandle<Piece> getMissingPiece(size_t index, cuid_t cuid) = 0;

  /**
   * Returns the piece denoted by index.
   * No status of the piece is changed in this method.
   */
  virtual SharedHandle<Piece> getPiece(size_t index) = 0;

  /**
   * Marks the piece whose index is index as missing.
   */
  virtual void markPieceMissing(size_t index) = 0;

  /**
   * Tells that the download of the specfied piece completes.
   */
  virtual void completePiece(const SharedHandle<Piece>& piece) = 0;

  /**
   * Tells that the download of the specified piece is canceled.
   */
  virtual void cancelPiece(const SharedHandle<Piece>& piece, cuid_t cuid) = 0;

  /**
   * Returns true if the specified piece is already downloaded.
   * Otherwise returns false.
   */
  virtual bool hasPiece(size_t index) = 0;

  virtual bool isPieceUsed(size_t index) = 0;

  virtual int64_t getTotalLength() = 0;

  virtual int64_t getFilteredTotalLength() = 0;

  virtual int64_t getCompletedLength() = 0;

  virtual int64_t getFilteredCompletedLength() = 0;

  virtual void setupFileFilter() = 0;
  
  virtual void clearFileFilter() = 0;
  
  /**
   * Returns true if download has completed.
   * If file filter is enabled, then returns true if those files have
   * downloaded.
   */
  virtual bool downloadFinished() = 0;

  /**
   * Returns true if all files have downloaded.
   * The file filter is ignored.
   */
  virtual bool allDownloadFinished() = 0;

  /**
   * Initializes DiskAdaptor.
   * TODO add better documentation here.
   */
  virtual void initStorage() = 0;

  virtual const unsigned char* getBitfield() = 0;

  virtual void setBitfield(const unsigned char* bitfield,
                           size_t bitfieldLength) = 0;
  
  virtual size_t getBitfieldLength() = 0;

  virtual bool isSelectiveDownloadingMode() = 0;

  virtual bool isEndGame() = 0;

  virtual void enterEndGame() = 0;

  // TODO We can remove this.
  virtual void setEndGamePieceNum(size_t num) = 0;

  virtual SharedHandle<DiskAdaptor> getDiskAdaptor() = 0;
  
  virtual int32_t getPieceLength(size_t index) = 0;

  /**
   * Adds piece index to advertise to other commands. They send have message
   * based on this information.
   */
  virtual void advertisePiece(cuid_t cuid, size_t index) = 0;

  /**
   * indexes is filled with piece index which is not advertised by the caller
   * command and newer than lastCheckTime.
   */
  virtual void getAdvertisedPieceIndexes(std::vector<size_t>& indexes,
                                         cuid_t myCuid,
                                         const Timer& lastCheckTime) = 0;

  /**
   * Removes have entry if specified seconds have elapsed since its
   * registration.
   */
  virtual void removeAdvertisedPiece(time_t elapsed) = 0;

  /**
   * Sets all bits in bitfield to 1.
   */
  virtual void markAllPiecesDone() = 0;

  /**
   * Sets all bits in bitfield(0 to length) to 1.
   */
  virtual void markPiecesDone(int64_t length) = 0;

  virtual void
  addInFlightPiece(const std::vector<SharedHandle<Piece> >& pieces) = 0;

  virtual size_t countInFlightPiece() = 0;

  virtual void getInFlightPieces(std::vector<SharedHandle<Piece> >& pieces) = 0;

  virtual void addPieceStats(size_t index) = 0;

  virtual void addPieceStats(const unsigned char* bitfield,
                             size_t bitfieldLength) = 0;

  virtual void subtractPieceStats(const unsigned char* bitfield,
                                  size_t bitfieldLength) = 0;

  virtual void updatePieceStats(const unsigned char* newBitfield,
                                size_t newBitfieldLength,
                                const unsigned char* oldBitfield) = 0;

  // Returns index x where all pieces in [index+1, x-1], inclusive,
  // are not used and not completed. If all pieces after index+1 are
  // used or completed, returns the number of pieces.
  virtual size_t getNextUsedIndex(size_t index) = 0;

  // Called when system detects download is not finished
  virtual void onDownloadIncomplete() = 0;
};

typedef SharedHandle<PieceStorage> PieceStorageHandle;

} // namespace aria2

#endif // D_PIECE_STORAGE_H
