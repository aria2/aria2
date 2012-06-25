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
# include <utime.h>
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

File::File(const File& c) : name_(c.name_) {}

File::~File() {}

File& File::operator=(const File& c)
{
  if(this != &c) {
    name_ = c.name_;
  }
  return *this;
}

int File::fillStat(a2_struct_stat& fstat) {
  return a2stat(utf8ToWChar(name_).c_str(), &fstat);
}

bool File::exists() {
  a2_struct_stat fstat;
  return fillStat(fstat) == 0;
}

bool File::isFile() {
  a2_struct_stat fstat;
  if(fillStat(fstat) < 0) {
    return false;
  }
  return S_ISREG(fstat.st_mode) == 1;
}

bool File::isDir() {
  a2_struct_stat fstat;
  if(fillStat(fstat) < 0) {
    return false;
  }
  return S_ISDIR(fstat.st_mode) == 1;
}

bool File::remove() {
  if(isFile()) {
    return a2unlink(utf8ToWChar(name_).c_str()) == 0;
  } else if(isDir()) {
    return a2rmdir(utf8ToWChar(name_).c_str()) == 0;
  } else {
    return false;
  }
}

int64_t File::size() {
  a2_struct_stat fstat;
  if(fillStat(fstat) < 0) {
    return 0;
  }
  return fstat.st_size;
}

bool File::mkdirs() {
  if(isDir()) {
    return false;
  }
#ifdef __MINGW32__
  std::string path = name_;
  for(std::string::iterator i = path.begin(), eoi = path.end(); i != eoi; ++i) {
    if(*i == '\\') {
      *i = '/';
    }
  }
  std::string::iterator dbegin;
  if(util::startsWith(path, "//")) {
    // UNC path
    std::string::size_type hostEnd = path.find('/', 2);
    if(hostEnd == std::string::npos) {
      // UNC path with only hostname considered as an error.
      return false;
    } else if(hostEnd == 2) {
      // If path starts with "///", it is not considered as UNC.
      dbegin = path.begin();
    } else {
      std::string::iterator i = path.begin()+hostEnd;
      std::string::iterator eoi = path.end();
      // //host/mount/dir/...
      //       |     |
      //       i (at this point)
      //             |
      //             dbegin (will be)
      // Skip to after first directory part. This is because
      // //host/dir appears to be non-directory and mkdir it fails.
      for(; i != eoi && *i == '/'; ++i);
      for(; i != eoi && *i != '/'; ++i);
      dbegin = i;
      A2_LOG_DEBUG(fmt("UNC Prefix %s",
                       std::string(path.begin(), dbegin).c_str()));
   }
  } else {
    dbegin = path.begin();
  }
  std::string::iterator begin = path.begin();
  std::string::iterator end = path.end();
  for(std::string::iterator i = dbegin; i != end;) {
#else // !__MINGW32__
  std::string::iterator begin = name_.begin();
  std::string::iterator end = name_.end();
  for(std::string::iterator i = begin; i != end;) {
#endif // !__MINGW32__
    std::string::iterator j = std::find(i, end, '/');
    if(std::distance(i, j) == 0) {
      ++i;
      continue;
    }
    i = j;
    if(i != end) {
      ++i;
    }
#ifdef __MINGW32__
    if(*(j-1) == ':') {
      // This is a drive letter, e.g. C:, so skip it.
      continue;
    }
#endif // __MINGW32__
    std::string dir(begin, j);
    A2_LOG_DEBUG(fmt("Making directory %s", dir.c_str()));
    if(File(dir).isDir()) {
      A2_LOG_DEBUG(fmt("%s exists and is a directory.", dir.c_str()));
      continue;
    }
    if(a2mkdir(utf8ToWChar(dir).c_str(), DIR_OPEN_MODE) == -1) {
      A2_LOG_DEBUG(fmt("Failed to create %s", dir.c_str()));
      return false;
    }
  }
  return true;
}

mode_t File::mode()
{
  a2_struct_stat fstat;
  if(fillStat(fstat) < 0) {
    return 0;
  }
  return fstat.st_mode;
}

std::string File::getBasename() const
{
  std::string::size_type lastSlashIndex =
    name_.find_last_of(getPathSeparators());
  if(lastSlashIndex == std::string::npos) {
    return name_;
  } else {
    return name_.substr(lastSlashIndex+1);
  }
}

std::string File::getDirname() const
{
  std::string::size_type lastSlashIndex =
    name_.find_last_of(getPathSeparators());
  if(lastSlashIndex == std::string::npos) {
    if(name_.empty()) {
      return A2STR::NIL;
    } else {
      return A2STR::DOT_C;
    }
  } else if(lastSlashIndex == 0) {
    return A2STR::SLASH_C;
  } else {
    return name_.substr(0, lastSlashIndex);
  }
}

bool File::isDir(const std::string& filename)
{
  return File(filename).isDir();
}

bool File::renameTo(const std::string& dest)
{
#ifdef __MINGW32__
  /* MinGW's rename() doesn't delete an existing destination */
  if (_waccess(utf8ToWChar(dest).c_str(), 0) == 0) {
    if (a2unlink(utf8ToWChar(dest).c_str()) != 0) {
      return false;
    }
  }
#endif // __MINGW32__
  if(a2rename(utf8ToWChar(name_).c_str(), utf8ToWChar(dest).c_str()) == 0) {
    name_ = dest;
    return true;
  } else {
    return false;
  }
}

bool File::utime(const Time& actime, const Time& modtime) const
{
#if defined HAVE_UTIMES && !(defined __MINGW32__)
  struct timeval times[2] = {
    { actime.getTime(), 0 },
    { modtime.getTime(), 0 }
  };
  return utimes(name_.c_str(), times) == 0;
#else // !HAVE_UTIMES
  a2utimbuf ub;
  ub.actime = actime.getTime();
  ub.modtime = modtime.getTime();
  return a2utime(utf8ToWChar(name_).c_str(), &ub) == 0;
#endif // !HAVE_UTIMES
}

Time File::getModifiedTime()
{
  a2_struct_stat fstat;
  if(fillStat(fstat) < 0) {
    return 0;
  }
  return Time(fstat.st_mtime);
}

std::string File::getCurrentDir()
{
#ifdef __MINGW32__
  const size_t buflen = 2048;
  wchar_t buf[buflen];
  if(_wgetcwd(buf, buflen)) {
    return wCharToUtf8(buf);
  } else {
    return A2STR::DOT_C;
  }
#else // !__MINGW32__
  const size_t buflen = 2048;
  char buf[buflen];
  if(getcwd(buf, buflen)) {
    return std::string(buf);
  } else {
    return A2STR::DOT_C;
  }
#endif // !__MINGW32__
}

const std::string& File::getPathSeparators()
{
#ifdef __MINGW32__
  static std::string s = "/\\";
#else // !__MINGW32__
  static std::string s = "/";
#endif // !__MINGW32__
  return s;
}

} // namespace aria2
