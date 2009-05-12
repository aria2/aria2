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
#ifndef _D_DOWNLOAD_RESULT_H_
#define _D_DOWNLOAD_RESULT_H_

#include "common.h"

#include <stdint.h>

#include <string>
#include <deque>

#include "SharedHandle.h"
#include "FileEntry.h"

namespace aria2 {

class DownloadResult
{
public:
  // RESULT is used as an exit status code.
  enum RESULT {
    FINISHED = 0,
    UNKNOWN_ERROR = 1,
    TIME_OUT = 2,
    RESOURCE_NOT_FOUND = 3,
    MAX_FILE_NOT_FOUND = 4,
    TOO_SLOW_DOWNLOAD_SPEED = 5,
    NETWORK_PROBLEM = 6,
    IN_PROGRESS = 7,
  };

  int32_t gid;
 
  std::deque<SharedHandle<FileEntry> > fileEntries;

  uint64_t totalLength;

  std::string uri;

  size_t numUri;

  uint64_t sessionDownloadLength;

  // milliseconds
  int64_t sessionTime;

  RESULT result;

  DownloadResult(int32_t gid,
		 const std::deque<SharedHandle<FileEntry> >& fileEntries,
		 uint64_t totalLength,
		 const std::string& uri,
		 size_t numUri,
		 uint64_t sessionDownloadLength,
		 int64_t sessionTime,
		 RESULT result):
    gid(gid),
    fileEntries(fileEntries),
    totalLength(totalLength),
    uri(uri),
    numUri(numUri),
    sessionDownloadLength(sessionDownloadLength),
    sessionTime(sessionTime),
    result(result) {}
};

typedef SharedHandle<DownloadResult> DownloadResultHandle;

} // namespace aria2

#endif // _D_DOWNLOAD_RESULT_H_
