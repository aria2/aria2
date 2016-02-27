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
#include "AbstractSingleDiskAdaptor.h"
#include "File.h"
#include "AdaptiveFileAllocationIterator.h"
#include "DiskWriter.h"
#include "FileEntry.h"
#include "TruncFileAllocationIterator.h"
#include "WrDiskCacheEntry.h"
#include "LogFactory.h"
#ifdef HAVE_SOME_FALLOCATE
#include "FallocFileAllocationIterator.h"
#endif // HAVE_SOME_FALLOCATE

namespace aria2 {

AbstractSingleDiskAdaptor::AbstractSingleDiskAdaptor()
    : totalLength_(0), readOnly_(false)
{
}

AbstractSingleDiskAdaptor::~AbstractSingleDiskAdaptor() {}

void AbstractSingleDiskAdaptor::initAndOpenFile()
{
  diskWriter_->initAndOpenFile(totalLength_);
}

void AbstractSingleDiskAdaptor::openFile()
{
  diskWriter_->openFile(totalLength_);
}

void AbstractSingleDiskAdaptor::closeFile() { diskWriter_->closeFile(); }

void AbstractSingleDiskAdaptor::openExistingFile()
{
  diskWriter_->openExistingFile(totalLength_);
}

void AbstractSingleDiskAdaptor::writeData(const unsigned char* data, size_t len,
                                          int64_t offset)
{
  diskWriter_->writeData(data, len, offset);
}

ssize_t AbstractSingleDiskAdaptor::readData(unsigned char* data, size_t len,
                                            int64_t offset)
{
  return diskWriter_->readData(data, len, offset);
}

ssize_t AbstractSingleDiskAdaptor::readDataDropCache(unsigned char* data,
                                                     size_t len, int64_t offset)
{
  auto rv = readData(data, len, offset);

  if (rv > 0) {
    diskWriter_->dropCache(len, offset);
  }

  return rv;
}

void AbstractSingleDiskAdaptor::writeCache(const WrDiskCacheEntry* entry)
{
  // Write cached data in 4KiB aligned offset. This reduces disk
  // activity especially on Windows 7 NTFS. In this code, we assume
  // that maximum length of DataCell data is 16KiB to simplify the
  // code.
  unsigned char buf[16_k];
  int64_t start = 0;
  size_t buflen = 0;
  size_t buffoffset = 0;
  const WrDiskCacheEntry::DataCellSet& dataSet = entry->getDataSet();
  for (auto& d : dataSet) {
    if (start + static_cast<ssize_t>(buflen) < d->goff) {
      A2_LOG_DEBUG(fmt("Cache flush goff=%" PRId64 ", len=%lu", start,
                       static_cast<unsigned long>(buflen)));
      writeData(buf + buffoffset, buflen - buffoffset, start);
      start = d->goff;
      buflen = buffoffset = 0;
    }
    if (buflen == 0 && (d->goff & 0xfff) == 0 && (d->len & 0xfff) == 0) {
      // Already aligned. Write it without copy.
      A2_LOG_DEBUG(fmt("Cache flush goff=%" PRId64 ", len=%lu", start,
                       static_cast<unsigned long>(d->len)));
      writeData(d->data + d->offset, d->len, start);
      start += d->len;
    }
    else {
      if (buflen == 0) {
        buflen = buffoffset = d->goff & 0xfff;
      }
      size_t wlen = std::min(sizeof(buf) - buflen, d->len);
      memcpy(buf + buflen, d->data + d->offset, wlen);
      buflen += wlen;
      if (buflen == sizeof(buf)) {
        A2_LOG_DEBUG(fmt("Cache flush goff=%" PRId64 ", len=%lu", start,
                         static_cast<unsigned long>(buflen)));
        writeData(buf + buffoffset, buflen - buffoffset, start);
        memcpy(buf, d->data + d->offset + wlen, d->len - wlen);
        start += sizeof(buf) - buffoffset;
        buflen = d->len - wlen;
        buffoffset = 0;
      }
    }
  }
  writeData(buf + buffoffset, buflen - buffoffset, start);
}

bool AbstractSingleDiskAdaptor::fileExists()
{
  return File(getFilePath()).exists();
}

int64_t AbstractSingleDiskAdaptor::size() { return File(getFilePath()).size(); }

void AbstractSingleDiskAdaptor::truncate(int64_t length)
{
  diskWriter_->truncate(length);
}

std::unique_ptr<FileAllocationIterator>
AbstractSingleDiskAdaptor::fileAllocationIterator()
{
  switch (getFileAllocationMethod()) {
#ifdef HAVE_SOME_FALLOCATE
  case (DiskAdaptor::FILE_ALLOC_FALLOC):
    return make_unique<FallocFileAllocationIterator>(diskWriter_.get(), size(),
                                                     totalLength_);
#endif // HAVE_SOME_FALLOCATE
  case (DiskAdaptor::FILE_ALLOC_TRUNC):
    return make_unique<TruncFileAllocationIterator>(diskWriter_.get(), size(),
                                                    totalLength_);
  default:
    return make_unique<AdaptiveFileAllocationIterator>(diskWriter_.get(),
                                                       size(), totalLength_);
  }
}

void AbstractSingleDiskAdaptor::enableReadOnly()
{
  diskWriter_->enableReadOnly();
  readOnly_ = true;
}

void AbstractSingleDiskAdaptor::disableReadOnly()
{
  diskWriter_->disableReadOnly();
  readOnly_ = false;
}

void AbstractSingleDiskAdaptor::enableMmap() { diskWriter_->enableMmap(); }

void AbstractSingleDiskAdaptor::cutTrailingGarbage()
{
  if (File(getFilePath()).size() > totalLength_) {
    diskWriter_->truncate(totalLength_);
  }
}

void AbstractSingleDiskAdaptor::setDiskWriter(
    std::unique_ptr<DiskWriter> diskWriter)
{
  diskWriter_ = std::move(diskWriter);
}

void AbstractSingleDiskAdaptor::setTotalLength(int64_t totalLength)
{
  totalLength_ = totalLength;
}

} // namespace aria2
