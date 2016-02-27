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
#include "NetStat.h"
#include "wallclock.h"

namespace aria2 {

NetStat::NetStat()
    : status_(NetStat::IDLE),
      avgDownloadSpeed_(0),
      avgUploadSpeed_(0),
      sessionDownloadLength_(0),
      sessionUploadLength_(0)
{
}

NetStat::~NetStat() {}

/**
 * Returns current download speed in byte per sec.
 */
int NetStat::calculateDownloadSpeed()
{
  return downloadSpeed_.calculateSpeed();
}

int NetStat::calculateAvgDownloadSpeed()
{
  return avgDownloadSpeed_ = downloadSpeed_.calculateAvgSpeed();
}

int NetStat::calculateUploadSpeed() { return uploadSpeed_.calculateSpeed(); }

int NetStat::calculateAvgUploadSpeed()
{
  return avgUploadSpeed_ = uploadSpeed_.calculateAvgSpeed();
}

void NetStat::updateDownload(size_t bytes)
{
  downloadSpeed_.update(bytes);
  sessionDownloadLength_ += bytes;
}

void NetStat::updateUpload(size_t bytes)
{
  uploadSpeed_.update(bytes);
  sessionUploadLength_ += bytes;
}

void NetStat::updateUploadSpeed(size_t bytes) { uploadSpeed_.update(bytes); }

void NetStat::updateUploadLength(size_t bytes)
{
  sessionUploadLength_ += bytes;
}

int NetStat::getMaxDownloadSpeed() const
{
  return downloadSpeed_.getMaxSpeed();
}

int NetStat::getMaxUploadSpeed() const { return uploadSpeed_.getMaxSpeed(); }

void NetStat::reset()
{
  downloadSpeed_.reset();
  uploadSpeed_.reset();
  downloadStartTime_ = global::wallclock();
  status_ = IDLE;
}

void NetStat::downloadStart()
{
  reset();
  status_ = ACTIVE;
}

void NetStat::downloadStop()
{
  calculateAvgDownloadSpeed();
  calculateAvgUploadSpeed();
  status_ = IDLE;
}

TransferStat NetStat::toTransferStat()
{
  TransferStat stat;
  stat.downloadSpeed = calculateDownloadSpeed();
  stat.uploadSpeed = calculateUploadSpeed();
  stat.sessionDownloadLength = getSessionDownloadLength();
  stat.sessionUploadLength = getSessionUploadLength();
  return stat;
}

} // namespace aria2
