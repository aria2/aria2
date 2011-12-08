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
#ifndef D_DEFAULT_BT_ANNOUNCE_H
#define D_DEFAULT_BT_ANNOUNCE_H

#include "BtAnnounce.h"
#include "TimerA2.h"
#include "AnnounceList.h"

namespace aria2 {

class DownloadContext;
class Option;
class BtRuntime;
class PieceStorage;
class PeerStorage;
class Randomizer;

class DefaultBtAnnounce : public BtAnnounce {
private:
  SharedHandle<DownloadContext> downloadContext_;
  int trackers_;
  Timer prevAnnounceTimer_;
  time_t interval_;
  time_t minInterval_;
  time_t userDefinedInterval_;
  int complete_;
  int incomplete_;
  AnnounceList announceList_;
  std::string trackerId_;
  const Option* option_;
  SharedHandle<Randomizer> randomizer_;
  SharedHandle<BtRuntime> btRuntime_;
  SharedHandle<PieceStorage> pieceStorage_;
  SharedHandle<PeerStorage> peerStorage_;
  uint16_t tcpPort_;
public:
  DefaultBtAnnounce(const SharedHandle<DownloadContext>& downloadContext,
                    const Option* option);

  virtual ~DefaultBtAnnounce();

  void setBtRuntime(const SharedHandle<BtRuntime>& btRuntime);

  const SharedHandle<BtRuntime>& getBtRuntime() const
  {
    return btRuntime_;
  }

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  const SharedHandle<PieceStorage>& getPieceStorage() const
  {
    return pieceStorage_;
  }

  void setPeerStorage(const SharedHandle<PeerStorage>& peerStorage);

  const SharedHandle<PeerStorage>& getPeerStorage() const
  {
    return peerStorage_;
  }

  bool isDefaultAnnounceReady();

  bool isStoppedAnnounceReady();

  bool isCompletedAnnounceReady();

  virtual bool isAnnounceReady();

  virtual std::string getAnnounceUrl();

  virtual void announceStart();

  virtual void announceSuccess();

  virtual void announceFailure();

  virtual bool isAllAnnounceFailed();

  virtual void resetAnnounce();

  virtual void processAnnounceResponse(const unsigned char* trackerResponse,
                                       size_t trackerResponseLength);

  virtual bool noMoreAnnounce();

  virtual void shuffleAnnounce();

  virtual void overrideMinInterval(time_t interval);

  virtual void setTcpPort(uint16_t port)
  {
    tcpPort_ = port;
  }

  void setRandomizer(const SharedHandle<Randomizer>& randomizer);

  time_t getInterval() const
  {
    return interval_;
  }

  time_t getMinInterval() const
  {
    return minInterval_;
  }

  int getComplete() const
  {
    return complete_;
  }

  int getIncomplete() const
  {
    return incomplete_;
  }

  const std::string& getTrackerID() const
  {
    return trackerId_;
  }

  void setUserDefinedInterval(time_t interval)
  {
    userDefinedInterval_ = interval;
  }
};

} // namespace aria2

#endif // D_DEFAULT_BT_ANNOUNCE_H
