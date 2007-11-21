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
#ifndef _D_UNKNOWN_LENGTH_PIECE_STORAGE_H_
#define _D_UNKNOWN_LENGTH_PIECE_STORAGE_H_

#include "PieceStorage.h"

class Option;
class DownloadContext;
extern typedef SharedHandle<DownloadContext> DownloadContextHandle;
class DiskWriterFactory;
extern typedef SharedHandle<DiskWriterFactory> DiskWriterFactoryHandle;

class UnknownLengthPieceStorage:public PieceStorage {
private:
  DownloadContextHandle _downloadContext;

  const Option* _option;
  
  DiskAdaptorHandle _diskAdaptor;

  DiskWriterFactoryHandle _diskWriterFactory;

  int64_t _totalLength;

  bool _downloadFinished;

  PieceHandle _piece;
public:
  UnknownLengthPieceStorage(const DownloadContextHandle& downloadContext,
			    const Option* option);

  virtual ~UnknownLengthPieceStorage();

  /**
   * Returns true if the peer has a piece that localhost doesn't have.
   * Otherwise returns false.
   */
  virtual bool hasMissingPiece(const PeerHandle& peer)
  {
    abort();
  }

  /**
   * Returns a piece that the peer has but localhost doesn't.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual PieceHandle getMissingPiece(const PeerHandle& peer)
  {
    abort();
  }
  /**
   * Returns a piece that the peer has but localhost doesn't.
   * Only pieces that declared as "fast" are returned.
   * The piece will be marked "used" status in order to prevent other command
   * from get the same piece. But in end game mode, same piece may be returned
   * to several commands.
   */
  virtual PieceHandle getMissingFastPiece(const PeerHandle& peer)
  {
    abort();
  }
  /**
   * Returns a missing piece if available. Otherwise returns 0;
   */
  virtual PieceHandle getMissingPiece();

  /**
   * Returns a missing piece whose index is index.
   * If a piece whose index is index is already acquired or currently used,
   * then returns 0.
   * Also returns 0 if any of missing piece is not available.
   */
  virtual PieceHandle getMissingPiece(int32_t index);

  /**
   * Returns the piece denoted by index.
   * No status of the piece is changed in this method.
   */
  virtual PieceHandle getPiece(int32_t index);

  /**
   * Tells that the download of the specfied piece completes.
   */
  virtual void completePiece(const PieceHandle& piece);

  /**
   * Tells that the download of the specified piece is canceled.
   */
  virtual void cancelPiece(const PieceHandle& piece);

  /**
   * Returns true if the specified piece is already downloaded.
   * Otherwise returns false.
   */
  virtual bool hasPiece(int32_t index);

  virtual bool isPieceUsed(int32_t index);

  virtual int64_t getTotalLength()
  {
    return _totalLength;
  }

  virtual int64_t getFilteredTotalLength()
  {
    return _totalLength;
  }

  virtual int64_t getCompletedLength()
  {
    // TODO we have to return actual completed length here?
    return _totalLength;
  }

  virtual int64_t getFilteredCompletedLength()
  {
    return getCompletedLength();
  }
  
  virtual void setFileFilter(const Strings& filePaths) {}

  virtual void setFileFilter(IntSequence seq) {}

  virtual void clearFileFilter() {}
  
  /**
   * Returns true if download has completed.
   * If file filter is enabled, then returns true if those files have
   * downloaded.
   */
  virtual bool downloadFinished()
  {
    return _downloadFinished;
  }

  /**
   * Returns true if all files have downloaded.
   * The file filter is ignored.
   */
  virtual bool allDownloadFinished()
  {
    return downloadFinished();
  }

  /**
   * Initializes DiskAdaptor.
   * TODO add better documentation here.
   */
  virtual void initStorage();

  virtual const unsigned char* getBitfield()
  {
    return 0;
  }

  virtual void setBitfield(const unsigned char* bitfield,
			   int32_t bitfieldLength) {}
  
  virtual int32_t getBitfieldLength()
  {
    return 0;
  }

  virtual bool isSelectiveDownloadingMode()
  {
    return false;
  }

  virtual void finishSelectiveDownloadingMode() {}

  virtual bool isEndGame()
  {
    return false;
  }

  virtual DiskAdaptorHandle getDiskAdaptor();
  
  virtual int32_t getPieceLength(int32_t index);

  /**
   * Adds piece index to advertise to other commands. They send have message
   * based on this information.
   */
  virtual void advertisePiece(int32_t cuid, int32_t index) {}

  /**
   * Returns piece index which is not advertised by the caller command and
   * newer than lastCheckTime.
   */
  virtual Integers getAdvertisedPieceIndexes(int32_t myCuid,
					     const Time& lastCheckTime)
  {
    return Integers();
  }

  /**
   * Removes have entry if specified seconds have elapsed since its
   * registration.
   */
  virtual void removeAdvertisedPiece(int32_t elapsed) {}

  /**
   * Sets all bits in bitfield to 1.
   */
  virtual void markAllPiecesDone();

  virtual void markPiecesDone(int64_t length)
  {
    // TODO not implemented yet
    abort();
  }

  virtual void markPieceMissing(int32_t index)
  {
    // TODO not implemented yet
    abort();
  }

  /**
   * Do nothing because loading in-flight piece is not supported for this
   * class.
   */
  virtual void addInFlightPiece(const Pieces& pieces) {}

  virtual int32_t countInFlightPiece()
  {
    return 0;
  }

  virtual Pieces getInFlightPieces();

  void setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory);
};

typedef SharedHandle<UnknownLengthPieceStorage> UnknownLengthPieceStorageHandle;

#endif // _D_UNKNOWN_LENGTH_PIECE_STORAGE_H_
