/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "File.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

File::File(const string& name):name(name) {}

File::~File() {}

int File::fillStat(struct stat& fstat) {
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

long long int File::size() {
  struct stat fstat;
  if(fillStat(fstat) < 0) {
    return 0;
  }
  return fstat.st_size;
}
