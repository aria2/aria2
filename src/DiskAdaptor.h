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
#ifndef D_DISK_ADAPTOR_H
#define D_DISK_ADAPTOR_H

#include "BinaryStream.h"

#include <string>
#include <vector>
#include <memory>

#include "TimeA2.h"

namespace aria2 {

class FileEntry;
class FileAllocationIterator;
class WrDiskCacheEntry;
class OpenedFileCounter;

class DiskAdaptor : public BinaryStream {
public:
  enum FileAllocationMethod {
    FILE_ALLOC_ADAPTIVE,
    FILE_ALLOC_FALLOC,
    FILE_ALLOC_TRUNC
  };

  DiskAdaptor();
  virtual ~DiskAdaptor();

  virtual void openFile() = 0;

  virtual void closeFile() = 0;

  virtual void openExistingFile() = 0;

  virtual void initAndOpenFile() = 0;

  virtual bool fileExists() = 0;

  virtual int64_t size() = 0;

  template <typename InputIterator>
  void setFileEntries(InputIterator first, InputIterator last)
  {
    fileEntries_.assign(first, last);
  }

  const std::vector<std::shared_ptr<FileEntry>>& getFileEntries() const
  {
    return fileEntries_;
  }

  virtual std::unique_ptr<FileAllocationIterator> fileAllocationIterator() = 0;

  virtual void enableReadOnly() {}

  virtual void disableReadOnly() {}

  virtual bool isReadOnlyEnabled() const { return false; }

  // Enables mmap feature. Some derived classes may require that files
  // have been opened before this method call.
  virtual void enableMmap() {}

  // Assumed each file length is stored in fileEntries or DiskAdaptor knows it.
  // If each actual file's length is larger than that, truncate file to that
  // length.
  // Call one of openFile/openExistingFile/initAndOpenFile before calling this
  // function.
  virtual void cutTrailingGarbage() = 0;

  // Returns the number of files, the actime and modtime of which are
  // successfully changed.
  virtual size_t utime(const Time& actime, const Time& modtime) = 0;

  // Just like readData(), but drop cache after read.
  virtual ssize_t readDataDropCache(unsigned char* data, size_t len,
                                    int64_t offset) = 0;

  // Writes cached data to the underlying disk.
  virtual void writeCache(const WrDiskCacheEntry* entry) = 0;

  // Force physical write of data from OS buffer cache.
  virtual void flushOSBuffers(){};

  void setFileAllocationMethod(FileAllocationMethod method)
  {
    fileAllocationMethod_ = method;
  }

  int getFileAllocationMethod() const { return fileAllocationMethod_; }

  // Closes at most |numClose| files if possible. This method is used to
  // ensure that global number of open file stays under certain limit.
  // Returns the number of closed files.
  virtual size_t tryCloseFile(size_t numClose) { return 0; }

  void
  setOpenedFileCounter(std::shared_ptr<OpenedFileCounter> openedFileCounter)
  {
    openedFileCounter_ = std::move(openedFileCounter);
  }

  const std::shared_ptr<OpenedFileCounter>& getOpenedFileCounter() const
  {
    return openedFileCounter_;
  }

private:
  std::vector<std::shared_ptr<FileEntry>> fileEntries_;

  FileAllocationMethod fileAllocationMethod_;

  std::shared_ptr<OpenedFileCounter> openedFileCounter_;
};

} // namespace aria2

#endif // D_DISK_ADAPTOR_H
