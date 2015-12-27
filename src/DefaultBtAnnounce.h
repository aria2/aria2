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
  DownloadContext* downloadContext_;
  int trackers_;
  Timer prevAnnounceTimer_;
  std::chrono::seconds interval_;
  std::chrono::seconds minInterval_;
  std::chrono::seconds userDefinedInterval_;
  int complete_;
  int incomplete_;
  AnnounceList announceList_;
  std::string trackerId_;
  const Option* option_;
  Randomizer* randomizer_;
  std::shared_ptr<BtRuntime> btRuntime_;
  std::shared_ptr<PieceStorage> pieceStorage_;
  std::shared_ptr<PeerStorage> peerStorage_;
  uint16_t tcpPort_;

  bool adjustAnnounceList();

public:
  DefaultBtAnnounce(DownloadContext* downloadContext, const Option* option);

  virtual ~DefaultBtAnnounce();

  void setBtRuntime(const std::shared_ptr<BtRuntime>& btRuntime);

  const std::shared_ptr<BtRuntime>& getBtRuntime() const { return btRuntime_; }

  void setPieceStorage(const std::shared_ptr<PieceStorage>& pieceStorage);

  const std::shared_ptr<PieceStorage>& getPieceStorage() const
  {
    return pieceStorage_;
  }

  void setPeerStorage(const std::shared_ptr<PeerStorage>& peerStorage);

  const std::shared_ptr<PeerStorage>& getPeerStorage() const
  {
    return peerStorage_;
  }

  bool isDefaultAnnounceReady();

  bool isStoppedAnnounceReady();

  bool isCompletedAnnounceReady();

  virtual bool isAnnounceReady() CXX11_OVERRIDE;

  virtual std::string getAnnounceUrl() CXX11_OVERRIDE;

  virtual std::shared_ptr<UDPTrackerRequest>
  createUDPTrackerRequest(const std::string& remoteAddr, uint16_t remotePort,
                          uint16_t localPort) CXX11_OVERRIDE;

  virtual void announceStart() CXX11_OVERRIDE;

  virtual void announceSuccess() CXX11_OVERRIDE;

  virtual void announceFailure() CXX11_OVERRIDE;

  virtual bool isAllAnnounceFailed() CXX11_OVERRIDE;

  virtual void resetAnnounce() CXX11_OVERRIDE;

  virtual void
  processAnnounceResponse(const unsigned char* trackerResponse,
                          size_t trackerResponseLength) CXX11_OVERRIDE;

  virtual void processUDPTrackerResponse(
      const std::shared_ptr<UDPTrackerRequest>& req) CXX11_OVERRIDE;

  virtual bool noMoreAnnounce() CXX11_OVERRIDE;

  virtual void shuffleAnnounce() CXX11_OVERRIDE;

  virtual void
  overrideMinInterval(std::chrono::seconds interval) CXX11_OVERRIDE;

  virtual void setTcpPort(uint16_t port) CXX11_OVERRIDE { tcpPort_ = port; }

  void setRandomizer(Randomizer* randomizer);

  const std::chrono::seconds& getInterval() const { return interval_; }

  const std::chrono::seconds& getMinInterval() const { return minInterval_; }

  int getComplete() const { return complete_; }

  int getIncomplete() const { return incomplete_; }

  const std::string& getTrackerID() const { return trackerId_; }

  void setUserDefinedInterval(std::chrono::seconds interval)
  {
    userDefinedInterval_ = std::move(interval);
  }
};

} // namespace aria2

#endif // D_DEFAULT_BT_ANNOUNCE_H
