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
#ifndef _D_TRANSFER_STAT_H_
#define _D_TRANSFER_STAT_H_

#include "common.h"
#include <stdint.h>

namespace aria2 {

class TransferStat {
public:
  unsigned int downloadSpeed;
  unsigned int uploadSpeed;
  uint64_t sessionDownloadLength;
  uint64_t sessionUploadLength;
  uint64_t allTimeUploadLength;

  void copy(const TransferStat& stat)
  {
    downloadSpeed = stat.downloadSpeed;
    uploadSpeed = stat.uploadSpeed;
    sessionDownloadLength = stat.sessionDownloadLength;
    sessionUploadLength = stat.sessionUploadLength;
    allTimeUploadLength = stat.allTimeUploadLength;
  }
public:
  TransferStat():downloadSpeed(0), uploadSpeed(0),
    sessionDownloadLength(0), sessionUploadLength(0),
    allTimeUploadLength(0) {}

  TransferStat(const TransferStat& stat)
  {
    copy(stat);
  }

  TransferStat& operator=(const TransferStat& stat)
  {
    if(this != &stat) {
      copy(stat);
    }
    return *this;
  }

  friend TransferStat operator+(const TransferStat& a, const TransferStat& b);

  friend TransferStat operator-(const TransferStat& a, const TransferStat& b);

  TransferStat& operator+=(const TransferStat& stat);

  TransferStat& operator-=(const TransferStat& stat);

  unsigned int getDownloadSpeed() const {
    return downloadSpeed;
  }

  void setDownloadSpeed(unsigned int s) { downloadSpeed = s; }

  unsigned int getUploadSpeed() const {
    return uploadSpeed;
  }

  void setUploadSpeed(unsigned int s) { uploadSpeed = s; }

  /**
   * Returns the number of bytes downloaded since the program started.
   * This is not the total number of bytes downloaded.
   */
  uint64_t getSessionDownloadLength() const {
    return sessionDownloadLength;
  }

  void setSessionDownloadLength(uint64_t s) { sessionDownloadLength = s; }

  /**
   * Returns the number of bytes uploaded since the program started.
   * This is not the total number of bytes uploaded.
   */
  uint64_t getSessionUploadLength() const {
    return sessionUploadLength;
  }

  void setSessionUploadLength(uint64_t s) { sessionUploadLength = s; }

  void setAllTimeUploadLength(uint64_t s)
  {
    allTimeUploadLength = s;
  }

  uint64_t getAllTimeUploadLength() const
  {
    return allTimeUploadLength;
  }
};

TransferStat operator+(const TransferStat& a, const TransferStat& b);

TransferStat operator-(const TransferStat& a, const TransferStat& b);

} // namespace aria2

#endif // _D_TRANSFER_STAT_H_
