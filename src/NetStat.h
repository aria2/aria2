/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_NET_STAT_H
#define D_NET_STAT_H

#include "common.h"

#include "SpeedCalc.h"
#include "TransferStat.h"

namespace aria2 {

class NetStat {
public:
  enum STATUS {
    IDLE,
    ACTIVE,
  };

  NetStat();
  ~NetStat();

  // Don't allow copying
  NetStat(const NetStat&);
  NetStat& operator=(const NetStat&);

  /**
   * Returns current download speed in byte per sec.
   */
  int calculateDownloadSpeed();

  int calculateNewestDownloadSpeed(int seconds);

  int calculateAvgDownloadSpeed();

  int calculateUploadSpeed();

  int calculateNewestUploadSpeed(int seconds);

  int calculateAvgUploadSpeed();

  void updateDownload(size_t bytes);

  void updateUpload(size_t bytes);

  void updateUploadSpeed(size_t bytes);

  void updateUploadLength(size_t bytes);

  int getMaxDownloadSpeed() const;

  int getMaxUploadSpeed() const;

  int getAvgDownloadSpeed() const { return avgDownloadSpeed_; }

  int getAvgUploadSpeed() const { return avgUploadSpeed_; }

  void reset();

  void downloadStart();

  void downloadStop();

  const Timer& getDownloadStartTime() const { return downloadStartTime_; }

  STATUS getStatus() const { return status_; }

  uint64_t getSessionDownloadLength() const { return sessionDownloadLength_; }

  uint64_t getSessionUploadLength() const { return sessionUploadLength_; }

  void addSessionDownloadLength(uint64_t length)
  {
    sessionDownloadLength_ += length;
  }

  TransferStat toTransferStat();

private:
  SpeedCalc downloadSpeed_;
  SpeedCalc uploadSpeed_;
  Timer downloadStartTime_;
  STATUS status_;
  int avgDownloadSpeed_;
  int avgUploadSpeed_;
  int64_t sessionDownloadLength_;
  int64_t sessionUploadLength_;
};

} // namespace aria2

#endif // D_NET_STAT_H
