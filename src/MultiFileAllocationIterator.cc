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
#include "MultiFileAllocationIterator.h"
#include "MultiDiskAdaptor.h"
#include "FileEntry.h"

#define BUFSIZE 16*1024

MultiFileAllocationIterator::MultiFileAllocationIterator(MultiDiskAdaptor* diskAdaptor):
  _diskAdaptor(diskAdaptor),
  _entries(makeFileEntries(diskAdaptor->getFileEntries(), diskAdaptor->getPieceLength())),
  _currentEntry(0),
  _offset(0)
{}

MultiFileAllocationIterator::~MultiFileAllocationIterator() {}

void MultiFileAllocationIterator::prepareNextEntry()
{
  _currentEntry = 0;
  _offset = 0;
  if(!_entries.empty()) {
    FileEntryHandle entry = _entries.front();
    _entries.pop_front();

    _currentEntry = entry;
    _offset = File(_diskAdaptor->getStoreDir()+"/"+
		   _diskAdaptor->getTopDir()+"/"+
		   _currentEntry->getPath()).size();
  }
}


void MultiFileAllocationIterator::allocateChunk()
{
  while(_currentEntry.isNull() || _currentEntry->getLength() <= _offset) {
    prepareNextEntry();
    if(_currentEntry.isNull()) {
      break;
    }
  }
  if(finished()) {
    return;
  }
  int32_t bufSize = BUFSIZE;
  unsigned char buf[BUFSIZE];
  memset(buf, 0, bufSize);
  
  int32_t wsize = _offset+bufSize > _currentEntry->getLength() ?
    _currentEntry->getLength()-_offset:bufSize;
  _diskAdaptor->writeData(buf, wsize, _offset+_currentEntry->getOffset());
  _offset += wsize;  
}

bool MultiFileAllocationIterator::finished()
{
  return _entries.empty() && _currentEntry.isNull();
}

int64_t MultiFileAllocationIterator::getTotalLength()
{
  if(_currentEntry.isNull()) {
    return 0;
  } else {
    return _currentEntry->getLength();
  }
}

const FileEntries& MultiFileAllocationIterator::getFileEntries() const
{
  return _entries;
}

FileEntries MultiFileAllocationIterator::makeFileEntries(const FileEntries& srcEntries, int32_t pieceLength) const
{
  if(pieceLength == 0) {
    FileEntries entries;
    for(FileEntries::const_iterator itr = srcEntries.begin(); itr != srcEntries.end(); ++itr) {
      if((*itr)->isRequested()) {
	entries.push_back(*itr);
      }
    }
    return entries;
  }
  FileEntries temp(srcEntries);
  temp.push_front(new FileEntry());
  FileEntries entries;
  FileEntries::const_iterator done = temp.begin();
  for(FileEntries::const_iterator itr = temp.begin()+1; itr != temp.end(); ++itr) {
    if(!(*itr)->isRequested()) {
      continue;
    }
    int64_t pieceStartOffset = ((*itr)->getOffset()/pieceLength)*pieceLength;
    for(FileEntries::const_iterator i = itr-1; i != done; --i) {
      if(pieceStartOffset < (*i)->getOffset()+(*i)->getLength()) {
	entries.push_back(*i);
      } else {
	break;
      }
    }
    entries.push_back(*itr);
    done = itr;
  }
  return entries;
}
