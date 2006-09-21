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

AbstractDiskWriter::AbstractDiskWriter():
  fd(0)
#ifdef ENABLE_MESSAGE_DIGEST				       
  ,ctx(DIGEST_ALGO_SHA1)
#endif // ENABLE_MESSAGE_DIGEST
{
#ifdef ENABLE_MESSAGE_DIGEST
  ctx.digestInit();
#endif // ENABLE_MESSAGE_DIGEST
}

AbstractDiskWriter::~AbstractDiskWriter() {
  closeFile();
#ifdef ENABLE_MESSAGE_DIGEST
  ctx.digestFree();
#endif // ENABLE_MESSAGE_DIGEST
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
  this->filename = filename;
  File f(filename);
  if(!f.isFile()) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), "file not found");
  }

  if((fd = open(filename.c_str(), O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }
}

void AbstractDiskWriter::createFile(const string& filename, int addFlags) {
  this->filename = filename;
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
    throw new DlAbortEx(EX_FILE_WRITE, filename.c_str(), strerror(errno));
  }
}

int AbstractDiskWriter::readDataInternal(char* data, int len) {
  int ret;
  if((ret = read(fd, data, len)) < 0) {
    throw new DlAbortEx(EX_FILE_READ, filename.c_str(), strerror(errno));
  }
  return ret;
}

string AbstractDiskWriter::sha1Sum(long long int offset, long long int length) {
#ifdef ENABLE_MESSAGE_DIGEST
  ctx.digestReset();
  try {
    int BUFSIZE = 16*1024;
    char buf[BUFSIZE];
    for(int i = 0; i < length/BUFSIZE; i++) {
      if(BUFSIZE != readData(buf, BUFSIZE, offset)) {
	throw string("error");
      }
      ctx.digestUpdate(buf, BUFSIZE);
      offset += BUFSIZE;
    }
    int r = length%BUFSIZE;
    if(r > 0) {
      if(r != readData(buf, r, offset)) {
	throw string("error");
      }
      ctx.digestUpdate(buf, r);
    }
    unsigned char hashValue[20];
    ctx.digestFinal(hashValue);
    return Util::toHex(hashValue, 20);
  } catch(string ex) {
    throw new DlAbortEx(EX_FILE_SHA1SUM, filename.c_str(), strerror(errno));
  }
#else
  return "";
#endif // ENABLE_MESSAGE_DIGEST
}

void AbstractDiskWriter::seek(long long int offset) {
  if(offset != lseek(fd, offset, SEEK_SET)) {
    throw new DlAbortEx(EX_FILE_SEEK, filename.c_str(), strerror(errno));
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
