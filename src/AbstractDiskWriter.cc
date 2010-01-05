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
#include "AbstractDiskWriter.h"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <cassert>

#ifdef __MINGW32__
# include <windows.h>
#endif // __MINGW32__

#include "File.h"
#include "util.h"
#include "message.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DlAbortEx.h"
#include "a2io.h"
#include "StringFormat.h"
#include "DownloadFailureException.h"

namespace aria2 {

AbstractDiskWriter::AbstractDiskWriter(const std::string& filename):
  _filename(filename),
  fd(-1),
  _readOnly(false),
  _directIOAllowed(false),
  logger(LogFactory::getInstance()) {}

AbstractDiskWriter::~AbstractDiskWriter()
{
  closeFile();
}

void AbstractDiskWriter::openFile(uint64_t totalLength)
{
  if(File(_filename).exists()) {
    openExistingFile(totalLength);
  } else {
    initAndOpenFile(totalLength);
  }
}

void AbstractDiskWriter::closeFile()
{
  if(fd >= 0) {
    close(fd);
    fd = -1;
  }
}

void AbstractDiskWriter::openExistingFile(uint64_t totalLength)
{
  if(!File(_filename).exists()) {
    throw DL_ABORT_EX
      (StringFormat(EX_FILE_OPEN, _filename.c_str(), MSG_FILE_NOT_FOUND).str());
  }

  int flags = O_BINARY;
  if(_readOnly) {
    flags |= O_RDONLY;
  } else {
    flags |= O_RDWR;
  }

  if((fd = open(_filename.c_str(), flags, OPEN_MODE)) < 0) {
    throw DL_ABORT_EX
      (StringFormat(EX_FILE_OPEN, _filename.c_str(), strerror(errno)).str());
  }
}

void AbstractDiskWriter::createFile(int addFlags)
{
  assert(!_filename.empty());
  util::mkdirs(File(_filename).getDirname());
  if((fd = open(_filename.c_str(), O_CREAT|O_RDWR|O_TRUNC|O_BINARY|addFlags,
                OPEN_MODE)) < 0) {
    throw DL_ABORT_EX(StringFormat(EX_FILE_OPEN,
                                   _filename.c_str(), strerror(errno)).str());
  }  
}

ssize_t AbstractDiskWriter::writeDataInternal(const unsigned char* data, size_t len)
{
  ssize_t writtenLength = 0;
  while((size_t)writtenLength < len) {
    ssize_t ret = 0;
    while((ret = write(fd, data+writtenLength, len-writtenLength)) == -1 && errno == EINTR);
    if(ret == -1) {
      return -1;
    }
    writtenLength += ret;
  }
  return writtenLength;
}

ssize_t AbstractDiskWriter::readDataInternal(unsigned char* data, size_t len)
{
  ssize_t ret = 0;
  while((ret = read(fd, data, len)) == -1 && errno == EINTR);
  return ret;
}

void AbstractDiskWriter::seek(off_t offset)
{
  if(lseek(fd, offset, SEEK_SET) == (off_t)-1) {
    throw DL_ABORT_EX
      (StringFormat(EX_FILE_SEEK, _filename.c_str(), strerror(errno)).str());
  }
}

void AbstractDiskWriter::writeData(const unsigned char* data, size_t len, off_t offset)
{
  seek(offset);
  if(writeDataInternal(data, len) < 0) {
    // If errno is ENOSPC(not enough space in device), throw
    // DownloadFailureException and abort download instantly.
    if(errno == ENOSPC) {
      throw DOWNLOAD_FAILURE_EXCEPTION
        (StringFormat(EX_FILE_WRITE, _filename.c_str(), strerror(errno)).str());
    }
    throw DL_ABORT_EX(StringFormat(EX_FILE_WRITE,
                                   _filename.c_str(), strerror(errno)).str());
  }
}

ssize_t AbstractDiskWriter::readData(unsigned char* data, size_t len, off_t offset)
{
  ssize_t ret;
  seek(offset);
  if((ret = readDataInternal(data, len)) < 0) {
    throw DL_ABORT_EX(StringFormat(EX_FILE_READ,
                                   _filename.c_str(), strerror(errno)).str());
  }
  return ret;
}

void AbstractDiskWriter::truncate(uint64_t length)
{
  if(fd == -1) {
    throw DL_ABORT_EX("File not opened.");
  }
#ifdef __MINGW32__
  // Since mingw32's ftruncate cannot handle over 2GB files, we use SetEndOfFile
  // instead.
  HANDLE handle = LongToHandle(_get_osfhandle(fd));
  seek(length);
  if(SetEndOfFile(handle) == 0) {
    throw DL_ABORT_EX(StringFormat("SetEndOfFile failed. cause: %s",
                                   GetLastError()).str());
  }
#else
  if(ftruncate(fd, length) == -1) {
    throw DL_ABORT_EX(StringFormat("ftruncate failed. cause: %s",
                                   strerror(errno)).str());
  }
#endif
}

#ifdef HAVE_POSIX_FALLOCATE
void AbstractDiskWriter::allocate(off_t offset, uint64_t length)
{
  if(fd == -1) {
    throw DL_ABORT_EX("File not yet opened.");
  }
  int r = posix_fallocate(fd, offset, length);
  if(r != 0) {
    throw DL_ABORT_EX(StringFormat("posix_fallocate failed. cause: %s",
                                   strerror(r)).str());
  }
}
#endif // HAVE_POSIX_FALLOCATE

uint64_t AbstractDiskWriter::size()
{
  return File(_filename).size();
}

void AbstractDiskWriter::enableDirectIO()
{
#ifdef ENABLE_DIRECT_IO
  if(_directIOAllowed) {
    int flg;
    while((flg = fcntl(fd, F_GETFL)) == -1 && errno == EINTR);
    while(fcntl(fd, F_SETFL, flg|O_DIRECT) == -1 && errno == EINTR);
  }
#endif // ENABLE_DIRECT_IO
}

void AbstractDiskWriter::disableDirectIO()
{
#ifdef ENABLE_DIRECT_IO
  int flg;
  while((flg = fcntl(fd, F_GETFL)) == -1 && errno == EINTR);
  while(fcntl(fd, F_SETFL, flg&(~O_DIRECT)) == -1 && errno == EINTR);
#endif // ENABLE_DIRECT_IO
}

void AbstractDiskWriter::enableReadOnly()
{
  _readOnly = true;
}

void AbstractDiskWriter::disableReadOnly()
{
  _readOnly = false;
}

} // namespace aria2
