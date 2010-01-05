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
#include "TransferStat.h"

namespace aria2 {

TransferStat operator+(const TransferStat& a, const TransferStat& b)
{
  TransferStat c;
  c.downloadSpeed = a.downloadSpeed+b.downloadSpeed;
  c.uploadSpeed = a.uploadSpeed+b.uploadSpeed;
  c.sessionDownloadLength = a.sessionDownloadLength+b.sessionDownloadLength;
  c.sessionUploadLength = a.sessionUploadLength+b.sessionUploadLength;
  return c;
}

TransferStat operator-(const TransferStat& a, const TransferStat& b)
{
  TransferStat c;
  if(a.downloadSpeed > b.downloadSpeed) {
    c.downloadSpeed = a.downloadSpeed-b.downloadSpeed;
  }
  if(a.uploadSpeed > b.uploadSpeed) {
    c.uploadSpeed = a.uploadSpeed-b.uploadSpeed;
  }
  if(a.sessionDownloadLength > b.sessionDownloadLength) {
    c.sessionDownloadLength = a.sessionDownloadLength-b.sessionDownloadLength;
  }
  if(a.sessionUploadLength > b.sessionUploadLength) {
    c.sessionUploadLength = a.sessionUploadLength-b.sessionUploadLength;
  }
  return c;
}

TransferStat& TransferStat::operator+=(const TransferStat& stat)
{
  downloadSpeed += stat.downloadSpeed;
  uploadSpeed += stat.uploadSpeed;
  sessionDownloadLength += stat.sessionDownloadLength;
  sessionUploadLength += stat.sessionUploadLength;
  return *this;
}

TransferStat& TransferStat::operator-=(const TransferStat& stat)
{
  if(downloadSpeed > stat.downloadSpeed) {
    downloadSpeed -= stat.downloadSpeed;
  } else {
    downloadSpeed = 0;
  }
  if(uploadSpeed > stat.uploadSpeed) {
    uploadSpeed -= stat.uploadSpeed;
  } else {
    uploadSpeed = 0;
  }
  if(sessionDownloadLength > stat.sessionDownloadLength) {
    sessionDownloadLength -= stat.sessionDownloadLength;
  } else {
    sessionDownloadLength = 0;
  }
  if(sessionUploadLength > stat.sessionUploadLength) {
    sessionUploadLength -= stat.sessionUploadLength;
  } else {
    sessionUploadLength = 0;
  }
  return *this;
}

} // namespace aria2
