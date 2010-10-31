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
#ifndef D_PEER_STAT_H
#define D_PEER_STAT_H

#include "common.h"

#include <string>

#include "SpeedCalc.h"
#include "SharedHandle.h"
#include "Command.h"
#include "wallclock.h"

namespace aria2 {

class PeerStat {
public:
  enum STATUS {
    IDLE,
    ACTIVE,
  };
private:
  cuid_t cuid_;
  std::string hostname_;
  std::string protocol_;
  SpeedCalc downloadSpeed_;
  SpeedCalc uploadSpeed_;
  Timer downloadStartTime_;
  PeerStat::STATUS status_;
  unsigned int avgDownloadSpeed_;
  unsigned int avgUploadSpeed_;
  uint64_t sessionDownloadLength_;
  uint64_t sessionUploadLength_;
public:

  PeerStat(cuid_t cuid, const std::string& hostname,
           const::std::string& protocol):
    cuid_(cuid),
    hostname_(hostname),
    protocol_(protocol),
    downloadStartTime_(global::wallclock),
    status_(PeerStat::IDLE),
    avgDownloadSpeed_(0),
    avgUploadSpeed_(0),
    sessionDownloadLength_(0),
    sessionUploadLength_(0) {}

  PeerStat(cuid_t cuid = 0):
    cuid_(cuid),
    status_(PeerStat::IDLE),
    avgDownloadSpeed_(0),
    avgUploadSpeed_(0),
    sessionDownloadLength_(0),
    sessionUploadLength_(0) {}

  /**
   * Returns current download speed in byte per sec.
   */
  unsigned int calculateDownloadSpeed() {
    return downloadSpeed_.calculateSpeed();
  }

  unsigned int calculateAvgDownloadSpeed() {
    avgDownloadSpeed_ = downloadSpeed_.calculateAvgSpeed();
    return avgDownloadSpeed_;
  }

  unsigned int calculateUploadSpeed() {
    return uploadSpeed_.calculateSpeed();
  }

  unsigned int calculateAvgUploadSpeed() {
    avgUploadSpeed_ = uploadSpeed_.calculateAvgSpeed();
    return avgUploadSpeed_;
  }

  void updateDownloadLength(size_t bytes) {
    downloadSpeed_.update(bytes);
    sessionDownloadLength_ += bytes;
  }

  void updateUploadLength(size_t bytes) {
    uploadSpeed_.update(bytes);
    sessionUploadLength_ += bytes;
  }

  unsigned int getMaxDownloadSpeed() const {
    return downloadSpeed_.getMaxSpeed();
  }

  unsigned int getMaxUploadSpeed() const {
    return uploadSpeed_.getMaxSpeed();
  }

  unsigned int getAvgDownloadSpeed() const {
    return avgDownloadSpeed_;
  }

  unsigned int getAvgUploadSpeed() const {
    return avgUploadSpeed_;
  }

  void reset() {
    downloadSpeed_.reset();
    uploadSpeed_.reset();
    downloadStartTime_ = global::wallclock;
    status_ = PeerStat::IDLE;
  }

  void downloadStart() {
    reset();
    status_ = PeerStat::ACTIVE;
  }

  void downloadStop() {
    calculateAvgDownloadSpeed();
    calculateAvgUploadSpeed();
    status_ = PeerStat::IDLE;
  }

  const Timer& getDownloadStartTime() const {
    return downloadStartTime_;
  }

  PeerStat::STATUS getStatus() const {
    return status_;
  }

  cuid_t getCuid() const {
    return cuid_;
  }

  const std::string& getHostname() const
  {
    return hostname_;
  }

  const std::string& getProtocol() const
  {
    return protocol_;
  }

  uint64_t getSessionDownloadLength() const
  {
    return sessionDownloadLength_;
  }

  uint64_t getSessionUploadLength() const
  {
    return sessionUploadLength_;
  }

  void addSessionDownloadLength(uint64_t length)
  {
    sessionDownloadLength_ += length;
  }
};

typedef SharedHandle<PeerStat> PeerStatHandle;

} // namespace aria2

#endif // D_PEER_STAT_H
