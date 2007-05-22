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
#include "LogFactory.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

AbstractDiskWriter::AbstractDiskWriter():
  fd(-1),
  fileAllocator(0),
  glowFileAllocator(0),
  logger(LogFactory::getInstance())
{}

AbstractDiskWriter::~AbstractDiskWriter()
{
  closeFile();
}

void AbstractDiskWriter::openFile(const string& filename, int64_t totalLength) {
  File f(filename);
  if(f.exists()) {
    openExistingFile(filename);
  } else {
    initAndOpenFile(filename, totalLength);
  }
}

void AbstractDiskWriter::closeFile() {
  if(fd >= 0) {
    close(fd);
    fd = -1;
  }
}

void AbstractDiskWriter::openExistingFile(const string& filename, int64_t totalLength) {
  this->filename = filename;
  File f(filename);
  if(!f.isFile()) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), "file not found");
  }

  if((fd = open(filename.c_str(), O_RDWR, OPEN_MODE)) < 0) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }
  if(f.size() < totalLength) {
    if(!fileAllocator.isNull()) {
      logger->notice("Allocating file %s, %s bytes",
		     filename.c_str(),
		     Util::ullitos(totalLength).c_str());
      glowFileAllocator->allocate(fd, totalLength);
    }
  }
}

void AbstractDiskWriter::createFile(const string& filename, int32_t addFlags) {
  this->filename = filename;
  // TODO proper filename handling needed
  assert(filename.size());
//   if(filename.empty()) {
//     filename = "index.html";
//   }
  if((fd = open(filename.c_str(), O_CREAT|O_RDWR|O_TRUNC|addFlags, OPEN_MODE)) < 0) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }  
}

int32_t AbstractDiskWriter::writeDataInternal(const char* data, int32_t len) {
  return write(fd, data, len);
}

int32_t AbstractDiskWriter::readDataInternal(char* data, int32_t len) {
  return read(fd, data, len);
}

#ifdef ENABLE_MESSAGE_DIGEST
string AbstractDiskWriter::messageDigest(int64_t offset, int64_t length,
					 const MessageDigestContext::DigestAlgo& algo)
{
  MessageDigestContext ctx(algo);
  ctx.digestInit();

  int32_t BUFSIZE = 16*1024;
  char buf[BUFSIZE];
  for(int64_t i = 0; i < length/BUFSIZE; i++) {
    if(BUFSIZE != readData(buf, BUFSIZE, offset)) {
      throw new DlAbortEx(EX_FILE_SHA1SUM, filename.c_str(), strerror(errno));
    }
    ctx.digestUpdate(buf, BUFSIZE);
    offset += BUFSIZE;
  }
  int32_t r = length%BUFSIZE;
  if(r > 0) {
    if(r != readData(buf, r, offset)) {
      throw new DlAbortEx(EX_FILE_SHA1SUM, filename.c_str(), strerror(errno));
    }
    ctx.digestUpdate(buf, r);
  }
  unsigned char hashValue[20];
  ctx.digestFinal(hashValue);

  return Util::toHex(hashValue, 20);
}
#endif // ENABLE_MESSAGE_DIGEST

void AbstractDiskWriter::seek(int64_t offset) {
  if(offset != lseek(fd, offset, SEEK_SET)) {
    throw new DlAbortEx(EX_FILE_SEEK, filename.c_str(), strerror(errno));
  }
}

void AbstractDiskWriter::writeData(const char* data, int32_t len, int64_t offset) {
  seek(offset);
  if(writeDataInternal(data, len) < 0) {
    throw new DlAbortEx(EX_FILE_WRITE, filename.c_str(), strerror(errno));
  }
}

int32_t AbstractDiskWriter::readData(char* data, int32_t len, int64_t offset) {
  int32_t ret;
  seek(offset);
  if((ret = readDataInternal(data, len)) < 0) {
    throw new DlAbortEx(EX_FILE_READ, filename.c_str(), strerror(errno));
  }
  return ret;
}

void AbstractDiskWriter::truncate(int64_t length)
{
  ftruncate(fd, length);
}

int64_t AbstractDiskWriter::size() const
{
  struct stat fileStat;
  if(fstat(fd, &fileStat) < 0) {
    return 0;
  }
  return fileStat.st_size;
}
