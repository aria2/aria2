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
#include "File.h"

#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_UTIME_H
#  include <utime.h>
#endif // HAVE_UTIME_H
#include <unistd.h>

#include <vector>
#include <cstring>
#include <cstdio>

#include "util.h"
#include "A2STR.h"
#include "array_fun.h"
#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

File::File(const std::string& name) : name_(name) {}

File::File(const File& c) = default;

File::~File() = default;

File& File::operator=(const File& c)
{
  if (this != &c) {
    name_ = c.name_;
  }
  return *this;
}

int File::fillStat(a2_struct_stat& fstat)
{
  return a2stat(utf8ToWChar(name_).c_str(), &fstat);
}

bool File::exists()
{
  a2_struct_stat fstat;
  return fillStat(fstat) == 0;
}

bool File::exists(std::string& err)
{
  a2_struct_stat fstat;
  if (fillStat(fstat) != 0) {
    err = fmt("Could not get file status: %s", strerror(errno));
    return false;
  }
  return true;
}

bool File::isFile()
{
  a2_struct_stat fstat;
  if (fillStat(fstat) < 0) {
    return false;
  }
  return S_ISREG(fstat.st_mode) == 1;
}

bool File::isDir()
{
  a2_struct_stat fstat;
  if (fillStat(fstat) < 0) {
    return false;
  }
  return S_ISDIR(fstat.st_mode) == 1;
}

bool File::remove()
{
  if (isFile()) {
    return a2unlink(utf8ToWChar(name_).c_str()) == 0;
  }
  else if (isDir()) {
    return a2rmdir(utf8ToWChar(name_).c_str()) == 0;
  }
  else {
    return false;
  }
}

#ifdef __MINGW32__
namespace {
HANDLE openFile(const std::string& filename, bool readOnly = true)
{
  DWORD desiredAccess = GENERIC_READ | (readOnly ? 0 : GENERIC_WRITE);
  DWORD sharedMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  DWORD creationDisp = OPEN_EXISTING;
  return CreateFileW(utf8ToWChar(filename).c_str(), desiredAccess, sharedMode,
                     /* lpSecurityAttributes */ nullptr, creationDisp,
                     FILE_ATTRIBUTE_NORMAL, /* hTemplateFile */ nullptr);
}
} // namespace
#endif // __MINGW32__

int64_t File::size()
{
#ifdef __MINGW32__
  // _wstat cannot be used for symlink.  It always returns 0.  Quoted
  // from https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx:
  //
  //   _wstat does not work with Windows Vista symbolic links. In
  //   these cases, _wstat will always report a file size of 0. _stat
  //   does work correctly with symbolic links.
  auto hn = openFile(name_);
  if (hn == INVALID_HANDLE_VALUE) {
    return 0;
  }
  LARGE_INTEGER fileSize;
  const auto rv = GetFileSizeEx(hn, &fileSize);
  CloseHandle(hn);
  return rv ? fileSize.QuadPart : 0;
#else  // !__MINGW32__
  a2_struct_stat fstat;
  if (fillStat(fstat) < 0) {
    return 0;
  }
  return fstat.st_size;
#endif // !__MINGW32__
}

bool File::mkdirs()
{
  if (isDir()) {
    return false;
  }
#ifdef __MINGW32__
  std::string path = name_;
  for (std::string::iterator i = path.begin(), eoi = path.end(); i != eoi;
       ++i) {
    if (*i == '\\') {
      *i = '/';
    }
  }
  std::string::iterator dbegin;
  if (util::startsWith(path, "//")) {
    // UNC path
    std::string::size_type hostEnd = path.find('/', 2);
    if (hostEnd == std::string::npos) {
      // UNC path with only hostname considered as an error.
      return false;
    }
    else if (hostEnd == 2) {
      // If path starts with "///", it is not considered as UNC.
      dbegin = path.begin();
    }
    else {
      std::string::iterator i = path.begin() + hostEnd;
      std::string::iterator eoi = path.end();
      // //host/mount/dir/...
      //       |     |
      //       i (at this point)
      //             |
      //             dbegin (will be)
      // Skip to after first directory part. This is because
      // //host/dir appears to be non-directory and mkdir it fails.
      for (; i != eoi && *i == '/'; ++i)
        ;
      for (; i != eoi && *i != '/'; ++i)
        ;
      dbegin = i;
      A2_LOG_DEBUG(
          fmt("UNC Prefix %s", std::string(path.begin(), dbegin).c_str()));
    }
  }
  else {
    dbegin = path.begin();
  }
  std::string::iterator begin = path.begin();
  std::string::iterator end = path.end();
  for (std::string::iterator i = dbegin; i != end;) {
#else  // !__MINGW32__
  std::string::iterator begin = name_.begin();
  std::string::iterator end = name_.end();
  for (std::string::iterator i = begin; i != end;) {
#endif // !__MINGW32__
    std::string::iterator j = std::find(i, end, '/');
    if (std::distance(i, j) == 0) {
      ++i;
      continue;
    }
    i = j;
    if (i != end) {
      ++i;
    }
#ifdef __MINGW32__
    if (*(j - 1) == ':') {
      // This is a drive letter, e.g. C:, so skip it.
      continue;
    }
#endif // __MINGW32__
    std::string dir(begin, j);
    A2_LOG_DEBUG(fmt("Making directory %s", dir.c_str()));
    if (File(dir).isDir()) {
      A2_LOG_DEBUG(fmt("%s exists and is a directory.", dir.c_str()));
      continue;
    }
    if (a2mkdir(utf8ToWChar(dir).c_str(), DIR_OPEN_MODE) == -1) {
      A2_LOG_DEBUG(fmt("Failed to create %s", dir.c_str()));
      return false;
    }
  }
  return true;
} // namespace aria2

mode_t File::mode()
{
  a2_struct_stat fstat;
  if (fillStat(fstat) < 0) {
    return 0;
  }
  return fstat.st_mode;
}

std::string File::getBasename() const
{
  std::string::size_type lastSlashIndex =
      name_.find_last_of(getPathSeparators());
  if (lastSlashIndex == std::string::npos) {
    return name_;
  }
  else {
    return name_.substr(lastSlashIndex + 1);
  }
}

std::string File::getDirname() const
{
  std::string::size_type lastSlashIndex =
      name_.find_last_of(getPathSeparators());
  if (lastSlashIndex == std::string::npos) {
    if (name_.empty()) {
      return A2STR::NIL;
    }
    else {
      return ".";
    }
  }
  else if (lastSlashIndex == 0) {
    return "/";
  }
  else {
    return name_.substr(0, lastSlashIndex);
  }
}

bool File::isDir(const std::string& filename) { return File(filename).isDir(); }

bool File::renameTo(const std::string& dest)
{
#ifdef __MINGW32__
  // MinGW's rename() doesn't delete an existing destination.  Better
  // to use MoveFileEx, which usually provides atomic move in aria2
  // usecase.
  if (MoveFileExW(utf8ToWChar(name_).c_str(), utf8ToWChar(dest).c_str(),
                  MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
    name_ = dest;
    return true;
  }

  return false;
#else  // !__MINGW32__
  if (rename(name_.c_str(), dest.c_str()) == 0) {
    name_ = dest;
    return true;
  }
  else {
    return false;
  }
#endif // !__MINGW32__
}

bool File::utime(const Time& actime, const Time& modtime) const
{
#if defined(HAVE_UTIMES) && !defined(__MINGW32__)
  struct timeval times[2] = {{actime.getTimeFromEpoch(), 0},
                             {modtime.getTimeFromEpoch(), 0}};
  return utimes(name_.c_str(), times) == 0;
#elif defined(__MINGW32__)
  auto hn = openFile(name_, false);
  if (hn == INVALID_HANDLE_VALUE) {
    auto errNum = GetLastError();
    A2_LOG_ERROR(fmt(EX_FILE_OPEN, name_.c_str(),
                     util::formatLastError(errNum).c_str()));
    return false;
  }
  // Use SetFileTime because Windows _wutime takes DST into
  // consideration.
  //
  // std::chrono::time_point::time_since_epoch returns the amount of
  // time between it and epoch Jan 1, 1970.  OTOH, FILETIME structure
  // expects the epoch as Jan 1, 1601.  The resolution is 100
  // nanoseconds.
  constexpr auto offset = 116444736000000000LL;
  uint64_t at = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    actime.getTime().time_since_epoch())
                        .count() /
                    100 +
                offset;
  uint64_t mt = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    modtime.getTime().time_since_epoch())
                        .count() /
                    100 +
                offset;
  FILETIME att{static_cast<DWORD>(at & 0xffffffff),
               static_cast<DWORD>(at >> 32)};
  FILETIME mtt{static_cast<DWORD>(mt & 0xffffffff),
               static_cast<DWORD>(mt >> 32)};
  auto rv = SetFileTime(hn, nullptr, &att, &mtt);
  if (!rv) {
    auto errNum = GetLastError();
    A2_LOG_ERROR(fmt("SetFileTime failed, cause: %s",
                     util::formatLastError(errNum).c_str()));
  }
  CloseHandle(hn);
  return rv;
#else  // !defined(HAVE_UTIMES) && !defined(__MINGW32__)
  a2utimbuf ub;
  ub.actime = actime.getTimeFromEpoch();
  ub.modtime = modtime.getTimeFromEpoch();
  return a2utime(utf8ToWChar(name_).c_str(), &ub) == 0;
#endif // !defined(HAVE_UTIMES) && !defined(__MINGW32__)
}

Time File::getModifiedTime()
{
  a2_struct_stat fstat;
  if (fillStat(fstat) < 0) {
    return 0;
  }
  return Time(fstat.st_mtime);
}

std::string File::getCurrentDir()
{
#ifdef __MINGW32__
  const size_t buflen = 2048;
  wchar_t buf[buflen];
  if (_wgetcwd(buf, buflen)) {
    return wCharToUtf8(buf);
  }
  else {
    return ".";
  }
#else  // !__MINGW32__
  const size_t buflen = 2048;
  char buf[buflen];
  if (getcwd(buf, buflen)) {
    return std::string(buf);
  }
  else {
    return ".";
  }
#endif // !__MINGW32__
}

const char* File::getPathSeparators()
{
#ifdef __MINGW32__
  return "/\\";
#else  // !__MINGW32__
  return "/";
#endif // !__MINGW32__
}

} // namespace aria2
