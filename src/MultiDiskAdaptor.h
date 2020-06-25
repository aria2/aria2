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
#ifndef D_MULTI_DISK_ADAPTOR_H
#define D_MULTI_DISK_ADAPTOR_H

#include "DiskAdaptor.h"

namespace aria2 {

class MultiFileAllocationIterator;
class FileEntry;
class DiskWriter;

class DiskWriterEntry {
private:
  std::shared_ptr<FileEntry> fileEntry_;
  std::unique_ptr<DiskWriter> diskWriter_;
  bool open_;
  bool needsFileAllocation_;
  bool needsDiskWriter_;

public:
  DiskWriterEntry(const std::shared_ptr<FileEntry>& fileEntry);

  const std::string& getFilePath() const;

  void initAndOpenFile();

  void openFile();

  void openExistingFile();

  void closeFile();

  bool isOpen() const { return open_; }

  bool fileExists();

  int64_t size() const;

  const std::shared_ptr<FileEntry>& getFileEntry() const { return fileEntry_; }

  void setDiskWriter(std::unique_ptr<DiskWriter> diskWriter);

  const std::unique_ptr<DiskWriter>& getDiskWriter() const
  {
    return diskWriter_;
  }

  bool operator<(const DiskWriterEntry& entry) const;

  bool needsFileAllocation() const { return needsFileAllocation_; }

  void needsFileAllocation(bool f) { needsFileAllocation_ = f; }

  bool needsDiskWriter() const { return needsDiskWriter_; }

  void needsDiskWriter(bool f) { needsDiskWriter_ = f; }
};

typedef std::vector<std::unique_ptr<DiskWriterEntry>> DiskWriterEntries;

class MultiDiskAdaptor : public DiskAdaptor {
  friend class MultiFileAllocationIterator;

private:
  int32_t pieceLength_;
  DiskWriterEntries diskWriterEntries_;

  std::vector<DiskWriterEntry*> openedDiskWriterEntries_;

  bool readOnly_;

  void resetDiskWriterEntries();

  void openIfNot(DiskWriterEntry* entry, void (DiskWriterEntry::*f)());

  ssize_t readData(unsigned char* data, size_t len, int64_t offset,
                   bool dropCache);

  static const int DEFAULT_MAX_OPEN_FILES = 100;

public:
  MultiDiskAdaptor();
  ~MultiDiskAdaptor();

  virtual void initAndOpenFile() CXX11_OVERRIDE;

  virtual void openFile() CXX11_OVERRIDE;

  virtual void openExistingFile() CXX11_OVERRIDE;

  virtual void closeFile() CXX11_OVERRIDE;

  virtual void writeData(const unsigned char* data, size_t len,
                         int64_t offset) CXX11_OVERRIDE;

  virtual ssize_t readData(unsigned char* data, size_t len,
                           int64_t offset) CXX11_OVERRIDE;

  virtual ssize_t readDataDropCache(unsigned char* data, size_t len,
                                    int64_t offset) CXX11_OVERRIDE;

  virtual void writeCache(const WrDiskCacheEntry* entry) CXX11_OVERRIDE;

  virtual void flushOSBuffers() CXX11_OVERRIDE;

  virtual bool fileExists() CXX11_OVERRIDE;

  virtual int64_t size() CXX11_OVERRIDE;

  virtual std::unique_ptr<FileAllocationIterator>
  fileAllocationIterator() CXX11_OVERRIDE;

  virtual void enableReadOnly() CXX11_OVERRIDE;

  virtual void disableReadOnly() CXX11_OVERRIDE;

  virtual bool isReadOnlyEnabled() const CXX11_OVERRIDE { return readOnly_; }

  // Enables mmap feature. This method must be called after files are
  // opened.
  virtual void enableMmap() CXX11_OVERRIDE;

  void setPieceLength(int32_t pieceLength) { pieceLength_ = pieceLength; }

  int32_t getPieceLength() const { return pieceLength_; }

  virtual void cutTrailingGarbage() CXX11_OVERRIDE;

  virtual size_t utime(const Time& actime, const Time& modtime) CXX11_OVERRIDE;

  const DiskWriterEntries& getDiskWriterEntries() const
  {
    return diskWriterEntries_;
  }

  virtual size_t tryCloseFile(size_t numClose) CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_MULTI_DISK_ADAPTOR_H
