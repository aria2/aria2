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
#include "WrDiskCacheEntry.h"
#include "OpenedFileCounter.h"

namespace aria2 {

DiskWriterEntry::DiskWriterEntry(const std::shared_ptr<FileEntry>& fileEntry)
    : fileEntry_{fileEntry},
      open_{false},
      needsFileAllocation_{false},
      needsDiskWriter_{false}
{
}

const std::string& DiskWriterEntry::getFilePath() const
{
  return fileEntry_->getPath();
}

void DiskWriterEntry::initAndOpenFile()
{
  if (diskWriter_) {
    diskWriter_->initAndOpenFile(fileEntry_->getLength());
    open_ = true;
  }
}

void DiskWriterEntry::openFile()
{
  if (diskWriter_) {
    diskWriter_->openFile(fileEntry_->getLength());
    open_ = true;
  }
}

void DiskWriterEntry::openExistingFile()
{
  if (diskWriter_) {
    diskWriter_->openExistingFile(fileEntry_->getLength());
    open_ = true;
  }
}

void DiskWriterEntry::closeFile()
{
  if (open_) {
    diskWriter_->closeFile();
    open_ = false;
  }
}

bool DiskWriterEntry::fileExists() { return fileEntry_->exists(); }

int64_t DiskWriterEntry::size() const { return File(getFilePath()).size(); }

void DiskWriterEntry::setDiskWriter(std::unique_ptr<DiskWriter> diskWriter)
{
  diskWriter_ = std::move(diskWriter);
}

bool DiskWriterEntry::operator<(const DiskWriterEntry& entry) const
{
  return *fileEntry_ < *entry.fileEntry_;
}

MultiDiskAdaptor::MultiDiskAdaptor() : pieceLength_{0}, readOnly_{false} {}

MultiDiskAdaptor::~MultiDiskAdaptor() { closeFile(); }

namespace {
std::unique_ptr<DiskWriterEntry>
createDiskWriterEntry(const std::shared_ptr<FileEntry>& fileEntry)
{
  auto entry = make_unique<DiskWriterEntry>(fileEntry);
  entry->needsFileAllocation(fileEntry->isRequested());
  return entry;
}
} // namespace

void MultiDiskAdaptor::resetDiskWriterEntries()
{
  assert(openedDiskWriterEntries_.empty());
  diskWriterEntries_.clear();
  if (getFileEntries().empty()) {
    return;
  }
  for (auto& fileEntry : getFileEntries()) {
    diskWriterEntries_.push_back(createDiskWriterEntry(fileEntry));
  }
  // TODO Currently, pieceLength_ == 0 is used for unit testing only.
  if (pieceLength_ > 0) {
    // Check shared piece forward
    int64_t lastOffset = 0;
    for (auto& dwent : diskWriterEntries_) {
      auto& fileEntry = dwent->getFileEntry();
      if (fileEntry->isRequested()) {
        // zero length file does not affect lastOffset.
        if (fileEntry->getLength() > 0) {
          lastOffset =
              (fileEntry->getLastOffset() - 1) / pieceLength_ * pieceLength_ +
              pieceLength_;
        }
      }
      else if (fileEntry->getOffset() < lastOffset) {
        // The files which shares last piece are not needed to be
        // allocated. They just require DiskWriter
        A2_LOG_DEBUG(fmt("%s needs DiskWriter", fileEntry->getPath().c_str()));
        dwent->needsDiskWriter(true);
      }
    }
    // Check shared piece backward
    lastOffset = std::numeric_limits<int64_t>::max();
    for (auto i = diskWriterEntries_.rbegin(), eoi = diskWriterEntries_.rend();
         i != eoi; ++i) {
      auto& fileEntry = (*i)->getFileEntry();
      if (fileEntry->isRequested()) {
        lastOffset = fileEntry->getOffset() / pieceLength_ * pieceLength_;
      }
      else if (lastOffset <= fileEntry->getOffset() || // length == 0 case
               lastOffset < fileEntry->getLastOffset()) {
        // We needs last part of the file, so file allocation is
        // required, especially for file system which does not support
        // sparse files.
        A2_LOG_DEBUG(
            fmt("%s needs file allocation", fileEntry->getPath().c_str()));
        (*i)->needsFileAllocation(true);
      }
    }
  }
  DefaultDiskWriterFactory dwFactory;
  for (auto& dwent : diskWriterEntries_) {
    if (dwent->needsFileAllocation() || dwent->needsDiskWriter() ||
        dwent->fileExists()) {
      A2_LOG_DEBUG(fmt("Creating DiskWriter for filename=%s",
                       dwent->getFilePath().c_str()));
      dwent->setDiskWriter(dwFactory.newDiskWriter(dwent->getFilePath()));
      if (readOnly_) {
        dwent->getDiskWriter()->enableReadOnly();
      }
      // TODO mmap is not enabled at this moment. Call enableMmap()
      // after this function call.
    }
  }
}

size_t MultiDiskAdaptor::tryCloseFile(size_t numClose)
{
  size_t left = numClose;
  for (; !openedDiskWriterEntries_.empty() && left > 0; --left) {
    // Choose one DiskWriterEntry randomly and close it.
    size_t index = SimpleRandomizer::getInstance()->getRandomNumber(
        openedDiskWriterEntries_.size());
    auto i = std::begin(openedDiskWriterEntries_);
    std::advance(i, index);
    (*i)->closeFile();
    (*i) = openedDiskWriterEntries_.back();
    openedDiskWriterEntries_.pop_back();
  }
  return numClose - left;
}

void MultiDiskAdaptor::openIfNot(DiskWriterEntry* entry,
                                 void (DiskWriterEntry::*open)())
{
  if (!entry->isOpen()) {
    // A2_LOG_NOTICE(fmt("DiskWriterEntry: Cache MISS. offset=%s",
    //        util::itos(entry->getFileEntry()->getOffset()).c_str()));
    auto& openedFileCounter = getOpenedFileCounter();
    if (openedFileCounter) {
      openedFileCounter->ensureMaxOpenFileLimit(1);
    }
    (entry->*open)();
    openedDiskWriterEntries_.push_back(entry);
  }
  else {
    // A2_LOG_NOTICE(fmt("DiskWriterEntry: Cache HIT. offset=%s",
    //        util::itos(entry->getFileEntry()->getOffset()).c_str()));
  }
}

void MultiDiskAdaptor::openFile()
{
  resetDiskWriterEntries();
  // util::mkdir() is called in AbstractDiskWriter::createFile(), so
  // we don't need to call it here.

  // Call DiskWriterEntry::openFile to make sure that zero-length files are
  // created.
  for (auto& dwent : diskWriterEntries_) {
    openIfNot(dwent.get(), &DiskWriterEntry::openFile);
  }
}

void MultiDiskAdaptor::initAndOpenFile()
{
  resetDiskWriterEntries();
  // util::mkdir() is called in AbstractDiskWriter::createFile(), so
  // we don't need to call it here.

  // Call DiskWriterEntry::initAndOpenFile to make files truncated.
  for (auto& dwent : diskWriterEntries_) {
    openIfNot(dwent.get(), &DiskWriterEntry::initAndOpenFile);
  }
}

void MultiDiskAdaptor::openExistingFile()
{
  resetDiskWriterEntries();
  // Not need to call openIfNot here.
}

void MultiDiskAdaptor::closeFile()
{
  for (auto& dwent : openedDiskWriterEntries_) {
    auto& dw = dwent->getDiskWriter();
    // required for unit test
    if (!dw) {
      continue;
    }
    dw->closeFile();
  }
  auto& openedFileCounter = getOpenedFileCounter();
  if (openedFileCounter) {
    openedFileCounter->reduceNumOfOpenedFile(openedDiskWriterEntries_.size());
  }
  openedDiskWriterEntries_.clear();
}

namespace {
bool isInRange(DiskWriterEntry* entry, int64_t offset)
{
  return entry->getFileEntry()->getOffset() <= offset &&
         offset < entry->getFileEntry()->getLastOffset();
}
} // namespace

namespace {
ssize_t calculateLength(DiskWriterEntry* entry, int64_t fileOffset, ssize_t rem)
{
  if (entry->getFileEntry()->getLength() < fileOffset + rem) {
    return entry->getFileEntry()->getLength() - fileOffset;
  }
  else {
    return rem;
  }
}
} // namespace

namespace {
class OffsetCompare {
public:
  bool operator()(int64_t offset, const std::unique_ptr<DiskWriterEntry>& dwe)
  {
    return offset < dwe->getFileEntry()->getOffset();
  }
};
} // namespace

namespace {
DiskWriterEntries::const_iterator
findFirstDiskWriterEntry(const DiskWriterEntries& diskWriterEntries,
                         int64_t offset)
{
  auto first =
      std::upper_bound(std::begin(diskWriterEntries),
                       std::end(diskWriterEntries), offset, OffsetCompare());
  --first;
  // In case when offset is out-of-range
  if (!isInRange((*first).get(), offset)) {
    throw DL_ABORT_EX(
        fmt(EX_FILE_OFFSET_OUT_OF_RANGE, static_cast<int64_t>(offset)));
  }
  return first;
}
} // namespace

namespace {
void throwOnDiskWriterNotOpened(DiskWriterEntry* e, int64_t offset)
{
  throw DL_ABORT_EX(
      fmt("DiskWriter for offset=%" PRId64 ", filename=%s is not opened.",
          static_cast<int64_t>(offset), e->getFilePath().c_str()));
}
} // namespace

void MultiDiskAdaptor::writeData(const unsigned char* data, size_t len,
                                 int64_t offset)
{
  auto first = findFirstDiskWriterEntry(diskWriterEntries_, offset);
  ssize_t rem = len;
  int64_t fileOffset = offset - (*first)->getFileEntry()->getOffset();
  for (auto i = first, eoi = diskWriterEntries_.cend(); i != eoi; ++i) {
    ssize_t writeLength = calculateLength((*i).get(), fileOffset, rem);
    openIfNot((*i).get(), &DiskWriterEntry::openFile);
    if (!(*i)->isOpen()) {
      throwOnDiskWriterNotOpened((*i).get(), offset + (len - rem));
    }

    (*i)->getDiskWriter()->writeData(data + (len - rem), writeLength,
                                     fileOffset);
    rem -= writeLength;
    fileOffset = 0;
    if (rem == 0) {
      break;
    }
  }
}

ssize_t MultiDiskAdaptor::readData(unsigned char* data, size_t len,
                                   int64_t offset)
{
  return readData(data, len, offset, false);
}

ssize_t MultiDiskAdaptor::readDataDropCache(unsigned char* data, size_t len,
                                            int64_t offset)
{
  return readData(data, len, offset, true);
}

ssize_t MultiDiskAdaptor::readData(unsigned char* data, size_t len,
                                   int64_t offset, bool dropCache)
{
  auto first = findFirstDiskWriterEntry(diskWriterEntries_, offset);
  ssize_t rem = len;
  ssize_t totalReadLength = 0;
  int64_t fileOffset = offset - (*first)->getFileEntry()->getOffset();
  for (auto i = first, eoi = diskWriterEntries_.cend(); i != eoi; ++i) {
    ssize_t readLength = calculateLength((*i).get(), fileOffset, rem);
    openIfNot((*i).get(), &DiskWriterEntry::openFile);
    if (!(*i)->isOpen()) {
      throwOnDiskWriterNotOpened((*i).get(), offset + (len - rem));
    }

    while (readLength > 0) {
      auto nread = (*i)->getDiskWriter()->readData(data + (len - rem),
                                                   readLength, fileOffset);

      if (nread == 0) {
        return totalReadLength;
      }

      totalReadLength += nread;

      if (dropCache) {
        (*i)->getDiskWriter()->dropCache(nread, fileOffset);
      }

      readLength -= nread;
      rem -= nread;
      fileOffset += nread;
    }

    fileOffset = 0;
    if (rem == 0) {
      break;
    }
  }
  return totalReadLength;
}

void MultiDiskAdaptor::writeCache(const WrDiskCacheEntry* entry)
{
  for (auto& d : entry->getDataSet()) {
    A2_LOG_DEBUG(fmt("Cache flush goff=%" PRId64 ", len=%lu", d->goff,
                     static_cast<unsigned long>(d->len)));
    writeData(d->data + d->offset, d->len, d->goff);
  }
}

void MultiDiskAdaptor::flushOSBuffers()
{
  for (auto& dwent : openedDiskWriterEntries_) {
    auto& dw = dwent->getDiskWriter();
    if (!dw) {
      continue;
    }
    dw->flushOSBuffers();
  }
}

bool MultiDiskAdaptor::fileExists()
{
  return std::find_if(std::begin(getFileEntries()), std::end(getFileEntries()),
                      std::mem_fn(&FileEntry::exists)) !=
         std::end(getFileEntries());
}

int64_t MultiDiskAdaptor::size()
{
  int64_t size = 0;
  for (auto& fe : getFileEntries()) {
    size += File(fe->getPath()).size();
  }
  return size;
}

std::unique_ptr<FileAllocationIterator>
MultiDiskAdaptor::fileAllocationIterator()
{
  return make_unique<MultiFileAllocationIterator>(this);
}

void MultiDiskAdaptor::enableReadOnly() { readOnly_ = true; }

void MultiDiskAdaptor::disableReadOnly() { readOnly_ = false; }

void MultiDiskAdaptor::enableMmap()
{
  for (auto& dwent : diskWriterEntries_) {
    auto& dw = dwent->getDiskWriter();
    if (dw) {
      dw->enableMmap();
    }
  }
}

void MultiDiskAdaptor::cutTrailingGarbage()
{
  for (auto& dwent : diskWriterEntries_) {
    int64_t length = dwent->getFileEntry()->getLength();
    if (File(dwent->getFilePath()).size() > length) {
      // We need open file before calling DiskWriter::truncate(int64_t)
      openIfNot(dwent.get(), &DiskWriterEntry::openFile);
      dwent->getDiskWriter()->truncate(length);
    }
  }
}

size_t MultiDiskAdaptor::utime(const Time& actime, const Time& modtime)
{
  size_t numOK = 0;
  for (auto& fe : getFileEntries()) {
    if (fe->isRequested()) {
      File f{fe->getPath()};
      if (f.isFile() && f.utime(actime, modtime)) {
        ++numOK;
      }
    }
  }
  return numOK;
}

} // namespace aria2
