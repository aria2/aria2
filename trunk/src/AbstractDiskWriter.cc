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
#include "AbstractDiskWriter.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "DlAbortEx.h"
#include "File.h"

AbstractDiskWriter::AbstractDiskWriter():fd(0) {}

AbstractDiskWriter::~AbstractDiskWriter() {
  if(fd != 0) {
    close(fd);
  }
}

void AbstractDiskWriter::closeFile() {
  if(fd != 0) {
    close(fd);
    fd = 0;
  }
}

void AbstractDiskWriter::openExistingFile(string filename) {
  File f(filename);
  if(!f.isFile()) {
    throw new DlAbortEx(strerror(errno));
  }

  if((fd = open(filename.c_str(), O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
}

void AbstractDiskWriter::createFile(string filename, int addFlags) {
  // TODO proper filename handling needed
  assert(filename.size());
//   if(filename.empty()) {
//     filename = "index.html";
//   }
  if((fd = open(filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC|addFlags, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(strerror(errno));
  }  
}

void AbstractDiskWriter::writeDataInternal(const char* data, int len) {
  if(write(fd, data, len) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
}
