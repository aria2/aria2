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
#include "Peer.h"
#include "Piece.h"
#include "DiskAdaptor.h"

class PieceStorage {
public:
  virtual ~PieceStorage() {}

  /**
   * Returns true if the peer has a piece that localhost doesn't have.
   * Otherwise returns false.
   */
  virtual bool hasMissingPiece(const PeerHandle& peer) = 0;

  /**
   * Returns a piece that the peer has but localhost doesn't.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual PieceHandle getMissingPiece(const PeerHandle& peer) = 0;
  /**
   * Returns a piece that the peer has but localhost doesn't.
   * Only pieces that declared as "fast" are returned.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual PieceHandle getMissingFastPiece(const PeerHandle& peer) = 0;

  /**
   * Returns the piece denoted by index.
   * No status of the piece is changed in this method.
   */
  virtual PieceHandle getPiece(int32_t index) = 0;

  /**
   * Tells that the download of the specfied piece completes.
   */
  virtual void completePiece(const PieceHandle& piece) = 0;

  /**
   * Tells that the download of the specified piece is canceled.
   */
  virtual void cancelPiece(const PieceHandle& piece) = 0;

  /**
   * Returns true if the specified piece is already downloaded.
   * Otherwise returns false.
   */
  virtual bool hasPiece(int32_t index) = 0;

  virtual int64_t getTotalLength() = 0;

  virtual int64_t getFilteredTotalLength() = 0;

  virtual int64_t getCompletedLength() = 0;

  virtual int64_t getFilteredCompletedLength() = 0;
  
  virtual void setFileFilter(const Strings& filePaths) = 0;

  virtual void setFileFilter(const Integers& fileIndexes) = 0;

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
			   int32_t bitfieldLength) = 0;
  
  virtual int32_t getBitfieldLength() = 0;

  virtual bool isSelectiveDownloadingMode() = 0;

  virtual void finishSelectiveDownloadingMode() = 0;

  virtual bool isEndGame() = 0;

  virtual DiskAdaptorHandle getDiskAdaptor() = 0;
  
  virtual int32_t getPieceLength(int32_t index) = 0;

  /**
   * Adds piece index to advertise to other commands. They send have message
   * based on this information.
   */
  virtual void advertisePiece(int32_t cuid, int32_t index) = 0;

  /**
   * Returns piece index which is not advertised by the caller command and
   * newer than lastCheckTime.
   */
  virtual Integers getAdvertisedPieceIndexes(int32_t myCuid,
					     const Time& lastCheckTime) = 0;

  /**
   * Removes have entry if specified seconds have elapsed since its
   * registration.
   */
  virtual void removeAdvertisedPiece(int32_t elapsed) = 0;

  /**
   * Sets all bits in bitfield to 1.
   */
  virtual void markAllPiecesDone() = 0;

  /**
   * Validates file integrity by comparing checksums.
   */
  virtual void checkIntegrity() = 0;
};

typedef SharedHandle<PieceStorage> PieceStorageHandle;

#endif // _D_PIECE_STORAGE_H_
