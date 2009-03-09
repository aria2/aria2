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
#ifndef _D_MULTI_DISK_ADAPTOR_H_
#define _D_MULTI_DISK_ADAPTOR_H_

#include "DiskAdaptor.h"

namespace aria2 {

class MultiFileAllocationIterator;
class FileEntry;
class DiskWriter;

class DiskWriterEntry {
private:
  SharedHandle<FileEntry> fileEntry;
  SharedHandle<DiskWriter> diskWriter;
  bool _open;
  bool _directIO;
  bool _needsFileAllocation;
public:
  DiskWriterEntry(const SharedHandle<FileEntry>& fileEntry);

  ~DiskWriterEntry();

  const std::string& getFilePath() const;

  void initAndOpenFile();

  void openFile();

  void openExistingFile();

  void closeFile();

  bool isOpen() const;

  bool fileExists();

  uint64_t size() const;

  SharedHandle<FileEntry> getFileEntry() const;

  void setDiskWriter(const SharedHandle<DiskWriter>& diskWriter);

  SharedHandle<DiskWriter> getDiskWriter() const;

  bool operator<(const DiskWriterEntry& entry) const;

  // Set _directIO to true.
  // Additionally, if diskWriter is opened, diskWriter->enableDirectIO() is
  // called.
  void enableDirectIO();

  // Set _directIO to false.
  // Additionally, if diskWriter is opened, diskWriter->disableDirectIO() is
  // called.
  void disableDirectIO();

  bool needsFileAllocation() const;

  void needsFileAllocation(bool f);
};

typedef SharedHandle<DiskWriterEntry> DiskWriterEntryHandle;

typedef std::deque<DiskWriterEntryHandle> DiskWriterEntries;

class MultiDiskAdaptor : public DiskAdaptor {
  friend class MultiFileAllocationIterator;
private:
  std::string topDir;
  size_t pieceLength;
  DiskWriterEntries diskWriterEntries;

  std::deque<SharedHandle<DiskWriterEntry> > _openedDiskWriterEntries;

  size_t _maxOpenFiles;

  bool _directIOAllowed;

  bool _readOnly;

  void resetDiskWriterEntries();

  void mkdir() const;

  std::string getTopDirPath() const;

  void openIfNot(const SharedHandle<DiskWriterEntry>& entry,
		 void (DiskWriterEntry::*f)());
 
  static const size_t DEFAULT_MAX_OPEN_FILES = 100;

public:
  MultiDiskAdaptor();

  virtual ~MultiDiskAdaptor();

  virtual void initAndOpenFile();

  virtual void openFile();

  virtual void openExistingFile();

  virtual void closeFile();

  virtual void onDownloadComplete();

  virtual void writeData(const unsigned char* data, size_t len,
			 off_t offset);

  virtual ssize_t readData(unsigned char* data, size_t len, off_t offset);

  virtual bool fileExists();

  virtual std::string getFilePath()
  {
    return getTopDirPath();
  }

  virtual uint64_t size();

  virtual SharedHandle<FileAllocationIterator> fileAllocationIterator();

  virtual void enableDirectIO();

  virtual void disableDirectIO();

  virtual void enableReadOnly();

  virtual void disableReadOnly();

  void setTopDir(const std::string& topDir) {
    this->topDir = topDir;
  }

  const std::string& getTopDir() const {
    return topDir;
  }

  void setPieceLength(size_t pieceLength) {
    this->pieceLength = pieceLength;
  }

  size_t getPieceLength() const {
    return pieceLength;
  }

  virtual bool directIOAllowed() const
  {
    return _directIOAllowed;
  }

  void setDirectIOAllowed(bool b)
  {
    _directIOAllowed = b;
  }

  virtual void cutTrailingGarbage();

  void setMaxOpenFiles(size_t maxOpenFiles);

  virtual size_t utime(const Time& actime, const Time& modtime);

  const std::deque<SharedHandle<DiskWriterEntry> >&
  getDiskWriterEntries() const;
};

typedef SharedHandle<MultiDiskAdaptor> MultiDiskAdaptorHandle;

} // namespace aria2

#endif // _D_MULTI_DISK_ADAPTOR_H_
