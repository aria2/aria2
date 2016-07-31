/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "PeerStat.h"
#include "wallclock.h"

namespace aria2 {

PeerStat::PeerStat(cuid_t cuid, const std::string& hostname,
                   const ::std::string& protocol)
    : cuid_(cuid), hostname_(hostname), protocol_(protocol)
{
}

PeerStat::PeerStat(cuid_t cuid) : cuid_(cuid) {}

PeerStat::~PeerStat() = default;

/**
 * Returns current download speed in byte per sec.
 */
int PeerStat::calculateDownloadSpeed()
{
  return netStat_.calculateDownloadSpeed();
}

int PeerStat::calculateAvgDownloadSpeed()
{
  return netStat_.calculateAvgDownloadSpeed();
}

int PeerStat::calculateUploadSpeed() { return netStat_.calculateUploadSpeed(); }

int PeerStat::calculateAvgUploadSpeed()
{
  return netStat_.calculateAvgUploadSpeed();
}

void PeerStat::updateDownload(size_t bytes) { netStat_.updateDownload(bytes); }

void PeerStat::updateUpload(size_t bytes) { netStat_.updateUpload(bytes); }

int PeerStat::getMaxDownloadSpeed() const
{
  return netStat_.getMaxDownloadSpeed();
}

int PeerStat::getMaxUploadSpeed() const { return netStat_.getMaxUploadSpeed(); }

int PeerStat::getAvgDownloadSpeed() const
{
  return netStat_.getAvgDownloadSpeed();
}

int PeerStat::getAvgUploadSpeed() const { return netStat_.getAvgUploadSpeed(); }

uint64_t PeerStat::getSessionDownloadLength() const
{
  return netStat_.getSessionDownloadLength();
}

uint64_t PeerStat::getSessionUploadLength() const
{
  return netStat_.getSessionUploadLength();
}

void PeerStat::addSessionDownloadLength(uint64_t length)
{
  netStat_.addSessionDownloadLength(length);
}

const Timer& PeerStat::getDownloadStartTime() const
{
  return netStat_.getDownloadStartTime();
}

NetStat::STATUS PeerStat::getStatus() const { return netStat_.getStatus(); }

void PeerStat::reset() { netStat_.reset(); }

void PeerStat::downloadStart() { netStat_.downloadStart(); }

void PeerStat::downloadStop() { netStat_.downloadStop(); }

TransferStat PeerStat::toTransferStat() { return netStat_.toTransferStat(); }

} // namespace aria2
