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
#ifndef _D_DEFAULT_BT_ANNOUNCE_H_
#define _D_DEFAULT_BT_ANNOUNCE_H_

#include "BtAnnounce.h"
#include "BtContext.h"
#include "TimeA2.h"
#include "AnnounceList.h"
#include "Option.h"
#include "Logger.h"
#include "BtRuntime.h"
#include "PieceStorage.h"
#include "PeerStorage.h"

class Randomizer;
extern typedef SharedHandle<Randomizer> RandomizerHandle;

#define DEFAULT_ANNOUNCE_INTERVAL 1800

class DefaultBtAnnounce : public BtAnnounce {
private:
  BtContextHandle btContext;
  int32_t trackers;
  Time prevAnnounceTime;
  int32_t interval;
  int32_t minInterval;
  int32_t complete;
  int32_t incomplete;
  AnnounceList announceList;
  string trackerId;
  string key;
  int32_t trackerNumTry;
  const Option* option;
  Logger* logger;
  RandomizerHandle _randomizer;
  BtRuntimeHandle btRuntime;
  PieceStorageHandle pieceStorage;
  PeerStorageHandle peerStorage;
public:
  DefaultBtAnnounce(BtContextHandle btContext, const Option* option);
  virtual ~DefaultBtAnnounce();

  void setBtRuntime(const BtRuntimeHandle& btRuntime) {
    this->btRuntime = btRuntime;
  }
  BtRuntimeHandle getBtRuntime() const { return btRuntime; }

  void setPieceStorage(const PieceStorageHandle& pieceStorage) {
    this->pieceStorage = pieceStorage;
  }
  PieceStorageHandle getPieceStorage() const { return pieceStorage; }

  void setPeerStorage(const PeerStorageHandle& peerStorage) {
    this->peerStorage = peerStorage;
  }
  PeerStorageHandle getPeerStorage() const { return peerStorage; }

  bool isDefaultAnnounceReady();

  bool isStoppedAnnounceReady();

  bool isCompletedAnnounceReady();

  virtual bool isAnnounceReady();

  virtual string getAnnounceUrl();

  virtual void announceStart();

  virtual void announceSuccess();

  virtual void announceFailure();

  virtual bool isAllAnnounceFailed();

  virtual void resetAnnounce();

  virtual void processAnnounceResponse(const char* trackerResponse,
				       size_t trackerResponseLength);

  virtual bool noMoreAnnounce();

  virtual void shuffleAnnounce();

  void generateKey();

  void setRandomizer(const RandomizerHandle& randomizer);
};

#endif // _D_DEFAULT_BT_ANNOUNCE_H_
