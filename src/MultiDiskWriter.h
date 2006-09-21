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
#ifndef _D_MULTI_DISK_WRITER_H_
#define _D_MULTI_DISK_WRITER_H_

#include "DefaultDiskWriter.h"
#include "TorrentMan.h"
#include "messageDigest.h"

class DiskWriterEntry {
public:
  FileEntry fileEntry;
  DiskWriter* diskWriter;
public:
  DiskWriterEntry(const FileEntry& fileEntry):fileEntry(fileEntry) {
    diskWriter = new DefaultDiskWriter(this->fileEntry.length);
  }
  ~DiskWriterEntry() {
    delete diskWriter;
  }
};

typedef deque<DiskWriterEntry*> DiskWriterEntries;

class MultiDiskWriter : public DiskWriter {
private:
  DiskWriterEntries diskWriterEntries;
  int pieceLength;
  bool isInRange(const DiskWriterEntry* entry, long long int offset) const;
  int calculateLength(const DiskWriterEntry* entry, long long int fileOffset, int rem) const;
  void clearEntries();
  MessageDigestContext ctx;
  void hashUpdate(DiskWriterEntry* entry, long long int offset, long long int length);

public:
  MultiDiskWriter(int pieceLength);
  virtual ~MultiDiskWriter();

  void setFileEntries(const FileEntries& fileEntries);

  virtual void openFile(const string& filename);
  virtual void initAndOpenFile(const string& filename);
  virtual void closeFile();
  virtual void openExistingFile(const string& filename);
  virtual void writeData(const char* data, int len, long long int position = 0);
  virtual int readData(char* data, int len, long long int position);
  virtual string sha1Sum(long long int offset, long long int length);
};

#endif // _D_MULTI_DISK_WRITER_H_
