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
#ifndef _D_DISK_ADAPTOR_H_
#define _D_DISK_ADAPTOR_H_

#include "common.h"
#include "FileEntry.h"
#include "Directory.h"
#include "DiskWriter.h"
#include "Logger.h"

class DiskAdaptor {
protected:
  DiskWriter* diskWriter;
  string storeDir;
  FileEntries fileEntries;
  const Directory* topDir;
  const Logger* logger;
  virtual string getFilePath() const = 0;
public:
  DiskAdaptor(DiskWriter* diskWriter);
  virtual ~DiskAdaptor();

  virtual void openFile();
  virtual void closeFile();
  virtual void openExistingFile();
  virtual void initAndOpenFile();
  void writeData(const char* data, int len, long long int position);
  int readData(char* data, int len, long long int position);
  string sha1Sum(long long int offset, long long int length);

  virtual void onDownloadComplete() = 0;  

  void setFileEntries(const FileEntries& fileEntries) {
    this->fileEntries = fileEntries;
  }
  FileEntry getFileEntryFromPath(const string& fileEntryPath) const;
  bool addDownloadEntry(const string& fileEntryPath);
  bool addDownloadEntry(int index);
  void addAllDownloadEntry();
  void removeAllDownloadEntry();

  void setStoreDir(const string& storeDir) { this->storeDir = storeDir; }
  string getStoreDir() const { return this->storeDir; }

  void setTopDir(const Directory* dirctory) {
    if(this->topDir != NULL) {
      delete topDir;
    }
    this->topDir = dirctory;
  }
  const Directory* getTopDir() const { return this->topDir; }
};

#endif // _D_DISK_ADAPTOR_H_
