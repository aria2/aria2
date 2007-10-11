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
#ifndef _D_DEFAULT_PIECE_STORAGE_H_
#define _D_DEFAULT_PIECE_STORAGE_H_

#include "PieceStorage.h"

class DownloadContext;
extern typedef SharedHandle<DownloadContext> DownloadContextHandle;
class BitfieldMan;
class Logger;
class Option;
extern typedef deque<PieceHandle> Pieces;
class DiskWriterFactory;
extern typedef SharedHandle<DiskWriterFactory> DiskWriterFactoryHandle;

#define END_GAME_PIECE_NUM 20

class HaveEntry {
private:
  int32_t cuid;
  int32_t index;
  Time registeredTime;
public:
  HaveEntry(int32_t cuid, int32_t index):
    cuid(cuid),
    index(index) {}

  int32_t getCuid() const { return cuid; }

  int32_t getIndex() const { return index; }

  const Time& getRegisteredTime() const { return registeredTime; }
};

typedef deque<HaveEntry> Haves;

class DefaultPieceStorage : public PieceStorage {
private:
  DownloadContextHandle downloadContext;
  BitfieldMan* bitfieldMan;
  DiskAdaptorHandle diskAdaptor;
  DiskWriterFactoryHandle _diskWriterFactory;
  Pieces usedPieces;
  int32_t endGamePieceNum;
  Logger* logger;
  const Option* option;
  Haves haves;

  int32_t getMissingPieceIndex(const PeerHandle& peer);
  int32_t getMissingFastPieceIndex(const PeerHandle& peer);
  PieceHandle checkOutPiece(int32_t index);
  int32_t deleteUsedPiecesByFillRate(int32_t fillRate, int32_t toDelete);
  void reduceUsedPieces(int32_t delMax);
  void deleteUsedPiece(const PieceHandle& piece);
  PieceHandle findUsedPiece(int32_t index) const;
public:
  DefaultPieceStorage(const DownloadContextHandle& downloadContext, const Option* option);
  virtual ~DefaultPieceStorage();

  virtual bool hasMissingPiece(const PeerHandle& peer);

  virtual PieceHandle getMissingPiece(const PeerHandle& peer);

  virtual PieceHandle getMissingFastPiece(const PeerHandle& peer);

  virtual PieceHandle getMissingPiece();

  virtual PieceHandle getMissingPiece(int32_t index);

  virtual PieceHandle getPiece(int32_t index);

  virtual void completePiece(const PieceHandle& piece);

  virtual void cancelPiece(const PieceHandle& piece);

  virtual bool hasPiece(int32_t index);

  virtual bool isPieceUsed(int32_t index);

  virtual int64_t getTotalLength();

  virtual int64_t getFilteredTotalLength();

  virtual int64_t getCompletedLength();

  virtual int64_t getFilteredCompletedLength();

  virtual void initStorage();

  virtual void setFileFilter(const Strings& filePaths);

  virtual void setFileFilter(const Integers& fileIndexes);

  virtual void clearFileFilter();

  virtual bool downloadFinished();

  virtual bool allDownloadFinished();

  virtual void setBitfield(const unsigned char* bitfield,
			   int32_t bitfieldLength);
  
  virtual int32_t getBitfieldLength();

  virtual const unsigned char* getBitfield();

  void setEndGamePieceNum(int32_t num) {
    endGamePieceNum = num;
  }

  int32_t getEndGamePieceNum() const {
    return endGamePieceNum;
  }

  virtual bool isSelectiveDownloadingMode();

  virtual void finishSelectiveDownloadingMode();

  virtual bool isEndGame();
  
  virtual DiskAdaptorHandle getDiskAdaptor();

  virtual int32_t getPieceLength(int32_t index);

  virtual void advertisePiece(int32_t cuid, int32_t index);

  virtual Integers getAdvertisedPieceIndexes(int32_t myCuid,
					     const Time& lastCheckTime);

  virtual void removeAdvertisedPiece(int32_t elapsed);

  virtual void markAllPiecesDone();

  virtual void markPiecesDone(int64_t length);

  virtual void markPieceMissing(int32_t index);

  virtual void addInFlightPiece(const Pieces& pieces);

  virtual int32_t countInFlightPiece();

  virtual Pieces getInFlightPieces();

  /**
   * This method is made private for test purpose only.
   */
  void addUsedPiece(const PieceHandle& piece);

  void setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory);
};

typedef SharedHandle<DefaultPieceStorage> DefaultPieceStorageHandle;

#endif // _D_DEFAULT_PIECE_STORAGE_H_
