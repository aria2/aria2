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
#include "DlAbortEx.h"
#include "a2io.h"
#include "fmt.h"
#include "DownloadFailureException.h"
#include "error_code.h"

namespace aria2 {

AbstractDiskWriter::AbstractDiskWriter(const std::string& filename)
  : filename_(filename),
    fd_(-1),
    readOnly_(false),
    directIOAllowed_(false)
{}

AbstractDiskWriter::~AbstractDiskWriter()
{
  closeFile();
}

void AbstractDiskWriter::openFile(uint64_t totalLength)
{
  try {
    openExistingFile(totalLength);
  } catch(RecoverableException& e) {
    if(e.getErrNum() == ENOENT) {
      initAndOpenFile(totalLength);
    } else {
      throw;
    }
  }
}

void AbstractDiskWriter::closeFile()
{
  if(fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

void AbstractDiskWriter::openExistingFile(uint64_t totalLength)
{
  int flags = O_BINARY;
  if(readOnly_) {
    flags |= O_RDONLY;
  } else {
    flags |= O_RDWR;
  }

  while((fd_ = open(filename_.c_str(), flags, OPEN_MODE)) == -1 &&
        errno == EINTR);
  if(fd_ < 0) {
    int errNum = errno;
    throw DL_ABORT_EX3
      (errNum,
       fmt(EX_FILE_OPEN,
           filename_.c_str(),
           util::safeStrerror(errNum).c_str()),
       error_code::FILE_OPEN_ERROR);
  }
}

void AbstractDiskWriter::createFile(int addFlags)
{
  assert(!filename_.empty());
  util::mkdirs(File(filename_).getDirname());

  while((fd_ = open(filename_.c_str(), O_CREAT|O_RDWR|O_TRUNC|O_BINARY|addFlags,
                    OPEN_MODE)) == -1 && errno == EINTR);
  if(fd_ < 0) {
    int errNum = errno;
    throw DL_ABORT_EX3
      (errNum,
       fmt(EX_FILE_OPEN,
           filename_.c_str(),
           util::safeStrerror(errNum).c_str()),
       error_code::FILE_CREATE_ERROR);
  }  
}

ssize_t AbstractDiskWriter::writeDataInternal(const unsigned char* data, size_t len)
{
  ssize_t writtenLength = 0;
  while((size_t)writtenLength < len) {
    ssize_t ret = 0;
    while((ret = write(fd_, data+writtenLength, len-writtenLength)) == -1 && errno == EINTR);
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
  while((ret = read(fd_, data, len)) == -1 && errno == EINTR);
  return ret;
}

void AbstractDiskWriter::seek(off_t offset)
{
  if(a2lseek(fd_, offset, SEEK_SET) == (off_t)-1) {
    int errNum = errno;
    throw DL_ABORT_EX2(fmt(EX_FILE_SEEK,
                           filename_.c_str(),
                           util::safeStrerror(errNum).c_str()),
                       error_code::FILE_IO_ERROR);
  }
}

void AbstractDiskWriter::writeData(const unsigned char* data, size_t len, off_t offset)
{
  seek(offset);
  if(writeDataInternal(data, len) < 0) {
    int errNum = errno;
    // If errno is ENOSPC(not enough space in device), throw
    // DownloadFailureException and abort download instantly.
    if(errNum == ENOSPC) {
      throw DOWNLOAD_FAILURE_EXCEPTION3
        (errNum,
         fmt(EX_FILE_WRITE,
             filename_.c_str(),
             util::safeStrerror(errNum).c_str()),
         error_code::NOT_ENOUGH_DISK_SPACE);
    } else {
      throw DL_ABORT_EX3
        (errNum,
         fmt(EX_FILE_WRITE,
             filename_.c_str(),
             util::safeStrerror(errNum).c_str()),
         error_code::FILE_IO_ERROR);
    }
  }
}

ssize_t AbstractDiskWriter::readData(unsigned char* data, size_t len, off_t offset)
{
  ssize_t ret;
  seek(offset);
  if((ret = readDataInternal(data, len)) < 0) {
    int errNum = errno;
    throw DL_ABORT_EX3
      (errNum,
       fmt(EX_FILE_READ,
           filename_.c_str(),
           util::safeStrerror(errNum).c_str()),
       error_code::FILE_IO_ERROR);
  }
  return ret;
}

void AbstractDiskWriter::truncate(uint64_t length)
{
  if(fd_ == -1) {
    throw DL_ABORT_EX("File not opened.");
  }
#ifdef __MINGW32__
  // Since mingw32's ftruncate cannot handle over 2GB files, we use SetEndOfFile
  // instead.
  HANDLE handle = LongToHandle(_get_osfhandle(fd_));
  seek(length);
  if(SetEndOfFile(handle) == 0) {
    throw DL_ABORT_EX2(fmt("SetEndOfFile failed. cause: %lx",
                           GetLastError()),
                       error_code::FILE_IO_ERROR);
  }
#else
  if(ftruncate(fd_, length) == -1) {
    int errNum = errno;
    throw DL_ABORT_EX3(errNum,
                       fmt("ftruncate failed. cause: %s",
                           util::safeStrerror(errNum).c_str()),
                       error_code::FILE_IO_ERROR);
  }
#endif
}

void AbstractDiskWriter::allocate(off_t offset, uint64_t length)
{
#ifdef  HAVE_SOME_FALLOCATE
  if(fd_ == -1) {
    throw DL_ABORT_EX("File not yet opened.");
  }
# ifdef HAVE_FALLOCATE
  // For linux, we use fallocate to detect file system supports
  // fallocate or not.
  int r;
  while((r = fallocate(fd_, 0, offset, length)) == -1 && errno == EINTR);
  int errNum = errno;
  if(r == -1) {
    throw DL_ABORT_EX3(errNum,
                       fmt("fallocate failed. cause: %s",
                           util::safeStrerror(errNum).c_str()),
                       error_code::FILE_IO_ERROR);
  }
# elif HAVE_POSIX_FALLOCATE
  int r = posix_fallocate(fd_, offset, length);
  if(r != 0) {
    throw DL_ABORT_EX3(r,
                       fmt("posix_fallocate failed. cause: %s",
                           util::safeStrerror(r).c_str()),
                       error_code::FILE_IO_ERROR);
  }
# else
#  error "no *_fallocate function available."
# endif
#endif // HAVE_SOME_FALLOCATE
}

uint64_t AbstractDiskWriter::size()
{
  return File(filename_).size();
}

void AbstractDiskWriter::enableDirectIO()
{
#ifdef ENABLE_DIRECT_IO
  if(directIOAllowed_) {
    int flg;
    while((flg = fcntl(fd_, F_GETFL)) == -1 && errno == EINTR);
    while(fcntl(fd_, F_SETFL, flg|O_DIRECT) == -1 && errno == EINTR);
  }
#endif // ENABLE_DIRECT_IO
}

void AbstractDiskWriter::disableDirectIO()
{
#ifdef ENABLE_DIRECT_IO
  int flg;
  while((flg = fcntl(fd_, F_GETFL)) == -1 && errno == EINTR);
  while(fcntl(fd_, F_SETFL, flg&(~O_DIRECT)) == -1 && errno == EINTR);
#endif // ENABLE_DIRECT_IO
}

void AbstractDiskWriter::enableReadOnly()
{
  readOnly_ = true;
}

void AbstractDiskWriter::disableReadOnly()
{
  readOnly_ = false;
}

} // namespace aria2
