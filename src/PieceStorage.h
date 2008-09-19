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
#ifndef _D_PIECE_STORAGE_H_
#define _D_PIECE_STORAGE_H_

#include "common.h"
#include "SharedHandle.h"
#include "TimeA2.h"
#include "IntSequence.h"
#include <string>
#include <deque>

namespace aria2 {

class Piece;
class Peer;
class DiskAdaptor;

class PieceStorage {
public:
  virtual ~PieceStorage() {}

  /**
   * Returns true if the peer has a piece that localhost doesn't have.
   * Otherwise returns false.
   */
  virtual bool hasMissingPiece(const SharedHandle<Peer>& peer) = 0;

  /**
   * Returns a piece that the peer has but localhost doesn't.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual SharedHandle<Piece>
  getMissingPiece(const SharedHandle<Peer>& peer) = 0;

  /**
   * Same as getMissingPiece(const SharedHandle<Peer>& peer), but the indexes in
   * excludedIndexes are excluded.
   */
  virtual SharedHandle<Piece> getMissingPiece
  (const SharedHandle<Peer>& peer,
   const std::deque<size_t>& excludedIndexes) = 0;

  /**
   * Returns a piece that the peer has but localhost doesn't.
   * Only pieces that declared as "fast" are returned.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual SharedHandle<Piece>
  getMissingFastPiece(const SharedHandle<Peer>& peer) = 0;
  
  /**
   * Same as getMissingFastPiece(const SharedHandle<Peer>& peer), but the
   * indexes in excludedIndexes are excluded.
   */
  virtual SharedHandle<Piece> getMissingFastPiece
  (const SharedHandle<Peer>& peer,
   const std::deque<size_t>& excludedIndexes) = 0;

  /**
   * Returns a missing piece if available. Otherwise returns 0;
   */
  virtual SharedHandle<Piece> getMissingPiece() = 0;

  /**
   * Returns a missing piece whose index is index.
   * If a piece whose index is index is already acquired or currently used,
   * then returns 0.
   * Also returns 0 if any of missing piece is not available.
   */
  virtual SharedHandle<Piece> getMissingPiece(size_t index) = 0;

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
  virtual void cancelPiece(const SharedHandle<Piece>& piece) = 0;

  /**
   * Returns true if the specified piece is already downloaded.
   * Otherwise returns false.
   */
  virtual bool hasPiece(size_t index) = 0;

  virtual bool isPieceUsed(size_t index) = 0;

  virtual uint64_t getTotalLength() = 0;

  virtual uint64_t getFilteredTotalLength() = 0;

  virtual uint64_t getCompletedLength() = 0;

  virtual uint64_t getFilteredCompletedLength() = 0;
  
  virtual void setFileFilter(const std::deque<std::string>& filePaths) = 0;

  virtual void setFileFilter(IntSequence seq) = 0;

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

  virtual void finishSelectiveDownloadingMode() = 0;

  virtual bool isEndGame() = 0;

  virtual SharedHandle<DiskAdaptor> getDiskAdaptor() = 0;
  
  virtual size_t getPieceLength(size_t index) = 0;

  /**
   * Adds piece index to advertise to other commands. They send have message
   * based on this information.
   */
  virtual void advertisePiece(int32_t cuid, size_t index) = 0;

  /**
   * indexes is filled with piece index which is not advertised by the caller
   * command and newer than lastCheckTime.
   */
  virtual void getAdvertisedPieceIndexes(std::deque<size_t>& indexes,
					 int32_t myCuid,
					 const Time& lastCheckTime) = 0;

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
  virtual void markPiecesDone(uint64_t length) = 0;

  virtual void
  addInFlightPiece(const std::deque<SharedHandle<Piece> >& pieces) = 0;

  virtual size_t countInFlightPiece() = 0;

  virtual void getInFlightPieces(std::deque<SharedHandle<Piece> >& pieces) = 0;

  virtual void addPieceStats(size_t index) = 0;

  virtual void addPieceStats(const unsigned char* bitfield,
			     size_t bitfieldLength) = 0;

  virtual void subtractPieceStats(const unsigned char* bitfield,
				  size_t bitfieldLength) = 0;

  virtual void updatePieceStats(const unsigned char* newBitfield,
				size_t newBitfieldLength,
				const unsigned char* oldBitfield) = 0;

};

typedef SharedHandle<PieceStorage> PieceStorageHandle;

} // namespace aria2

#endif // _D_PIECE_STORAGE_H_
