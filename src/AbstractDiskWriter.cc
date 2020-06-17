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
#ifdef HAVE_MMAP
#  include <sys/mman.h>
#endif // HAVE_MMAP
#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <cassert>

#include "File.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "a2io.h"
#include "fmt.h"
#include "DownloadFailureException.h"
#include "error_code.h"
#include "LogFactory.h"

namespace aria2 {

AbstractDiskWriter::AbstractDiskWriter(const std::string& filename)
    : filename_(filename),
      fd_(A2_BAD_FD),
#ifdef __MINGW32__
      mapView_(0),
#else  // !__MINGW32__
#endif // !__MINGW32__
      readOnly_(false),
      enableMmap_(false),
      mapaddr_(nullptr),
      maplen_(0)

{
}

AbstractDiskWriter::~AbstractDiskWriter() { closeFile(); }

namespace {
// Returns error code depending on the platform. For MinGW32, return
// the value of GetLastError(). Otherwise, return errno.
int fileError()
{
#ifdef __MINGW32__
  return GetLastError();
#else  // !__MINGW32__
  return errno;
#endif // !__MINGW32__
}
} // namespace

namespace {
// Formats error message for error code errNum. For MinGW32, errNum is
// assumed to be the return value of GetLastError(). Otherwise, it is
// errno.
std::string fileStrerror(int errNum)
{
#ifdef __MINGW32__
  auto msg = util::formatLastError(errNum);
  if (msg.empty()) {
    char buf[256];
    snprintf(buf, sizeof(buf), "File I/O error %x", errNum);
    return buf;
  }
  return msg;
#else  // !__MINGW32__
  return util::safeStrerror(errNum);
#endif // !__MINGW32__
}
} // namespace

void AbstractDiskWriter::openFile(int64_t totalLength)
{
  try {
    openExistingFile(totalLength);
  }
  catch (RecoverableException& e) {
    if (
#ifdef __MINGW32__
        e.getErrNum() == ERROR_FILE_NOT_FOUND ||
        e.getErrNum() == ERROR_PATH_NOT_FOUND
#else  // !__MINGW32__
        e.getErrNum() == ENOENT
#endif // !__MINGW32__
    ) {
      initAndOpenFile(totalLength);
    }
    else {
      throw;
    }
  }
}

void AbstractDiskWriter::closeFile()
{
#if defined(HAVE_MMAP) || defined(__MINGW32__)
  if (mapaddr_) {
    int errNum = 0;
#  ifdef __MINGW32__
    if (!UnmapViewOfFile(mapaddr_)) {
      errNum = GetLastError();
    }
    CloseHandle(mapView_);
    mapView_ = INVALID_HANDLE_VALUE;
#  else  // !__MINGW32__
    if (munmap(mapaddr_, maplen_) == -1) {
      errNum = errno;
    }
#  endif // !__MINGW32__
    if (errNum != 0) {
      int errNum = fileError();
      A2_LOG_ERROR(fmt("Unmapping file %s failed: %s", filename_.c_str(),
                       fileStrerror(errNum).c_str()));
    }
    else {
      A2_LOG_INFO(fmt("Unmapping file %s succeeded", filename_.c_str()));
    }
    mapaddr_ = nullptr;
    maplen_ = 0;
  }
#endif // HAVE_MMAP || defined __MINGW32__
  if (fd_ != A2_BAD_FD) {
#ifdef __MINGW32__
    CloseHandle(fd_);
#else  // !__MINGW32__
    close(fd_);
#endif // !__MINGW32__
    fd_ = A2_BAD_FD;
  }
}

namespace {
#ifdef __MINGW32__
HANDLE openFileWithFlags(const std::string& filename, int flags,
                         error_code::Value errCode)
{
  HANDLE hn;
  DWORD desiredAccess = 0;
  DWORD sharedMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  DWORD creationDisp = 0;

  if (flags & O_RDWR) {
    desiredAccess = GENERIC_READ | GENERIC_WRITE;
  }
  else if (flags & O_WRONLY) {
    desiredAccess = GENERIC_WRITE;
  }
  else {
    desiredAccess = GENERIC_READ;
  }
  if (flags & O_CREAT) {
    if (flags & O_TRUNC) {
      creationDisp |= CREATE_ALWAYS;
    }
    else {
      creationDisp |= CREATE_NEW;
    }
  }
  else {
    creationDisp |= OPEN_EXISTING;
  }
  hn = CreateFileW(utf8ToWChar(filename).c_str(), desiredAccess, sharedMode,
                   /* lpSecurityAttributes */ 0, creationDisp,
                   FILE_ATTRIBUTE_NORMAL, /* hTemplateFile */ 0);
  if (hn == INVALID_HANDLE_VALUE) {
    int errNum = GetLastError();
    throw DL_ABORT_EX3(
        errNum,
        fmt(EX_FILE_OPEN, filename.c_str(), fileStrerror(errNum).c_str()),
        errCode);
  }
  return hn;
}
#else // !__MINGW32__
int openFileWithFlags(const std::string& filename, int flags,
                      error_code::Value errCode)
{
  int fd;
  while ((fd = a2open(utf8ToWChar(filename).c_str(), flags, OPEN_MODE)) == -1 &&
         errno == EINTR)
    ;
  if (fd < 0) {
    int errNum = errno;
    throw DL_ABORT_EX3(
        errNum,
        fmt(EX_FILE_OPEN, filename.c_str(), util::safeStrerror(errNum).c_str()),
        errCode);
  }
  util::make_fd_cloexec(fd);
#  if defined(__APPLE__) && defined(__MACH__)
  // This may reduce memory consumption on Mac OS X.
  fcntl(fd, F_NOCACHE, 1);
#  endif // __APPLE__ && __MACH__
  return fd;
}
#endif   // !__MINGW32__
} // namespace

void AbstractDiskWriter::openExistingFile(int64_t totalLength)
{
  int flags = O_BINARY;
  if (readOnly_) {
    flags |= O_RDONLY;
  }
  else {
    flags |= O_RDWR;
  }
  fd_ = openFileWithFlags(filename_, flags, error_code::FILE_OPEN_ERROR);
}

void AbstractDiskWriter::createFile(int addFlags)
{
  assert(!filename_.empty());
  util::mkdirs(File(filename_).getDirname());
  fd_ = openFileWithFlags(filename_,
                          O_CREAT | O_RDWR | O_TRUNC | O_BINARY | addFlags,
                          error_code::FILE_CREATE_ERROR);
}

ssize_t AbstractDiskWriter::writeDataInternal(const unsigned char* data,
                                              size_t len, int64_t offset)
{
  if (mapaddr_) {
    memcpy(mapaddr_ + offset, data, len);
    return len;
  }
  else {
    ssize_t writtenLength = 0;
    seek(offset);
    while ((size_t)writtenLength < len) {
#ifdef __MINGW32__
      DWORD nwrite;
      if (WriteFile(fd_, data + writtenLength, len - writtenLength, &nwrite,
                    0)) {
        writtenLength += nwrite;
      }
      else {
        return -1;
      }
#else  // !__MINGW32__
      ssize_t ret = 0;
      while ((ret = write(fd_, data + writtenLength, len - writtenLength)) ==
                 -1 &&
             errno == EINTR)
        ;
      if (ret == -1) {
        return -1;
      }
      writtenLength += ret;
#endif // !__MINGW32__
    }
    return writtenLength;
  }
}

ssize_t AbstractDiskWriter::readDataInternal(unsigned char* data, size_t len,
                                             int64_t offset)
{
  if (mapaddr_) {
    if (offset >= maplen_) {
      return 0;
    }
    auto readlen = std::min(maplen_ - offset, static_cast<int64_t>(len));
    memcpy(data, mapaddr_ + offset, readlen);
    return readlen;
  }
  else {
    seek(offset);
#ifdef __MINGW32__
    DWORD nread;
    if (ReadFile(fd_, data, len, &nread, 0)) {
      return nread;
    }
    else {
      return -1;
    }
#else  // !__MINGW32__
    ssize_t ret = 0;
    while ((ret = read(fd_, data, len)) == -1 && errno == EINTR)
      ;
    return ret;
#endif // !__MINGW32__
  }
}

void AbstractDiskWriter::seek(int64_t offset)
{
  assert(offset >= 0);
#ifdef __MINGW32__
  LARGE_INTEGER fileLength;
  fileLength.QuadPart = offset;
  if (SetFilePointerEx(fd_, fileLength, 0, FILE_BEGIN) == 0)
#else  // !__MINGW32__
  if (a2lseek(fd_, offset, SEEK_SET) == (a2_off_t)-1)
#endif // !__MINGW32__
  {
    int errNum = fileError();
    throw DL_ABORT_EX2(
        fmt(EX_FILE_SEEK, filename_.c_str(), fileStrerror(errNum).c_str()),
        error_code::FILE_IO_ERROR);
  }
}

void AbstractDiskWriter::ensureMmapWrite(size_t len, int64_t offset)
{
#if defined(HAVE_MMAP) || defined(__MINGW32__)
  if (enableMmap_) {
    if (mapaddr_) {
      if (static_cast<int64_t>(len + offset) > maplen_) {
        int errNum = 0;
#  ifdef __MINGW32__
        if (!UnmapViewOfFile(mapaddr_)) {
          errNum = GetLastError();
        }
        CloseHandle(mapView_);
        mapView_ = INVALID_HANDLE_VALUE;
#  else  // !__MINGW32__
        if (munmap(mapaddr_, maplen_) == -1) {
          errNum = errno;
        }
#  endif // !__MINGW32__
        if (errNum != 0) {
          A2_LOG_ERROR(fmt("Unmapping file %s failed: %s", filename_.c_str(),
                           fileStrerror(errNum).c_str()));
        }
        mapaddr_ = nullptr;
        maplen_ = 0;
        enableMmap_ = false;
      }
    }
    else {
      int64_t filesize = size();

      if (filesize == 0) {
        // mapping 0 length file is useless.  Also munmap with size ==
        // 0 will fail with EINVAL.
        enableMmap_ = false;
        return;
      }

      if (static_cast<uint64_t>(std::numeric_limits<size_t>::max()) <
          static_cast<uint64_t>(filesize)) {
        // filesize could overflow in 32bit OS with 64bit off_t type
        // the filesize will be truncated if provided as a 32bit size_t
        enableMmap_ = false;
        return;
      }

      int errNum = 0;
      if (static_cast<int64_t>(len + offset) <= filesize) {
#  ifdef __MINGW32__
        mapView_ = CreateFileMapping(fd_, 0, PAGE_READWRITE, filesize >> 32,
                                     filesize & 0xffffffffu, 0);
        if (mapView_) {
          mapaddr_ = reinterpret_cast<unsigned char*>(
              MapViewOfFile(mapView_, FILE_MAP_WRITE, 0, 0, 0));
          if (!mapaddr_) {
            errNum = GetLastError();
            CloseHandle(mapView_);
            mapView_ = INVALID_HANDLE_VALUE;
          }
        }
        else {
          errNum = GetLastError();
        }
#  else  // !__MINGW32__
        auto pa =
            mmap(nullptr, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);

        if (pa == MAP_FAILED) {
          errNum = errno;
        }
        else {
          mapaddr_ = reinterpret_cast<unsigned char*>(pa);
        }
#  endif // !__MINGW32__
        if (mapaddr_) {
          A2_LOG_DEBUG(fmt("Mapping file %s succeeded, length=%" PRId64 "",
                           filename_.c_str(), static_cast<uint64_t>(filesize)));
          maplen_ = filesize;
        }
        else {
          A2_LOG_WARN(fmt("Mapping file %s failed: %s", filename_.c_str(),
                          fileStrerror(errNum).c_str()));
          enableMmap_ = false;
        }
      }
    }
  }
#endif // HAVE_MMAP || __MINGW32__
}

namespace {
// Returns true if |errNum| indicates that disk is full.
bool isDiskFullError(int errNum)
{
  return
#ifdef __MINGW32__
      errNum == ERROR_DISK_FULL || errNum == ERROR_HANDLE_DISK_FULL
#else  // !__MINGW32__
      errNum == ENOSPC
#endif // !__MINGW32__
      ;
}
} // namespace

void AbstractDiskWriter::writeData(const unsigned char* data, size_t len,
                                   int64_t offset)
{
  ensureMmapWrite(len, offset);
  if (writeDataInternal(data, len, offset) < 0) {
    int errNum = fileError();
    // If the error indicates disk full situation, throw
    // DownloadFailureException and abort download instantly.
    if (isDiskFullError(errNum)) {
      throw DOWNLOAD_FAILURE_EXCEPTION3(
          errNum,
          fmt(EX_FILE_WRITE, filename_.c_str(), fileStrerror(errNum).c_str()),
          error_code::NOT_ENOUGH_DISK_SPACE);
    }
    else {
      throw DL_ABORT_EX3(
          errNum,
          fmt(EX_FILE_WRITE, filename_.c_str(), fileStrerror(errNum).c_str()),
          error_code::FILE_IO_ERROR);
    }
  }
}

ssize_t AbstractDiskWriter::readData(unsigned char* data, size_t len,
                                     int64_t offset)
{
  ssize_t ret;
  if ((ret = readDataInternal(data, len, offset)) < 0) {
    int errNum = fileError();
    throw DL_ABORT_EX3(
        errNum,
        fmt(EX_FILE_READ, filename_.c_str(), fileStrerror(errNum).c_str()),
        error_code::FILE_IO_ERROR);
  }
  return ret;
}

void AbstractDiskWriter::truncate(int64_t length)
{
  if (fd_ == A2_BAD_FD) {
    throw DL_ABORT_EX("File not yet opened.");
  }
#ifdef __MINGW32__
  // Since mingw32's ftruncate cannot handle over 2GB files, we use
  // SetEndOfFile instead.
  seek(length);
  if (SetEndOfFile(fd_) == 0)
#else  // !__MINGW32__
  if (a2ftruncate(fd_, length) == -1)
#endif // !__MINGW32__
  {
    int errNum = fileError();
    throw DL_ABORT_EX2(
        fmt("File truncation failed. cause: %s", fileStrerror(errNum).c_str()),
        error_code::FILE_IO_ERROR);
  }
}

void AbstractDiskWriter::allocate(int64_t offset, int64_t length, bool sparse)
{
  if (fd_ == A2_BAD_FD) {
    throw DL_ABORT_EX("File not yet opened.");
  }
  if (sparse) {
#ifdef __MINGW32__
    DWORD bytesReturned;
    if (!DeviceIoControl(fd_, FSCTL_SET_SPARSE, 0, 0, 0, 0, &bytesReturned,
                         0)) {
      A2_LOG_WARN(fmt("Making file sparse failed or pending: %s",
                      fileStrerror(GetLastError()).c_str()));
    }
#endif // __MINGW32__
    truncate(offset + length);
    return;
  }
#ifdef HAVE_SOME_FALLOCATE
#  ifdef __MINGW32__
  truncate(offset + length);
  if (!SetFileValidData(fd_, offset + length)) {
    auto errNum = fileError();
    A2_LOG_WARN(fmt(
        "File allocation (SetFileValidData) failed (cause: %s). File will be "
        "allocated by filling zero, which blocks whole aria2 execution. Run "
        "aria2 as an administrator or use a different file allocation method "
        "(see --file-allocation).",
        fileStrerror(errNum).c_str()));
  }
#  elif defined(__APPLE__) && defined(__MACH__)
  const auto toalloc = offset + length - size();
  fstore_t fstore = {F_ALLOCATECONTIG | F_ALLOCATEALL, F_PEOFPOSMODE, 0,
                     toalloc, 0};
  if (fcntl(fd_, F_PREALLOCATE, &fstore) == -1) {
    // Retry non-contig.
    fstore.fst_flags = F_ALLOCATEALL;
    if (fcntl(fd_, F_PREALLOCATE, &fstore) == -1) {
      int err = errno;
      throw DL_ABORT_EX3(
          err,
          fmt("fcntl(F_PREALLOCATE) of %" PRId64 " failed. cause: %s",
              fstore.fst_length, util::safeStrerror(err).c_str()),
          error_code::FILE_IO_ERROR);
    }
  }
  // This forces the allocation on disk.
  ftruncate(fd_, offset + length);
#  elif HAVE_FALLOCATE
  // For linux, we use fallocate to detect file system supports
  // fallocate or not.
  int r;
  while ((r = fallocate(fd_, 0, offset, length)) == -1 && errno == EINTR)
    ;
  int errNum = errno;
  if (r == -1) {
    throw DL_ABORT_EX3(
        errNum,
        fmt("fallocate failed. cause: %s", util::safeStrerror(errNum).c_str()),
        isDiskFullError(errNum) ? error_code::NOT_ENOUGH_DISK_SPACE
                                : error_code::FILE_IO_ERROR);
  }
#  elif HAVE_POSIX_FALLOCATE
  int r = posix_fallocate(fd_, offset, length);
  if (r != 0) {
    throw DL_ABORT_EX3(
        r,
        fmt("posix_fallocate failed. cause: %s", util::safeStrerror(r).c_str()),
        isDiskFullError(r) ? error_code::NOT_ENOUGH_DISK_SPACE
                           : error_code::FILE_IO_ERROR);
  }
#  else
#    error "no *_fallocate function available."
#  endif
#endif // HAVE_SOME_FALLOCATE
}

int64_t AbstractDiskWriter::size() { return File(filename_).size(); }

void AbstractDiskWriter::enableReadOnly() { readOnly_ = true; }

void AbstractDiskWriter::disableReadOnly() { readOnly_ = false; }

void AbstractDiskWriter::enableMmap() { enableMmap_ = true; }

void AbstractDiskWriter::dropCache(int64_t len, int64_t offset)
{
#ifdef HAVE_POSIX_FADVISE
  posix_fadvise(fd_, offset, len, POSIX_FADV_DONTNEED);
#endif // HAVE_POSIX_FADVISE
}

void AbstractDiskWriter::flushOSBuffers()
{
  if (fd_ == A2_BAD_FD) {
    return;
  }
#ifdef __MINGW32__
  FlushFileBuffers(fd_);
#else // !__MINGW32__
  fsync(fd_);
#endif // __MINGW32__
}

} // namespace aria2
