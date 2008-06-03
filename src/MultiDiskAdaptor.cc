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
#include "MultiDiskAdaptor.h"
#include "DefaultDiskWriter.h"
#include "message.h"
#include "Util.h"
#include "FileEntry.h"
#include "MultiFileAllocationIterator.h"
#include "DefaultDiskWriterFactory.h"
#include "DlAbortEx.h"
#include "File.h"
#include "StringFormat.h"
#include <algorithm>

namespace aria2 {

DiskWriterEntry::DiskWriterEntry(const SharedHandle<FileEntry>& fileEntry):
  fileEntry(fileEntry) {}

DiskWriterEntry::~DiskWriterEntry() {}

std::string DiskWriterEntry::getFilePath(const std::string& topDir) const
{
  return topDir+"/"+fileEntry->getPath();
}

void DiskWriterEntry::initAndOpenFile(const std::string& topDir)
{
  diskWriter->initAndOpenFile(getFilePath(topDir), fileEntry->getLength());
}

void DiskWriterEntry::openFile(const std::string& topDir)
{
  diskWriter->openFile(getFilePath(topDir), fileEntry->getLength());
}

void DiskWriterEntry::openExistingFile(const std::string& topDir)
{
  diskWriter->openExistingFile(getFilePath(topDir), fileEntry->getLength());
}

void DiskWriterEntry::closeFile()
{
  diskWriter->closeFile();
}

bool DiskWriterEntry::fileExists(const std::string& topDir)
{
  return File(getFilePath(topDir)).exists();
}

uint64_t DiskWriterEntry::size() const
{
  return diskWriter->size();
}

SharedHandle<FileEntry> DiskWriterEntry::getFileEntry() const
{
  return fileEntry;
}

void DiskWriterEntry::setDiskWriter(const SharedHandle<DiskWriter>& diskWriter)
{
  this->diskWriter = diskWriter;
}

SharedHandle<DiskWriter> DiskWriterEntry::getDiskWriter() const
{
  return diskWriter;
}

bool DiskWriterEntry::operator<(const DiskWriterEntry& entry) const
{
  return fileEntry < entry.fileEntry;
}

MultiDiskAdaptor::MultiDiskAdaptor():
  pieceLength(0) {}

MultiDiskAdaptor::~MultiDiskAdaptor() {}

void MultiDiskAdaptor::resetDiskWriterEntries()
{
  diskWriterEntries.clear();
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    DiskWriterEntryHandle entry(new DiskWriterEntry(*itr));
    if((*itr)->isRequested()) {
      entry->setDiskWriter(DefaultDiskWriterFactory().newDiskWriter());
    } else {
      SharedHandle<DiskWriter> dw(new DefaultDiskWriter());
      entry->setDiskWriter(dw);
    }
    entry->getDiskWriter()->setDirectIOAllowed(_directIOAllowed);
    diskWriterEntries.push_back(entry);
  }
}

std::string MultiDiskAdaptor::getTopDirPath() const
{
  return storeDir+"/"+topDir;
}

void MultiDiskAdaptor::mkdir(const std::string& topDirPath) const
{
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    (*itr)->setupDir(topDirPath);
  }
}

void MultiDiskAdaptor::openFile()
{
  const std::string topDirPath = getTopDirPath();
  mkdir(topDirPath);
  resetDiskWriterEntries();
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); ++itr) {
    (*itr)->openFile(topDirPath);
  }
}

void MultiDiskAdaptor::initAndOpenFile()
{
  const std::string topDirPath = getTopDirPath();
  mkdir(topDirPath);
  resetDiskWriterEntries();
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); ++itr) {
    (*itr)->initAndOpenFile(topDirPath);
  }
}

void MultiDiskAdaptor::openExistingFile()
{
  const std::string topDirPath = getTopDirPath();
  resetDiskWriterEntries();
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); ++itr) {
    (*itr)->openExistingFile(topDirPath);
  }
}

void MultiDiskAdaptor::closeFile()
{
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); ++itr) {
    (*itr)->closeFile();
  }
}

void MultiDiskAdaptor::onDownloadComplete()
{
  closeFile();
  openFile();
}

static bool isInRange(const DiskWriterEntryHandle entry, off_t offset)
{
  return entry->getFileEntry()->getOffset() <= offset &&
    (uint64_t)offset < entry->getFileEntry()->getOffset()+entry->getFileEntry()->getLength();
}

static size_t calculateLength(const DiskWriterEntryHandle entry,
			      off_t fileOffset, size_t rem)
{
  size_t length;
  if(entry->getFileEntry()->getLength() < (uint64_t)fileOffset+rem) {
    length = entry->getFileEntry()->getLength()-fileOffset;
  } else {
    length = rem;
  }
  return length;
}

class OffsetCompare {
public:
  bool operator()(off_t offset, const SharedHandle<DiskWriterEntry>& dwe)
  {
    return offset < dwe->getFileEntry()->getOffset();
  }
};

static DiskWriterEntries::const_iterator
findFirstDiskWriterEntry(const DiskWriterEntries& diskWriterEntries, off_t offset)
{
  DiskWriterEntries::const_iterator first =
    std::upper_bound(diskWriterEntries.begin(), diskWriterEntries.end(),
		     offset, OffsetCompare());

  --first;

  // In case when offset is out-of-range
  if(!isInRange(*first, offset)) {
    throw DlAbortEx
      (StringFormat(EX_FILE_OFFSET_OUT_OF_RANGE,
		    Util::itos(offset, true).c_str()).str());
  }
  return first;
}

void MultiDiskAdaptor::writeData(const unsigned char* data, size_t len,
				 off_t offset)
{
  DiskWriterEntries::const_iterator first = findFirstDiskWriterEntry(diskWriterEntries, offset);

  size_t rem = len;
  off_t fileOffset = offset-(*first)->getFileEntry()->getOffset();
  for(DiskWriterEntries::const_iterator i = first; i != diskWriterEntries.end(); ++i) {
    size_t writeLength = calculateLength(*i, fileOffset, rem);
    (*i)->getDiskWriter()->writeData(data+(len-rem), writeLength, fileOffset);
    rem -= writeLength;
    fileOffset = 0;
    if(rem == 0) {
      break;
    }
  }
}

ssize_t MultiDiskAdaptor::readData(unsigned char* data, size_t len, off_t offset)
{
  DiskWriterEntries::const_iterator first = findFirstDiskWriterEntry(diskWriterEntries, offset);

  size_t rem = len;
  size_t totalReadLength = 0;
  off_t fileOffset = offset-(*first)->getFileEntry()->getOffset();
  for(DiskWriterEntries::const_iterator i = first; i != diskWriterEntries.end(); ++i) {
    size_t readLength = calculateLength(*i, fileOffset, rem);
    totalReadLength +=
      (*i)->getDiskWriter()->readData(data+(len-rem), readLength, fileOffset);
    rem -= readLength;
    fileOffset = 0;
    if(rem == 0) {
      break;
    }
  }
  return totalReadLength;
}

bool MultiDiskAdaptor::fileExists()
{
  if(diskWriterEntries.empty()) {
    resetDiskWriterEntries();
  }
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    if((*itr)->fileExists(getTopDirPath())) {
      return true;
    }				   
  }
  return false;
}

// TODO call DiskWriter::openFile() before calling this function.
uint64_t MultiDiskAdaptor::size() const
{
  uint64_t size = 0;
  for(DiskWriterEntries::const_iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    size += (*itr)->size();
  }
  return size;
}

FileAllocationIteratorHandle MultiDiskAdaptor::fileAllocationIterator()
{
  return SharedHandle<FileAllocationIterator>(new MultiFileAllocationIterator(this));
}

void MultiDiskAdaptor::enableDirectIO()
{
  for(DiskWriterEntries::const_iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); ++itr) {
    (*itr)->getDiskWriter()->enableDirectIO();
  }
}

void MultiDiskAdaptor::disableDirectIO()
{
  for(DiskWriterEntries::const_iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); ++itr) {
    (*itr)->getDiskWriter()->disableDirectIO();
  }
}

} // namespace aria2
