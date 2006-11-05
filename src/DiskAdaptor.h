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
#ifndef _D_DISK_ADAPTOR_H_
#define _D_DISK_ADAPTOR_H_

#include "common.h"
#include "FileEntry.h"
#include "Directory.h"
#include "DiskWriter.h"
#include "Logger.h"
#include "FileEntry.h"

class DiskAdaptor {
protected:
  DiskWriter* diskWriter;
  string storeDir;
  FileEntries fileEntries;
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

  FileEntryHandle getFileEntryFromPath(const string& fileEntryPath) const;

  const FileEntries& getFileEntries() const { return fileEntries; }

  bool addDownloadEntry(const string& fileEntryPath);
  bool addDownloadEntry(int index);
  void addAllDownloadEntry();
  void removeAllDownloadEntry();

  void setStoreDir(const string& storeDir) { this->storeDir = storeDir; }

  string getStoreDir() const { return this->storeDir; }
};

#endif // _D_DISK_ADAPTOR_H_
