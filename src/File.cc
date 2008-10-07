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
#include "File.h"

#include <stdlib.h>
#include <sys/types.h>
#include <utime.h>

#include <deque>
#include <cstring>
#include <cstdio>

#include "Util.h"
#include "A2STR.h"

namespace aria2 {

#ifdef __MINGW32__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif // __MINGW32__

File::File(const std::string& name):name(name) {}

File::~File() {}

int File::fillStat(a2_struct_stat& fstat) {
  return stat(name.c_str(), &fstat);
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
    return unlink(name.c_str()) == 0;
  } else if(isDir()) {
    return rmdir(name.c_str()) == 0;
  } else {
    return false;
  }
}

uint64_t File::size() {
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
  std::deque<std::string> dirs;
  Util::slice(dirs, name, '/');
  if(!dirs.size()) {
    return true;
  }

  std::string accDir;
  if(Util::startsWith(name, A2STR::SLASH_C)) {
    accDir = A2STR::SLASH_C;
  }
  for(std::deque<std::string>::const_iterator itr = dirs.begin(); itr != dirs.end();
      itr++, accDir += A2STR::SLASH_C) {
    accDir += *itr;
    if(File(accDir).isDir()) {
      continue;
    }
    if(a2mkdir(accDir.c_str(), DIR_OPEN_MODE) == -1) {
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
  std::string::size_type lastSlashIndex = name.find_last_of(A2STR::SLASH_C);
  if(lastSlashIndex == std::string::npos) {
    return name;
  } else {
    return name.substr(lastSlashIndex+1);
  }
}

std::string File::getDirname() const
{
  std::string::size_type lastSlashIndex = name.find_last_of(A2STR::SLASH_C);
  if(lastSlashIndex == std::string::npos) {
    if(name.empty()) {
      return A2STR::NIL;
    } else {
      return A2STR::DOT_C;
    }
  } else if(lastSlashIndex == 0) {
    return A2STR::SLASH_C;
  } else {
    return name.substr(0, lastSlashIndex);
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
  if (_access(dest.c_str(), 0) == 0) {
    if (_unlink(dest.c_str()) != 0) {
      return false;
    }
  }
#endif // __MINGW32__
  if(rename(name.c_str(), dest.c_str()) == 0) {
    name = dest;
    return true;
  } else {
    return false;
  }
}

bool File::utime(const Time& actime, const Time& modtime) const
{
  struct utimbuf ub;
  ub.actime = actime.getTime();
  ub.modtime = modtime.getTime();
  return ::utime(name.c_str(), &ub) == 0;
}

Time File::getModifiedTime()
{
  a2_struct_stat fstat;
  if(fillStat(fstat) < 0) {
    return 0;
  }
  return Time(fstat.st_mtime);
}

} // namespace aria2
