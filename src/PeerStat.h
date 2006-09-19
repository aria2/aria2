/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_PEER_STAT_H_
#define _D_PEER_STAT_H_

#include "common.h"
#include "SpeedCalc.h"
#include "SharedHandle.h"

class PeerStat {
public:
  enum STATUS {
    IDLE,
    ACTIVE
  };
private:
  int cuid;
  SpeedCalc downloadSpeed;
  Time downloadStartTime;
  STATUS status;
public:

  PeerStat(int cuid):cuid(cuid), status(IDLE) {}

  ~PeerStat() {}

  /**
   * Returns current download speed in byte per sec.
   */
  int calculateDownloadSpeed() {
    return downloadSpeed.calculateSpeed();
  }

  void updateDownloadLength(int bytes) {
    downloadSpeed.update(bytes);
  }

  int getMaxSpeed() const {
    return downloadSpeed.getMaxSpeed();
  }

  void reset() {
    downloadSpeed.reset();
    downloadStartTime.reset();
  }

  void downloadStart() {
    reset();
    status = ACTIVE;
  }

  void downloadStop() {
    status = IDLE;
  }

  const Time& getDownloadStartTime() const {
    return downloadStartTime;
  }

  STATUS getStatus() const {
    return status;
  }

  int getCuid() const {
    return cuid;
  }
};

typedef SharedHandle<PeerStat> PeerStatHandle;

#endif // _D_PEER_STAT_H_
