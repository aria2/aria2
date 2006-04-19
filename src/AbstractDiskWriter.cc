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
#include "DlAbortEx.h"
#include "File.h"
#include "Util.h"
#include "message.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

AbstractDiskWriter::AbstractDiskWriter():fd(0) {
#ifdef ENABLE_SHA1DIGEST
  sha1DigestInit(ctx);
#endif // ENABLE_SHA1DIGEST
}

AbstractDiskWriter::~AbstractDiskWriter() {
  closeFile();
#ifdef ENABLE_SHA1DIGEST
  sha1DigestFree(ctx);
#endif // ENABLE_SHA1DIGEST
}

void AbstractDiskWriter::openFile(const string& filename) {
  File f(filename);
  if(f.exists()) {
    openExistingFile(filename);
  } else {
    initAndOpenFile(filename);
  }
}

void AbstractDiskWriter::closeFile() {
  if(fd >= 0) {
    close(fd);
    fd = -1;
  }
}

void AbstractDiskWriter::openExistingFile(const string& filename) {
  File f(filename);
  if(!f.isFile()) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), "file not found");
  }

  if((fd = open(filename.c_str(), O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }
}

void AbstractDiskWriter::createFile(const string& filename, int addFlags) {
  // TODO proper filename handling needed
  assert(filename.size());
//   if(filename.empty()) {
//     filename = "index.html";
//   }
  if((fd = open(filename.c_str(), O_CREAT|O_RDWR|O_TRUNC|addFlags, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }  
}

void AbstractDiskWriter::writeDataInternal(const char* data, int len) {
  if(write(fd, data, len) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
}

int AbstractDiskWriter::readDataInternal(char* data, int len) {
  int ret;
  if((ret = read(fd, data, len)) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
  return ret;
}

string AbstractDiskWriter::sha1Sum(long long int offset, long long int length) {
#ifdef ENABLE_SHA1DIGEST
  sha1DigestReset(ctx);
  try {
    int BUFSIZE = 16*1024;
    char buf[BUFSIZE];
    for(int i = 0; i < length/BUFSIZE; i++) {
      if(BUFSIZE != readData(buf, BUFSIZE, offset)) {
	throw "error";
      }
      sha1DigestUpdate(ctx, buf, BUFSIZE);
      offset += BUFSIZE;
    }
    int r = length%BUFSIZE;
    if(r > 0) {
      if(r != readData(buf, r, offset)) {
	throw "error";
      }
      sha1DigestUpdate(ctx, buf, r);
    }
    unsigned char hashValue[20];
    sha1DigestFinal(ctx, hashValue);
    return Util::toHex(hashValue, 20);
  } catch(string ex) {
    throw new DlAbortEx(strerror(errno));
  }
#else
  return "";
#endif // ENABLE_SHA1DIGEST
}

void AbstractDiskWriter::seek(long long int offset) {
  if(offset != lseek(fd, offset, SEEK_SET)) {
    throw new DlAbortEx(strerror(errno));
  }
}

void AbstractDiskWriter::writeData(const char* data, int len, long long int offset) {
  seek(offset);
  writeDataInternal(data, len);
}

int AbstractDiskWriter::readData(char* data, int len, long long int offset) {
  seek(offset);
  return readDataInternal(data, len);
}
