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
  SharedHandle<FileEntry> fileEntry_;
  SharedHandle<DiskWriter> diskWriter_;
  bool open_;
  bool needsFileAllocation_;
public:
  DiskWriterEntry(const SharedHandle<FileEntry>& fileEntry);

  const std::string& getFilePath() const;

  void initAndOpenFile();

  void openFile();

  void openExistingFile();

  void closeFile();

  bool isOpen() const
  {
    return open_;
  }

  bool fileExists();

  int64_t size() const;

  const SharedHandle<FileEntry>& getFileEntry() const
  {
    return fileEntry_;
  }

  void setDiskWriter(const SharedHandle<DiskWriter>& diskWriter);

  const SharedHandle<DiskWriter>& getDiskWriter() const
  {
    return diskWriter_;
  }

  bool operator<(const DiskWriterEntry& entry) const;

  bool needsFileAllocation() const
  {
    return needsFileAllocation_;
  }

  void needsFileAllocation(bool f)
  {
    needsFileAllocation_ = f;
  }

};

typedef SharedHandle<DiskWriterEntry> DiskWriterEntryHandle;

typedef std::vector<DiskWriterEntryHandle> DiskWriterEntries;

class MultiDiskAdaptor : public DiskAdaptor {
  friend class MultiFileAllocationIterator;
private:
  int32_t pieceLength_;
  DiskWriterEntries diskWriterEntries_;

  std::vector<SharedHandle<DiskWriterEntry> > openedDiskWriterEntries_;

  int maxOpenFiles_;

  bool readOnly_;
  bool enableMmap_;

  void resetDiskWriterEntries();

  void openIfNot(const SharedHandle<DiskWriterEntry>& entry,
                 void (DiskWriterEntry::*f)());
 
  static const int DEFAULT_MAX_OPEN_FILES = 100;

public:
  MultiDiskAdaptor();
  ~MultiDiskAdaptor();

  virtual void initAndOpenFile();

  virtual void openFile();

  virtual void openExistingFile();

  virtual void closeFile();

  virtual void writeData(const unsigned char* data, size_t len,
                         int64_t offset);

  virtual ssize_t readData(unsigned char* data, size_t len, int64_t offset);

  virtual bool fileExists();

  virtual int64_t size();

  virtual SharedHandle<FileAllocationIterator> fileAllocationIterator();

  virtual void enableReadOnly();

  virtual void disableReadOnly();

  virtual bool isReadOnlyEnabled() const { return readOnly_; }

  virtual void enableMmap();

  void setPieceLength(int32_t pieceLength)
  {
    pieceLength_ = pieceLength;
  }

  int32_t getPieceLength() const {
    return pieceLength_;
  }

  virtual void cutTrailingGarbage();

  void setMaxOpenFiles(int maxOpenFiles);

  virtual size_t utime(const Time& actime, const Time& modtime);

  const std::vector<SharedHandle<DiskWriterEntry> >&
  getDiskWriterEntries() const
  {
    return diskWriterEntries_;
  }

};

typedef SharedHandle<MultiDiskAdaptor> MultiDiskAdaptorHandle;

} // namespace aria2

#endif // D_MULTI_DISK_ADAPTOR_H
