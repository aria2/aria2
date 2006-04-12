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
#ifndef _D_MULTI_DISK_WRITER_H_
#define _D_MULTI_DISK_WRITER_H_

#include "DefaultDiskWriter.h"
#include "TorrentMan.h"
#include "messageDigest.h"

class DiskWriterEntry {
public:
  FileEntry fileEntry;
  DiskWriter* diskWriter;
  bool enabled;
public:
  DiskWriterEntry(const FileEntry& fileEntry, bool enabled):fileEntry(fileEntry), enabled(enabled) {
    if(enabled) {
      diskWriter = new DefaultDiskWriter();
    }
  }
  ~DiskWriterEntry() {
    if(enabled) {
      delete diskWriter;
    }
  }
};

typedef deque<DiskWriterEntry*> DiskWriterEntries;

class MultiDiskWriter : public DiskWriter {
private:
  DiskWriterEntries diskWriterEntries;

  bool isInRange(const DiskWriterEntry* entry, long long int offset) const;
  int calculateLength(const DiskWriterEntry* entry, long long int fileOffset, int rem) const;
  void clearEntries();
#ifdef ENABLE_SHA1DIGEST
  MessageDigestContext ctx;
  void hashUpdate(const DiskWriterEntry* entry, long long int offset, long long int length) const;
#endif // ENABLE_SHA1DIGEST

public:
  MultiDiskWriter();
  virtual ~MultiDiskWriter();

  void setMultiFileEntries(const MultiFileEntries& multiFileEntries, int pieceLength);

  virtual void openFile(const string& filename);
  virtual void initAndOpenFile(string filename);
  virtual void closeFile();
  virtual void openExistingFile(string filename);
  virtual void writeData(const char* data, int len, long long int position = 0);
  virtual int readData(char* data, int len, long long int position);
  virtual string sha1Sum(long long int offset, long long int length);
};

#endif // _D_MULTI_DISK_WRITER_H_
