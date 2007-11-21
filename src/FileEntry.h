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
#ifndef _D_FILE_ENTRY_H_
#define _D_FILE_ENTRY_H_

#include "common.h"
#include "File.h"

class FileEntry {
private:
  string path;
  Strings _uris;
  int64_t length;
  int64_t offset;
  bool extracted;
  bool requested;
public:
  FileEntry():length(0), offset(0), extracted(false), requested(false) {}

  FileEntry(const string& path, int64_t length, int64_t offset, const Strings& uris = Strings());

  ~FileEntry();

  FileEntry& operator=(const FileEntry& entry);

  string getBasename() const
  {
    return File(path).getBasename();
  }

  string getDirname() const
  {
    return File(path).getDirname();
  }

  const string& getPath() const { return path; }

  void setPath(const string& path) { this->path = path; }

  int64_t getLength() const { return length; }

  void setLength(int64_t length) { this->length = length; }

  int64_t getOffset() const { return offset; }

  void setOffset(int64_t offset) { this->offset = offset; }

  bool isExtracted() const { return extracted; }

  void setExtracted(bool flag) { this->extracted = flag; }

  bool isRequested() const { return requested; }

  void setRequested(bool flag) { this->requested = flag; }

  void setupDir(const string& parentDir);

  const Strings& getAssociatedUris() const
  {
    return _uris;
  }

  bool operator<(const FileEntry& fileEntry) const;
};

typedef SharedHandle<FileEntry> FileEntryHandle;
typedef deque<FileEntryHandle> FileEntries;

#endif // _D_FILE_ENTRY_H_
