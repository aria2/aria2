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
#include "Util.h"
#define basename posix_basename
#include <libgen.h>
// use GNU version basename
#undef basename
#include <cstring>
#include <stdlib.h>
#include <deque>

namespace aria2 {

#ifdef __MINGW32__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif // __MINGW32__

File::File(const std::string& name):name(name) {}

File::~File() {}

int32_t File::fillStat(struct stat& fstat) {
  return stat(name.c_str(), &fstat);
}

bool File::exists() {
  struct stat fstat;
  return fillStat(fstat) == 0;
}

bool File::isFile() {
  struct stat fstat;
  if(fillStat(fstat) < 0) {
    return false;
  }
  return S_ISREG(fstat.st_mode) == 1;
}

bool File::isDir() {
  struct stat fstat;
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

int64_t File::size() {
  struct stat fstat;
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
  if(Util::startsWith(name, "/")) {
    accDir = "/";
  }
  for(std::deque<std::string>::const_iterator itr = dirs.begin(); itr != dirs.end();
      itr++, accDir += "/") {
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
  struct stat fstat;
  if(fillStat(fstat) < 0) {
    return 0;
  }
  return fstat.st_mode;
}

std::string File::getBasename() const
{
  char* s = strdup(name.c_str());
  std::string bname = basename(s);
  free(s);
  return bname;
}

std::string File::getDirname() const
{
  char* s = strdup(name.c_str());
  std::string dname = dirname(s);
  free(s);
  return dname;
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

} // namespace aria2
