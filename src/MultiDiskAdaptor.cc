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
#include "MultiDiskAdaptor.h"

#include <cassert>
#include <algorithm>
#include <map>

#include "DefaultDiskWriter.h"
#include "message.h"
#include "util.h"
#include "FileEntry.h"
#include "MultiFileAllocationIterator.h"
#include "DefaultDiskWriterFactory.h"
#include "DlAbortEx.h"
#include "File.h"
#include "StringFormat.h"
#include "Logger.h"
#include "SimpleRandomizer.h"

namespace aria2 {

DiskWriterEntry::DiskWriterEntry(const SharedHandle<FileEntry>& fileEntry):
  _fileEntry(fileEntry), _open(false), _directIO(false),
  _needsFileAllocation(false) {}

const std::string& DiskWriterEntry::getFilePath() const
{
  return _fileEntry->getPath();
}

void DiskWriterEntry::initAndOpenFile()
{
  if(!_diskWriter.isNull()) {
    _diskWriter->initAndOpenFile(_fileEntry->getLength());
    if(_directIO) {
      _diskWriter->enableDirectIO();
    }
    _open = true;
  }
}

void DiskWriterEntry::openFile()
{
  if(!_diskWriter.isNull()) {
    _diskWriter->openFile(_fileEntry->getLength());
    if(_directIO) {
      _diskWriter->enableDirectIO();
    }
    _open = true;
  }
}

void DiskWriterEntry::openExistingFile()
{
  if(!_diskWriter.isNull()) {
    _diskWriter->openExistingFile(_fileEntry->getLength());
    if(_directIO) {
      _diskWriter->enableDirectIO();
    }
    _open = true;
  }
}

void DiskWriterEntry::closeFile()
{
  if(_open) {
    _diskWriter->closeFile();
    _open = false;
  }
}

bool DiskWriterEntry::fileExists()
{
  return _fileEntry->exists();
}

uint64_t DiskWriterEntry::size() const
{
  return File(getFilePath()).size();
}

void DiskWriterEntry::setDiskWriter(const SharedHandle<DiskWriter>& diskWriter)
{
  _diskWriter = diskWriter;
}

bool DiskWriterEntry::operator<(const DiskWriterEntry& entry) const
{
  return _fileEntry < entry._fileEntry;
}

void DiskWriterEntry::enableDirectIO()
{
  if(_open) {
    _diskWriter->enableDirectIO();
  }
  _directIO = true;
}

void DiskWriterEntry::disableDirectIO()
{
  if(_open) {
    _diskWriter->disableDirectIO();
  }
  _directIO = false;
}

MultiDiskAdaptor::MultiDiskAdaptor():
  _pieceLength(0),
  _maxOpenFiles(DEFAULT_MAX_OPEN_FILES),
  _directIOAllowed(false),
  _readOnly(false) {}

static SharedHandle<DiskWriterEntry> createDiskWriterEntry
(const SharedHandle<FileEntry>& fileEntry,
 bool needsFileAllocation)
{
  SharedHandle<DiskWriterEntry> entry(new DiskWriterEntry(fileEntry));
  entry->needsFileAllocation(needsFileAllocation);
  return entry;
} 

void MultiDiskAdaptor::resetDiskWriterEntries()
{
  _diskWriterEntries.clear();

  if(getFileEntries().empty()) {
    return;
  }

  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        getFileEntries().begin(), eoi = getFileEntries().end(); i != eoi; ++i) {
    _diskWriterEntries.push_back
      (createDiskWriterEntry(*i, (*i)->isRequested()));
  }
  std::map<std::string, bool> dwreq;
  // TODO Currently, _pieceLength == 0 is used for unit testing only.
  if(_pieceLength > 0) {
    std::vector<SharedHandle<DiskWriterEntry> >::const_iterator done =
      _diskWriterEntries.begin();
    for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator itr =
          _diskWriterEntries.begin(), eoi = _diskWriterEntries.end();
        itr != eoi;) {
      const SharedHandle<FileEntry>& fileEntry = (*itr)->getFileEntry();

      if(!fileEntry->isRequested()) {
        ++itr;
        continue;
      }
      off_t pieceStartOffset =
        (fileEntry->getOffset()/_pieceLength)*_pieceLength;
      if(itr != _diskWriterEntries.begin()) {
        for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator i =
              itr-1; true; --i) {
          const SharedHandle<FileEntry>& fileEntry = (*i)->getFileEntry();
          if(pieceStartOffset <= fileEntry->getOffset() ||
             (uint64_t)pieceStartOffset <
             fileEntry->getOffset()+fileEntry->getLength()) {
            (*i)->needsFileAllocation(true);
          } else {
            break;
          }
          if(i == done) {
            break;
          }
        }
      }

      if(fileEntry->getLength() > 0) {
        off_t lastPieceStartOffset =
          (fileEntry->getOffset()+fileEntry->getLength()-1)/
          _pieceLength*_pieceLength;
        if(getLogger()->debug()) {
          getLogger()->debug("Checking adjacent backward file to %s"
                             " whose lastPieceStartOffset+_pieceLength=%lld",
                             fileEntry->getPath().c_str(),
                             lastPieceStartOffset+_pieceLength);
        }
        ++itr;
        // adjacent backward files are not needed to be allocated. They
        // just requre DiskWriter
        for(; itr != eoi &&
              (!(*itr)->getFileEntry()->isRequested() ||
               (*itr)->getFileEntry()->getLength() == 0); ++itr) {
          if(getLogger()->debug()) {
            getLogger()->debug("file=%s, offset=%lld",
                               (*itr)->getFileEntry()->getPath().c_str(),
                               (*itr)->getFileEntry()->getOffset());
          }
          if((*itr)->getFileEntry()->getOffset() <
             static_cast<off_t>(lastPieceStartOffset+_pieceLength)) {
            if(getLogger()->debug()) {
              getLogger()->debug("%s needs diskwriter",
                                 (*itr)->getFileEntry()->getPath().c_str());
            }
            dwreq[(*itr)->getFileEntry()->getPath()] = true;
          } else {
            break;
          }
        }
        done = itr-1;
      } else {
        done = itr;
        ++itr;
      }
    }
  }
  DefaultDiskWriterFactory dwFactory;
  for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator i =
        _diskWriterEntries.begin(), eoi = _diskWriterEntries.end();
      i != eoi; ++i) {
    if((*i)->needsFileAllocation() ||
       dwreq.find((*i)->getFileEntry()->getPath()) != dwreq.end() ||
       (*i)->fileExists()) {
      if(getLogger()->debug()) {
        getLogger()->debug("Creating DiskWriter for filename=%s",
                           (*i)->getFilePath().c_str());
      }
      (*i)->setDiskWriter(dwFactory.newDiskWriter((*i)->getFilePath()));
      if(_directIOAllowed) {
        (*i)->getDiskWriter()->allowDirectIO();
      }
      if(_readOnly) {
        (*i)->getDiskWriter()->enableReadOnly();
      }
    }
  }
}

void MultiDiskAdaptor::mkdir() const
{
  for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator i =
        _diskWriterEntries.begin(), eoi = _diskWriterEntries.end();
      i != eoi; ++i) {
    (*i)->getFileEntry()->setupDir();
  }
}

void MultiDiskAdaptor::openIfNot
(const SharedHandle<DiskWriterEntry>& entry, void (DiskWriterEntry::*open)())
{
  if(!entry->isOpen()) {
    //     getLogger()->debug("DiskWriterEntry: Cache MISS. offset=%s",
    //            util::itos(entry->getFileEntry()->getOffset()).c_str());
 
    size_t numOpened = _openedDiskWriterEntries.size();
    (entry.get()->*open)();
    if(numOpened >= _maxOpenFiles) {
      // Cache is full. 
      // Choose one DiskWriterEntry randomly and close it.
      size_t index =
        SimpleRandomizer::getInstance()->getRandomNumber(numOpened);
      std::vector<SharedHandle<DiskWriterEntry> >::iterator i =
        _openedDiskWriterEntries.begin();
      std::advance(i, index);
      (*i)->closeFile();
      (*i) = entry;
    } else {
      _openedDiskWriterEntries.push_back(entry);
    } 
  } else {
    //     getLogger()->debug("DiskWriterEntry: Cache HIT. offset=%s",
    //            util::itos(entry->getFileEntry()->getOffset()).c_str());
  }
}

void MultiDiskAdaptor::openFile()
{
  resetDiskWriterEntries();
  mkdir();
  // Call DiskWriterEntry::openFile to make sure that zero-length files are
  // created.
  for(DiskWriterEntries::const_iterator itr = _diskWriterEntries.begin(),
        eoi = _diskWriterEntries.end(); itr != eoi; ++itr) {
    openIfNot(*itr, &DiskWriterEntry::openFile);
  }
}

void MultiDiskAdaptor::initAndOpenFile()
{
  resetDiskWriterEntries();
  mkdir();
  // Call DiskWriterEntry::initAndOpenFile to make files truncated.
  for(DiskWriterEntries::const_iterator itr = _diskWriterEntries.begin(),
        eoi = _diskWriterEntries.end(); itr != eoi; ++itr) {
    openIfNot(*itr, &DiskWriterEntry::initAndOpenFile);
  }
}

void MultiDiskAdaptor::openExistingFile()
{
  resetDiskWriterEntries();
  // Not need to call openIfNot here.
}

void MultiDiskAdaptor::closeFile()
{
  std::for_each(_diskWriterEntries.begin(), _diskWriterEntries.end(),
                mem_fun_sh(&DiskWriterEntry::closeFile));
}

static bool isInRange(const DiskWriterEntryHandle entry, off_t offset)
{
  return entry->getFileEntry()->getOffset() <= offset &&
    (uint64_t)offset <
    entry->getFileEntry()->getOffset()+entry->getFileEntry()->getLength();
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
findFirstDiskWriterEntry
(const DiskWriterEntries& diskWriterEntries, off_t offset)
{
  DiskWriterEntries::const_iterator first =
    std::upper_bound(diskWriterEntries.begin(), diskWriterEntries.end(),
                     offset, OffsetCompare());

  --first;

  // In case when offset is out-of-range
  if(!isInRange(*first, offset)) {
    throw DL_ABORT_EX
      (StringFormat(EX_FILE_OFFSET_OUT_OF_RANGE,
                    util::itos(offset, true).c_str()).str());
  }
  return first;
}

static void throwOnDiskWriterNotOpened(const SharedHandle<DiskWriterEntry>& e,
                                       off_t offset)
{
  throw DL_ABORT_EX
    (StringFormat("DiskWriter for offset=%s, filename=%s is not opened.",
                  util::itos(offset).c_str(),
                  e->getFilePath().c_str()).str());  
}

void MultiDiskAdaptor::writeData(const unsigned char* data, size_t len,
                                 off_t offset)
{
  DiskWriterEntries::const_iterator first =
    findFirstDiskWriterEntry(_diskWriterEntries, offset);

  size_t rem = len;
  off_t fileOffset = offset-(*first)->getFileEntry()->getOffset();
  for(DiskWriterEntries::const_iterator i = first,
        eoi = _diskWriterEntries.end(); i != eoi; ++i) {
    size_t writeLength = calculateLength(*i, fileOffset, rem);

    openIfNot(*i, &DiskWriterEntry::openFile);

    if(!(*i)->isOpen()) {
      throwOnDiskWriterNotOpened(*i, offset+(len-rem));
    }

    (*i)->getDiskWriter()->writeData(data+(len-rem), writeLength, fileOffset);
    rem -= writeLength;
    fileOffset = 0;
    if(rem == 0) {
      break;
    }
  }
}

ssize_t MultiDiskAdaptor::readData
(unsigned char* data, size_t len, off_t offset)
{
  DiskWriterEntries::const_iterator first =
    findFirstDiskWriterEntry(_diskWriterEntries, offset);

  size_t rem = len;
  size_t totalReadLength = 0;
  off_t fileOffset = offset-(*first)->getFileEntry()->getOffset();
  for(DiskWriterEntries::const_iterator i = first,
        eoi = _diskWriterEntries.end(); i != eoi; ++i) {
    size_t readLength = calculateLength(*i, fileOffset, rem);

    openIfNot(*i, &DiskWriterEntry::openFile);

    if(!(*i)->isOpen()) {
      throwOnDiskWriterNotOpened(*i, offset+(len-rem));
    }

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
  return std::find_if(getFileEntries().begin(), getFileEntries().end(),
                      mem_fun_sh(&FileEntry::exists)) !=
    getFileEntries().end();
}

uint64_t MultiDiskAdaptor::size()
{
  uint64_t size = 0;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        getFileEntries().begin(), eoi = getFileEntries().end(); i != eoi; ++i) {
    size += File((*i)->getPath()).size();
  }
  return size;
}

FileAllocationIteratorHandle MultiDiskAdaptor::fileAllocationIterator()
{
  return SharedHandle<FileAllocationIterator>
    (new MultiFileAllocationIterator(this));
}

void MultiDiskAdaptor::enableDirectIO()
{
  std::for_each(_diskWriterEntries.begin(), _diskWriterEntries.end(),
                mem_fun_sh(&DiskWriterEntry::enableDirectIO));
}

void MultiDiskAdaptor::disableDirectIO()
{
  std::for_each(_diskWriterEntries.begin(), _diskWriterEntries.end(),
                mem_fun_sh(&DiskWriterEntry::disableDirectIO));
}

void MultiDiskAdaptor::enableReadOnly()
{
  _readOnly = true;
}

void MultiDiskAdaptor::disableReadOnly()
{
  _readOnly = false;
}

void MultiDiskAdaptor::cutTrailingGarbage()
{
  for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator i =
        _diskWriterEntries.begin(), eoi = _diskWriterEntries.end();
      i != eoi; ++i) {
    uint64_t length = (*i)->getFileEntry()->getLength();
    if(File((*i)->getFilePath()).size() > length) {
      // We need open file before calling DiskWriter::truncate(uint64_t)
      openIfNot(*i, &DiskWriterEntry::openFile);
      (*i)->getDiskWriter()->truncate(length);
    }
  }
}

void MultiDiskAdaptor::setMaxOpenFiles(size_t maxOpenFiles)
{
  _maxOpenFiles = maxOpenFiles;
}

size_t MultiDiskAdaptor::utime(const Time& actime, const Time& modtime)
{
  size_t numOK = 0;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        getFileEntries().begin(), eoi = getFileEntries().end(); i != eoi; ++i) {
    if((*i)->isRequested()) {
      File f((*i)->getPath());
      if(f.isFile() && f.utime(actime, modtime)) {
        ++numOK;
      }
    }
  }
  return numOK;
}

} // namespace aria2
