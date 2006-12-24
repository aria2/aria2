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
#include "BtContext.h"
#include "DiskAdaptor.h"
#include "BitfieldMan.h"
#include "Logger.h"
#include "Option.h"
#include "Piece.h"

#define END_GAME_PIECE_NUM 20

class HaveEntry {
private:
  int cuid;
  int index;
  Time registeredTime;
public:
  HaveEntry(int cuid, int index):
    cuid(cuid),
    index(index) {}

  int getCuid() const { return cuid; }

  int getIndex() const { return index; }

  const Time& getRegisteredTime() const { return registeredTime; }
};

typedef deque<HaveEntry> Haves;

class DefaultPieceStorage : public PieceStorage {
private:
  BtContextHandle btContext;
  BitfieldMan* bitfieldMan;
  DiskAdaptor* diskAdaptor;
  Pieces usedPieces;
  int endGamePieceNum;
  Logger* logger;
  const Option* option;
  Haves haves;

  int getMissingPieceIndex(const PeerHandle& peer);
  int getMissingFastPieceIndex(const PeerHandle& peer);
  PieceHandle checkOutPiece(int index);
  int deleteUsedPiecesByFillRate(int fillRate, int toDelete);
  void reduceUsedPieces(int delMax);
  void deleteUsedPiece(const PieceHandle& piece);
  PieceHandle findUsedPiece(int index) const;
public:
  DefaultPieceStorage(BtContextHandle btContext, const Option* option);
  virtual ~DefaultPieceStorage();

  virtual bool hasMissingPiece(const PeerHandle& peer);

  virtual PieceHandle getMissingPiece(const PeerHandle& peer);

  virtual PieceHandle getMissingFastPiece(const PeerHandle& peer);

  virtual PieceHandle getPiece(int index);

  virtual void completePiece(const PieceHandle& piece);

  virtual void cancelPiece(const PieceHandle& piece);

  virtual bool hasPiece(int index);

  virtual long long int getTotalLength();

  virtual long long int getFilteredTotalLength();

  virtual long long int getCompletedLength();

  virtual long long int getFilteredCompletedLength();

  virtual void initStorage();

  virtual void setFileFilter(const Strings& filePaths);

  virtual void setFileFilter(const Integers& fileIndexes);

  virtual void clearFileFilter();

  virtual bool downloadFinished();

  virtual void setBitfield(const unsigned char* bitfield,
			   int bitfieldLength);
  
  virtual int getBitfieldLength();

  virtual const unsigned char* getBitfield();

  void setEndGamePieceNum(int num) {
    endGamePieceNum = num;
  }

  int getEndGamePieceNum() const {
    return endGamePieceNum;
  }

  virtual bool isSelectiveDownloadingMode();

  virtual void finishSelectiveDownloadingMode();

  virtual bool isEndGame();
  
  virtual DiskAdaptor* getDiskAdaptor();

  virtual int getPieceLength(int index);

  virtual void advertisePiece(int cuid, int index);

  virtual Integers getAdvertisedPieceIndexes(int myCuid,
					     const Time& lastCheckTime);

  virtual void removeAdvertisedPiece(int elapsed);

  /**
   * This method is made private for test purpose only.
   */
  void addUsedPiece(const PieceHandle& piece);
};

#endif // _D_DEFAULT_PIECE_STORAGE_H_
