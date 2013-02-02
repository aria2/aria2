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
# include "FallocFileAllocationIterator.h"
#endif // HAVE_SOME_FALLOCATE

namespace aria2 {

AbstractSingleDiskAdaptor::AbstractSingleDiskAdaptor():
  totalLength_(0), readOnly_(false) {}

AbstractSingleDiskAdaptor::~AbstractSingleDiskAdaptor() {}

void AbstractSingleDiskAdaptor::initAndOpenFile()
{
  diskWriter_->initAndOpenFile(totalLength_);
}

void AbstractSingleDiskAdaptor::openFile()
{
  diskWriter_->openFile(totalLength_);
}

void AbstractSingleDiskAdaptor::closeFile()
{
  diskWriter_->closeFile();
}

void AbstractSingleDiskAdaptor::openExistingFile()
{
  diskWriter_->openExistingFile(totalLength_);
}

void AbstractSingleDiskAdaptor::writeData
(const unsigned char* data, size_t len, int64_t offset)
{
  diskWriter_->writeData(data, len, offset);
}

ssize_t AbstractSingleDiskAdaptor::readData
(unsigned char* data, size_t len, int64_t offset)
{
  return diskWriter_->readData(data, len, offset);
}

void AbstractSingleDiskAdaptor::writeCache(const WrDiskCacheEntry* entry)
{
  // Write cached data in 4KiB aligned offset. This reduces disk
  // activity especially on Windows 7 NTFS. In this code, we assume
  // that maximum length of DataCell data is 16KiB to simplify the
  // code.
  unsigned char buf[16*1024];
  int64_t start = 0;
  size_t buflen = 0;
  size_t buffoffset = 0;
  const WrDiskCacheEntry::DataCellSet& dataSet = entry->getDataSet();
  for(WrDiskCacheEntry::DataCellSet::const_iterator i = dataSet.begin(),
        eoi = dataSet.end(); i != eoi; ++i) {
    if(start+static_cast<ssize_t>(buflen) < (*i)->goff) {
      A2_LOG_DEBUG(fmt("Cache flush goff=%"PRId64", len=%lu",
                       start, static_cast<unsigned long>(buflen)));
      writeData(buf+buffoffset, buflen-buffoffset, start);
      start = (*i)->goff;
      buflen = buffoffset = 0;
    }
    if(buflen == 0 && ((*i)->goff & 0xfff) == 0 && ((*i)->len & 0xfff) == 0) {
      // Already aligned. Write it without copy.
      A2_LOG_DEBUG(fmt("Cache flush goff=%"PRId64", len=%lu",
                       start, static_cast<unsigned long>((*i)->len)));
      writeData((*i)->data + (*i)->offset, (*i)->len, start);
      start += (*i)->len;
    } else {
      if(buflen == 0) {
        buflen = buffoffset = (*i)->goff & 0xfff;
      }
      size_t wlen = std::min(sizeof(buf) - buflen, (*i)->len);
      memcpy(buf+buflen, (*i)->data+(*i)->offset, wlen);
      buflen += wlen;
      if(buflen == sizeof(buf)) {
        A2_LOG_DEBUG(fmt("Cache flush goff=%"PRId64", len=%lu",
                         start, static_cast<unsigned long>(buflen)));
        writeData(buf+buffoffset, buflen-buffoffset, start);
        memcpy(buf, (*i)->data + (*i)->offset + wlen, (*i)->len - wlen);
        start += sizeof(buf) - buffoffset;
        buflen = (*i)->len - wlen;
        buffoffset = 0;
      }
    }
  }
  writeData(buf+buffoffset, buflen-buffoffset, start);
}

bool AbstractSingleDiskAdaptor::fileExists()
{
  return File(getFilePath()).exists();
}

int64_t AbstractSingleDiskAdaptor::size()
{
  return File(getFilePath()).size();
}

void AbstractSingleDiskAdaptor::truncate(int64_t length)
{
  diskWriter_->truncate(length);
}

SharedHandle<FileAllocationIterator>
AbstractSingleDiskAdaptor::fileAllocationIterator()
{
  switch(getFileAllocationMethod()) {
#ifdef HAVE_SOME_FALLOCATE
  case(DiskAdaptor::FILE_ALLOC_FALLOC): {
    SharedHandle<FallocFileAllocationIterator> h
      (new FallocFileAllocationIterator
       (diskWriter_.get(), size() ,totalLength_));
    return h;
  }
#endif // HAVE_SOME_FALLOCATE
  case(DiskAdaptor::FILE_ALLOC_TRUNC): {
    SharedHandle<TruncFileAllocationIterator> h
      (new TruncFileAllocationIterator
       (diskWriter_.get(), size(), totalLength_));
    return h;
  }
  default: {
    SharedHandle<AdaptiveFileAllocationIterator> h
      (new AdaptiveFileAllocationIterator
       (diskWriter_.get(), size(), totalLength_));
    return h;
  }
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

void AbstractSingleDiskAdaptor::enableMmap()
{
  diskWriter_->enableMmap();
}

void AbstractSingleDiskAdaptor::cutTrailingGarbage()
{
  if(File(getFilePath()).size() > totalLength_) {
    diskWriter_->truncate(totalLength_);
  }
}

void AbstractSingleDiskAdaptor::setDiskWriter
(const SharedHandle<DiskWriter>& diskWriter)
{
  diskWriter_ = diskWriter;
}

void AbstractSingleDiskAdaptor::setTotalLength(int64_t totalLength)
{
  totalLength_ = totalLength;
}

} // namespace aria2
