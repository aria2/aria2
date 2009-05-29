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
#include "DownloadContext.h"

namespace aria2 {

DownloadContext::DownloadContext():
  _dir("."),
  _downloadStartTime(0),
  _downloadStopTime(_downloadStartTime) {}

DownloadContext::~DownloadContext() {}

void DownloadContext::setDir(const std::string& dir)
{
  _dir = dir;
}

SharedHandle<Signature> DownloadContext::getSignature() const
{
  return _signature;
}

void DownloadContext::setSignature(const SharedHandle<Signature>& signature)
{
  _signature = signature;
}

void DownloadContext::resetDownloadStartTime()
{
  _downloadStartTime.reset();
}

void DownloadContext::resetDownloadStopTime()
{
  _downloadStopTime.reset();
}

int64_t DownloadContext::calculateSessionTime() const
{
  if(_downloadStopTime.isNewer(_downloadStartTime)) {
    return
      _downloadStopTime.getTimeInMillis()-_downloadStartTime.getTimeInMillis();
  } else {
    return 0;
  }
}

} // namespace aria2
