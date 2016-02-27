/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "WrDiskCacheEntry.h"

#include <cstring>

#include "DiskAdaptor.h"
#include "RecoverableException.h"
#include "DownloadFailureException.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

WrDiskCacheEntry::WrDiskCacheEntry(
    const std::shared_ptr<DiskAdaptor>& diskAdaptor)
    : sizeKey_(0),
      lastUpdate_(0),
      size_(0),
      error_(CACHE_ERR_SUCCESS),
      errorCode_(error_code::UNDEFINED),
      diskAdaptor_(diskAdaptor)
{
}

WrDiskCacheEntry::~WrDiskCacheEntry()
{
  if (!set_.empty()) {
    A2_LOG_WARN(fmt("WrDiskCacheEntry is not empty size=%lu",
                    static_cast<unsigned long>(size_)));
  }
  deleteDataCells();
}

void WrDiskCacheEntry::deleteDataCells()
{
  for (auto& e : set_) {
    delete[] e->data;
    delete e;
  }
  set_.clear();
  size_ = 0;
}

void WrDiskCacheEntry::writeToDisk()
{
  try {
    diskAdaptor_->writeCache(this);
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR_EX("Error when trying to flush write cache", e);
    error_ = CACHE_ERR_ERROR;
    errorCode_ = e.getErrorCode();
  }
  deleteDataCells();
}

void WrDiskCacheEntry::clear() { deleteDataCells(); }

bool WrDiskCacheEntry::cacheData(DataCell* dataCell)
{
  A2_LOG_DEBUG(fmt("WrDiskCacheEntry cache goff=%" PRId64 ", len=%lu",
                   dataCell->goff, static_cast<unsigned long>(dataCell->len)));
  if (set_.insert(dataCell).second) {
    size_ += dataCell->len;
    return true;
  }
  else {
    return false;
  }
}

size_t WrDiskCacheEntry::append(int64_t goff, const unsigned char* data,
                                size_t len)
{
  if (set_.empty()) {
    return 0;
  }
  auto i = set_.end();
  --i;
  if (static_cast<int64_t>((*i)->goff + (*i)->len) == goff) {
    size_t wlen = std::min((*i)->capacity - (*i)->len, len);
    memcpy((*i)->data + (*i)->offset + (*i)->len, data, wlen);
    (*i)->len += wlen;
    size_ += wlen;
    return wlen;
  }
  else {
    return 0;
  }
}

} // namespace aria2
