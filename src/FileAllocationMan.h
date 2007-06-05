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
#ifndef _D_FILE_ALLOCATION_MAN_H_
#define _D_FILE_ALLOCATION_MAN_H_

#include "common.h"
#include "Request.h"
#include "RequestGroup.h"
#include "FileAllocationEntry.h"

class FileAllocationMan {
private:
  FileAllocationEntries _fileAllocationEntries;
  FileAllocationEntryHandle _currentFileAllocationEntry;
public:
  FileAllocationMan():_currentFileAllocationEntry(0) {}

  bool isFileAllocationBeingExecuted() const
  {
    return _currentFileAllocationEntry.get() != 0;
  }

  FileAllocationEntryHandle getCurrentFileAllocationEntry()
  {
    return _currentFileAllocationEntry;
  }

  void markCurrentFileAllocationEntryDone()
  {
    _currentFileAllocationEntry = 0;
  }

  bool nextFileAllocationEntryExists() const
  {
    return !_fileAllocationEntries.empty();
  }

  FileAllocationEntryHandle popNextFileAllocationEntry()
  {
    if(!nextFileAllocationEntryExists()) {
      return 0;
    }
    FileAllocationEntryHandle entry = _fileAllocationEntries.front();
    _fileAllocationEntries.pop_front();
    _currentFileAllocationEntry = entry;
    return entry;
  }

  void pushFileAllocationEntry(const FileAllocationEntryHandle& entry)
  {
    _fileAllocationEntries.push_back(entry);
  }

  int32_t countFileAllocationEntryInQueue() const
  {
    return _fileAllocationEntries.size();
  }
};

typedef SharedHandle<FileAllocationMan> FileAllocationManHandle;

#endif // _D_FILE_ALLOCATION_MAN_H_
