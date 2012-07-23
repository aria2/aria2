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
#include "fmt.h"
#include "Logger.h"
#include "LogFactory.h"
#include "SimpleRandomizer.h"

namespace aria2 {

DiskWriterEntry::DiskWriterEntry(const SharedHandle<FileEntry>& fileEntry)
  : fileEntry_(fileEntry),
    open_(false),
    needsFileAllocation_(false)
{}

const std::string& DiskWriterEntry::getFilePath() const
{
  return fileEntry_->getPath();
}

void DiskWriterEntry::initAndOpenFile()
{
  if(diskWriter_) {
    diskWriter_->initAndOpenFile(fileEntry_->getLength());
    open_ = true;
  }
}

void DiskWriterEntry::openFile()
{
  if(diskWriter_) {
    diskWriter_->openFile(fileEntry_->getLength());
    open_ = true;
  }
}

void DiskWriterEntry::openExistingFile()
{
  if(diskWriter_) {
    diskWriter_->openExistingFile(fileEntry_->getLength());
    open_ = true;
  }
}

void DiskWriterEntry::closeFile()
{
  if(open_) {
    diskWriter_->closeFile();
    open_ = false;
  }
}

bool DiskWriterEntry::fileExists()
{
  return fileEntry_->exists();
}

int64_t DiskWriterEntry::size() const
{
  return File(getFilePath()).size();
}

void DiskWriterEntry::setDiskWriter(const SharedHandle<DiskWriter>& diskWriter)
{
  diskWriter_ = diskWriter;
}

bool DiskWriterEntry::operator<(const DiskWriterEntry& entry) const
{
  return *fileEntry_ < *entry.fileEntry_;
}

MultiDiskAdaptor::MultiDiskAdaptor()
  : pieceLength_(0),
    maxOpenFiles_(DEFAULT_MAX_OPEN_FILES),
    readOnly_(false),
    enableMmap_(false)
{}

MultiDiskAdaptor::~MultiDiskAdaptor() {}

namespace {
SharedHandle<DiskWriterEntry> createDiskWriterEntry
(const SharedHandle<FileEntry>& fileEntry,
 bool needsFileAllocation)
{
  SharedHandle<DiskWriterEntry> entry(new DiskWriterEntry(fileEntry));
  entry->needsFileAllocation(needsFileAllocation);
  return entry;
} 
} // namespace

void MultiDiskAdaptor::resetDiskWriterEntries()
{
  diskWriterEntries_.clear();

  if(getFileEntries().empty()) {
    return;
  }

  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        getFileEntries().begin(), eoi = getFileEntries().end(); i != eoi; ++i) {
    diskWriterEntries_.push_back
      (createDiskWriterEntry(*i, (*i)->isRequested()));
  }
  std::map<std::string, bool> dwreq;
  // TODO Currently, pieceLength_ == 0 is used for unit testing only.
  if(pieceLength_ > 0) {
    std::vector<SharedHandle<DiskWriterEntry> >::const_iterator done =
      diskWriterEntries_.begin();
    for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator itr =
          diskWriterEntries_.begin(), eoi = diskWriterEntries_.end();
        itr != eoi;) {
      const SharedHandle<FileEntry>& fileEntry = (*itr)->getFileEntry();

      if(!fileEntry->isRequested()) {
        ++itr;
        continue;
      }
      int64_t pieceStartOffset =
        (fileEntry->getOffset()/pieceLength_)*pieceLength_;
      if(itr != diskWriterEntries_.begin()) {
        for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator i =
              itr-1; true; --i) {
          const SharedHandle<FileEntry>& fileEntry = (*i)->getFileEntry();
          if(pieceStartOffset <= fileEntry->getOffset() ||
             pieceStartOffset < fileEntry->getLastOffset()) {
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
        int64_t lastPieceStartOffset =
          (fileEntry->getOffset()+fileEntry->getLength()-1)/
          pieceLength_*pieceLength_;
        A2_LOG_DEBUG(fmt("Checking adjacent backward file to %s"
                         " whose lastPieceStartOffset+pieceLength_=%" PRId64 "",
                         fileEntry->getPath().c_str(),
                         static_cast<int64_t>
                         (lastPieceStartOffset+pieceLength_)));
        ++itr;
        // adjacent backward files are not needed to be allocated. They
        // just requre DiskWriter
        for(; itr != eoi &&
              (!(*itr)->getFileEntry()->isRequested() ||
               (*itr)->getFileEntry()->getLength() == 0); ++itr) {
          A2_LOG_DEBUG
            (fmt("file=%s, offset=%" PRId64 "",
                 (*itr)->getFileEntry()->getPath().c_str(),
                 (*itr)->getFileEntry()->getOffset()));
          if((*itr)->getFileEntry()->getOffset() <
             lastPieceStartOffset+pieceLength_) {
            A2_LOG_DEBUG
              (fmt("%s needs diskwriter",
                   (*itr)->getFileEntry()->getPath().c_str()));
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
        diskWriterEntries_.begin(), eoi = diskWriterEntries_.end();
      i != eoi; ++i) {
    if((*i)->needsFileAllocation() ||
       dwreq.find((*i)->getFileEntry()->getPath()) != dwreq.end() ||
       (*i)->fileExists()) {
      A2_LOG_DEBUG(fmt("Creating DiskWriter for filename=%s",
                       (*i)->getFilePath().c_str()));
      (*i)->setDiskWriter(dwFactory.newDiskWriter((*i)->getFilePath()));
      if(readOnly_) {
        (*i)->getDiskWriter()->enableReadOnly();
      }
      if(enableMmap_) {
        (*i)->getDiskWriter()->enableMmap();
      }
    }
  }
}

void MultiDiskAdaptor::openIfNot
(const SharedHandle<DiskWriterEntry>& entry, void (DiskWriterEntry::*open)())
{
  if(!entry->isOpen()) {
    //     A2_LOG_DEBUG(fmt("DiskWriterEntry: Cache MISS. offset=%s",
    //            util::itos(entry->getFileEntry()->getOffset()).c_str()));
 
    int numOpened = openedDiskWriterEntries_.size();
    (entry.get()->*open)();
    if(numOpened >= maxOpenFiles_) {
      // Cache is full. 
      // Choose one DiskWriterEntry randomly and close it.
      size_t index =
        SimpleRandomizer::getInstance()->getRandomNumber(numOpened);
      std::vector<SharedHandle<DiskWriterEntry> >::iterator i =
        openedDiskWriterEntries_.begin();
      std::advance(i, index);
      (*i)->closeFile();
      (*i) = entry;
    } else {
      openedDiskWriterEntries_.push_back(entry);
    } 
  } else {
    //     A2_LOG_DEBUG(fmt("DiskWriterEntry: Cache HIT. offset=%s",
    //            util::itos(entry->getFileEntry()->getOffset()).c_str()));
  }
}

void MultiDiskAdaptor::openFile()
{
  resetDiskWriterEntries();
  // util::mkdir() is called in AbstractDiskWriter::createFile(), so
  // we don't need to call it here.

  // Call DiskWriterEntry::openFile to make sure that zero-length files are
  // created.
  for(DiskWriterEntries::const_iterator itr = diskWriterEntries_.begin(),
        eoi = diskWriterEntries_.end(); itr != eoi; ++itr) {
    openIfNot(*itr, &DiskWriterEntry::openFile);
  }
}

void MultiDiskAdaptor::initAndOpenFile()
{
  resetDiskWriterEntries();
  // util::mkdir() is called in AbstractDiskWriter::createFile(), so
  // we don't need to call it here.

  // Call DiskWriterEntry::initAndOpenFile to make files truncated.
  for(DiskWriterEntries::const_iterator itr = diskWriterEntries_.begin(),
        eoi = diskWriterEntries_.end(); itr != eoi; ++itr) {
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
  std::for_each(diskWriterEntries_.begin(), diskWriterEntries_.end(),
                mem_fun_sh(&DiskWriterEntry::closeFile));
}

namespace {
bool isInRange(const DiskWriterEntryHandle entry, int64_t offset)
{
  return entry->getFileEntry()->getOffset() <= offset &&
    offset < entry->getFileEntry()->getLastOffset();
}
} // namespace

namespace {
ssize_t calculateLength(const DiskWriterEntryHandle entry,
                        int64_t fileOffset, ssize_t rem)
{
  if(entry->getFileEntry()->getLength() < fileOffset+rem) {
    return entry->getFileEntry()->getLength()-fileOffset;
  } else {
    return rem;
  }
}
} // namespace

namespace {
class OffsetCompare {
public:
  bool operator()(int64_t offset, const SharedHandle<DiskWriterEntry>& dwe)
  {
    return offset < dwe->getFileEntry()->getOffset();
  }
};
} // namespace

namespace {
DiskWriterEntries::const_iterator findFirstDiskWriterEntry
(const DiskWriterEntries& diskWriterEntries, int64_t offset)
{
  DiskWriterEntries::const_iterator first =
    std::upper_bound(diskWriterEntries.begin(), diskWriterEntries.end(),
                     offset, OffsetCompare());

  --first;

  // In case when offset is out-of-range
  if(!isInRange(*first, offset)) {
    throw DL_ABORT_EX
      (fmt(EX_FILE_OFFSET_OUT_OF_RANGE, static_cast<int64_t>(offset)));
  }
  return first;
}
} // namespace

namespace {
void throwOnDiskWriterNotOpened(const SharedHandle<DiskWriterEntry>& e,
                                int64_t offset)
{
  throw DL_ABORT_EX
    (fmt("DiskWriter for offset=%" PRId64 ", filename=%s is not opened.",
         static_cast<int64_t>(offset),
         e->getFilePath().c_str()));  
}
} // namespace

void MultiDiskAdaptor::writeData(const unsigned char* data, size_t len,
                                 int64_t offset)
{
  DiskWriterEntries::const_iterator first =
    findFirstDiskWriterEntry(diskWriterEntries_, offset);

  ssize_t rem = len;
  int64_t fileOffset = offset-(*first)->getFileEntry()->getOffset();
  for(DiskWriterEntries::const_iterator i = first,
        eoi = diskWriterEntries_.end(); i != eoi; ++i) {
    ssize_t writeLength = calculateLength(*i, fileOffset, rem);

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
(unsigned char* data, size_t len, int64_t offset)
{
  DiskWriterEntries::const_iterator first =
    findFirstDiskWriterEntry(diskWriterEntries_, offset);

  ssize_t rem = len;
  ssize_t totalReadLength = 0;
  int64_t fileOffset = offset-(*first)->getFileEntry()->getOffset();
  for(DiskWriterEntries::const_iterator i = first,
        eoi = diskWriterEntries_.end(); i != eoi; ++i) {
    ssize_t readLength = calculateLength(*i, fileOffset, rem);

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

int64_t MultiDiskAdaptor::size()
{
  int64_t size = 0;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        getFileEntries().begin(), eoi = getFileEntries().end(); i != eoi; ++i) {
    size += File((*i)->getPath()).size();
  }
  return size;
}

SharedHandle<FileAllocationIterator> MultiDiskAdaptor::fileAllocationIterator()
{
  return SharedHandle<FileAllocationIterator>
    (new MultiFileAllocationIterator(this));
}

void MultiDiskAdaptor::enableReadOnly()
{
  readOnly_ = true;
}

void MultiDiskAdaptor::disableReadOnly()
{
  readOnly_ = false;
}

void MultiDiskAdaptor::enableMmap()
{
  enableMmap_ = true;
}

void MultiDiskAdaptor::cutTrailingGarbage()
{
  for(std::vector<SharedHandle<DiskWriterEntry> >::const_iterator i =
        diskWriterEntries_.begin(), eoi = diskWriterEntries_.end();
      i != eoi; ++i) {
    int64_t length = (*i)->getFileEntry()->getLength();
    if(File((*i)->getFilePath()).size() > length) {
      // We need open file before calling DiskWriter::truncate(int64_t)
      openIfNot(*i, &DiskWriterEntry::openFile);
      (*i)->getDiskWriter()->truncate(length);
    }
  }
}

void MultiDiskAdaptor::setMaxOpenFiles(int maxOpenFiles)
{
  maxOpenFiles_ = maxOpenFiles;
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
