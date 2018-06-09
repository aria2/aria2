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
#include "MultiFileAllocationIterator.h"
#include "MultiDiskAdaptor.h"
#include "FileEntry.h"
#include "AdaptiveFileAllocationIterator.h"
#include "TruncFileAllocationIterator.h"
#ifdef HAVE_SOME_FALLOCATE
#  include "FallocFileAllocationIterator.h"
#endif // HAVE_SOME_FALLOCATE
#include "DiskWriter.h"
#include "DefaultDiskWriterFactory.h"
#include "LogFactory.h"

namespace aria2 {

MultiFileAllocationIterator::MultiFileAllocationIterator(
    MultiDiskAdaptor* diskAdaptor)
    : diskAdaptor_{diskAdaptor},
      entryItr_{std::begin(diskAdaptor_->getDiskWriterEntries())}
{
}

MultiFileAllocationIterator::~MultiFileAllocationIterator()
{
  if (diskWriter_) {
    diskWriter_->closeFile();
  }
}

void MultiFileAllocationIterator::allocateChunk()
{
  if (fileAllocationIterator_) {
    if (!fileAllocationIterator_->finished()) {
      fileAllocationIterator_->allocateChunk();
      return;
    }

    if (diskWriter_) {
      diskWriter_->closeFile();
      diskWriter_.reset();
    }
    fileAllocationIterator_.reset();
    ++entryItr_;
  }

  while (entryItr_ != std::end(diskAdaptor_->getDiskWriterEntries())) {
    if (!(*entryItr_)->getDiskWriter()) {
      ++entryItr_;
      continue;
    }

    auto& fileEntry = (*entryItr_)->getFileEntry();
    // we use dedicated DiskWriter instead of
    // (*entryItr_)->getDiskWriter().  This is because
    // SingleFileAllocationIterator cannot reopen file if file is
    // closed by OpenedFileCounter.
    diskWriter_ =
        DefaultDiskWriterFactory().newDiskWriter((*entryItr_)->getFilePath());
    // Open file before calling DiskWriterEntry::size().  Calling
    // private function of MultiDiskAdaptor.
    diskWriter_->openFile(fileEntry->getLength());

    if ((*entryItr_)->needsFileAllocation() &&
        (*entryItr_)->size() < fileEntry->getLength()) {
      A2_LOG_INFO(fmt("Allocating file %s: target size=%" PRId64
                      ", current size=%" PRId64,
                      (*entryItr_)->getFilePath().c_str(),
                      fileEntry->getLength(), (*entryItr_)->size()));
      switch (diskAdaptor_->getFileAllocationMethod()) {
#ifdef HAVE_SOME_FALLOCATE
      case (DiskAdaptor::FILE_ALLOC_FALLOC):
        fileAllocationIterator_ = make_unique<FallocFileAllocationIterator>(
            diskWriter_.get(), (*entryItr_)->size(), fileEntry->getLength());
        break;
#endif // HAVE_SOME_FALLOCATE
      case (DiskAdaptor::FILE_ALLOC_TRUNC):
        fileAllocationIterator_ = make_unique<TruncFileAllocationIterator>(
            diskWriter_.get(), (*entryItr_)->size(), fileEntry->getLength());
        break;
      default:
        fileAllocationIterator_ = make_unique<AdaptiveFileAllocationIterator>(
            diskWriter_.get(), (*entryItr_)->size(), fileEntry->getLength());
        break;
      }
      fileAllocationIterator_->allocateChunk();
      return;
    }

    diskWriter_->closeFile();
    diskWriter_.reset();

    ++entryItr_;
  }
}

bool MultiFileAllocationIterator::finished()
{
  return entryItr_ == std::end(diskAdaptor_->getDiskWriterEntries()) &&
         (!fileAllocationIterator_ || fileAllocationIterator_->finished());
}

int64_t MultiFileAllocationIterator::getCurrentLength()
{
  if (!fileAllocationIterator_) {
    return 0;
  }
  else {
    return fileAllocationIterator_->getCurrentLength();
  }
}

int64_t MultiFileAllocationIterator::getTotalLength()
{
  if (!fileAllocationIterator_) {
    return 0;
  }
  else {
    return fileAllocationIterator_->getTotalLength();
  }
}

const DiskWriterEntries&
MultiFileAllocationIterator::getDiskWriterEntries() const
{
  return diskAdaptor_->getDiskWriterEntries();
}

} // namespace aria2
