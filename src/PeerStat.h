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
#include "SpeedCalc.h"
#include "SharedHandle.h"
#include <sys/time.h>

class PeerStat {
public:
  enum STATUS {
    IDLE,
    ACTIVE,
    REQUEST_IDLE,
  };
private:
  int32_t cuid;
  SpeedCalc downloadSpeed;
  SpeedCalc uploadSpeed;
  Time downloadStartTime;
  PeerStat::STATUS status;
public:

  PeerStat(int32_t cuid = 0):cuid(cuid), status(PeerStat::IDLE) {}

  ~PeerStat() {}

  /**
   * Returns current download speed in byte per sec.
   */
  int32_t calculateDownloadSpeed() {
    return downloadSpeed.calculateSpeed();
  }

  int32_t calculateDownloadSpeed(const struct timeval& now) {
    return downloadSpeed.calculateSpeed(now);
  }

  int32_t calculateUploadSpeed() {
    return uploadSpeed.calculateSpeed();
  }

  int32_t calculateUploadSpeed(const struct timeval& now) {
    return uploadSpeed.calculateSpeed(now);
  }

  void updateDownloadLength(int32_t bytes) {
    downloadSpeed.update(bytes);
  }

  void updateUploadLength(int32_t bytes) {
    uploadSpeed.update(bytes);
  }

  int32_t getMaxDownloadSpeed() const {
    return downloadSpeed.getMaxSpeed();
  }

  int32_t getMaxUploadSpeed() const {
    return uploadSpeed.getMaxSpeed();
  }

  int32_t getAvgDownloadSpeed() const {
    return downloadSpeed.getAvgSpeed();
  }

  int32_t getAvgUploadSpeed() const {
    return uploadSpeed.getAvgSpeed();
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
    status = PeerStat::IDLE;
  }

  void requestIdle() {
    status = PeerStat::REQUEST_IDLE;
  }

  const Time& getDownloadStartTime() const {
    return downloadStartTime;
  }

  PeerStat::STATUS getStatus() const {
    return status;
  }

  int32_t getCuid() const {
    return cuid;
  }
};

typedef SharedHandle<PeerStat> PeerStatHandle;

#endif // _D_PEER_STAT_H_
