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
#ifndef _D_PEER_STAT_H_
#define _D_PEER_STAT_H_

#include "common.h"

#include <string>

#include "SpeedCalc.h"
#include "SharedHandle.h"
#include "Command.h"

namespace aria2 {

class PeerStat {
public:
  enum STATUS {
    IDLE,
    ACTIVE,
  };
private:
  cuid_t cuid;
  std::string _hostname;
  std::string _protocol;
  SpeedCalc downloadSpeed;
  SpeedCalc uploadSpeed;
  Time downloadStartTime;
  PeerStat::STATUS status;
  unsigned int _avgDownloadSpeed;
  unsigned int _avgUploadSpeed;
  uint64_t _sessionDownloadLength;
  uint64_t _sessionUploadLength;
public:

  PeerStat(cuid_t cuid, const std::string& hostname,
	   const::std::string& protocol):
    cuid(cuid),
    _hostname(hostname),
    _protocol(protocol),
    status(PeerStat::IDLE),
    _avgDownloadSpeed(0),
    _avgUploadSpeed(0),
    _sessionDownloadLength(0),
    _sessionUploadLength(0) {}

  PeerStat(cuid_t cuid = 0):cuid(cuid), status(PeerStat::IDLE),
			    _avgDownloadSpeed(0),
			    _avgUploadSpeed(0),
			    _sessionDownloadLength(0),
			    _sessionUploadLength(0) {}

  ~PeerStat() {}

  /**
   * Returns current download speed in byte per sec.
   */
  unsigned int calculateDownloadSpeed() {
    return downloadSpeed.calculateSpeed();
  }

  unsigned int calculateDownloadSpeed(const struct timeval& now) {
    return downloadSpeed.calculateSpeed(now);
  }

  unsigned int calculateAvgDownloadSpeed() {
    _avgDownloadSpeed = downloadSpeed.calculateAvgSpeed();
    return _avgDownloadSpeed;
  }

  unsigned int calculateUploadSpeed() {
    return uploadSpeed.calculateSpeed();
  }

  unsigned int calculateUploadSpeed(const struct timeval& now) {
    return uploadSpeed.calculateSpeed(now);
  }

  unsigned int calculateAvgUploadSpeed() {
    _avgUploadSpeed = uploadSpeed.calculateAvgSpeed();
    return _avgUploadSpeed;
  }

  void updateDownloadLength(size_t bytes) {
    downloadSpeed.update(bytes);
    _sessionDownloadLength += bytes;
  }

  void updateUploadLength(size_t bytes) {
    uploadSpeed.update(bytes);
    _sessionUploadLength += bytes;
  }

  unsigned int getMaxDownloadSpeed() const {
    return downloadSpeed.getMaxSpeed();
  }

  unsigned int getMaxUploadSpeed() const {
    return uploadSpeed.getMaxSpeed();
  }

  unsigned int getAvgDownloadSpeed() const {
    return _avgDownloadSpeed;
  }

  unsigned int getAvgUploadSpeed() const {
    return _avgUploadSpeed;
  }

  void reset() {
    downloadSpeed.reset();
    uploadSpeed.reset();
    downloadStartTime.reset();
    status = PeerStat::IDLE;
  }

  void downloadStart() {
    reset();
    status = PeerStat::ACTIVE;
  }

  void downloadStop() {
    calculateAvgDownloadSpeed();
    calculateAvgUploadSpeed();
    status = PeerStat::IDLE;
  }

  const Time& getDownloadStartTime() const {
    return downloadStartTime;
  }

  PeerStat::STATUS getStatus() const {
    return status;
  }

  cuid_t getCuid() const {
    return cuid;
  }

  const std::string& getHostname() const
  {
    return _hostname;
  }

  const std::string& getProtocol() const
  {
    return _protocol;
  }

  uint64_t getSessionDownloadLength() const
  {
    return _sessionDownloadLength;
  }

  uint64_t getSessionUploadLength() const
  {
    return _sessionUploadLength;
  }
};

typedef SharedHandle<PeerStat> PeerStatHandle;

} // namespace aria2

#endif // _D_PEER_STAT_H_
