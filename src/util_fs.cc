/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2016 Tatsuhiro Tsujikawa
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

#include "a2io.h"
#include "util.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else // _WIN32
#include <sys/statvfs.h>
#endif

namespace aria2 {
namespace util {
namespace filesystem {

space_info space(const char* path, std::error_code& ec)
{
  space_info rv{static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1),
                static_cast<uintmax_t>(-1)};
  if (!path || !*path) {
    path = ".";
  }
#ifdef _WIN32
  ULARGE_INTEGER sp_avail, sp_free, sp_cap;
  auto wpath = utf8ToWChar(path);
  if (GetDiskFreeSpaceExW(wpath.c_str(), &sp_avail, &sp_cap, &sp_free)) {
    rv.capacity = static_cast<uintmax_t>(sp_cap.QuadPart);
    rv.available = static_cast<uintmax_t>(sp_avail.QuadPart);
    rv.free = static_cast<uintmax_t>(sp_free.QuadPart);
    ec.clear();
  }
  else {
    ec.assign(GetLastError(), std::system_category());
  }
#else  // _WIN32
  struct statvfs st;
  if (!statvfs(path, &st)) {
    rv.capacity = static_cast<uintmax_t>(st.f_blocks) * st.f_frsize;
    rv.free = static_cast<uintmax_t>(st.f_bfree) * st.f_frsize;
    rv.available = static_cast<uintmax_t>(st.f_bavail) * st.f_frsize;
    ec.clear();
  }
  else {
    ec.assign(errno, std::system_category());
  }
#endif // _WIN32

  return rv;
}

space_info space_downwards(const char* path, std::error_code& ec)
{
  auto rv = space(path, ec);
  if (!ec) {
    return rv;
  }
  std::string spath(path);
  for (;;) {
    if (spath.empty()) {
      break;
    }
#if _WIN32
    if (spath == "\\\\") {
      // raw UNC prefix
      break;
    }
#endif
    auto pos = spath.find_last_of("/\\");
    if (pos == std::string::npos) {
      spath = "";
    }
    else {
      spath = spath.substr(0, pos);
    }
    rv = space(spath.c_str(), ec);
    if (!ec) {
      break;
    }
  }
  return rv;
}

} // namespace filesystem
} // namespace util
} // namespace aria2
