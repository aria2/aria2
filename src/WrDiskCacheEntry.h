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
#ifndef D_WR_DISK_CACHE_ENTRY_H
#define D_WR_DISK_CACHE_ENTRY_H

#include "common.h"

#include <set>
#include <memory>

#include "a2functional.h"
#include "error_code.h"

namespace aria2 {

class DiskAdaptor;
class WrDiskCache;

class WrDiskCacheEntry {
public:
  struct DataCell {
    // Where the data is going to be put in DiskAdaptor
    int64_t goff;
    // data must be len+offset bytes. Thus, the cached data is
    // [data+offset, data+offset+len).
    unsigned char* data;
    size_t offset;
    size_t len;
    // valid memory range from data+offset
    size_t capacity;
    bool operator<(const DataCell& rhs) const { return goff < rhs.goff; }
  };

  typedef std::set<DataCell*, DerefLess<DataCell*>> DataCellSet;

  WrDiskCacheEntry(const std::shared_ptr<DiskAdaptor>& diskAdaptor);
  ~WrDiskCacheEntry();

  // Flushes the cached data to the disk and deletes them.
  void writeToDisk();
  // Deletes cached data without flushing to the disk.
  void clear();

  // Caches |dataCell|
  bool cacheData(DataCell* dataCell);

  // Appends into last dataCell in set_ if the region is
  // contagious. Returns the number of copied bytes.
  size_t append(int64_t goff, const unsigned char* data, size_t len);

  size_t getSize() const { return size_; }
  void setSizeKey(size_t sizeKey) { sizeKey_ = sizeKey; }
  size_t getSizeKey() const { return sizeKey_; }
  void setLastUpdate(int64_t clock) { lastUpdate_ = clock; }
  int64_t getLastUpdate() const { return lastUpdate_; }
  bool operator<(const WrDiskCacheEntry& rhs) const
  {
    return sizeKey_ > rhs.sizeKey_ ||
           (sizeKey_ == rhs.sizeKey_ && lastUpdate_ < rhs.lastUpdate_);
  }

  enum { CACHE_ERR_SUCCESS, CACHE_ERR_ERROR };

  int getError() const { return error_; }
  error_code::Value getErrorCode() const { return errorCode_; }

  const DataCellSet& getDataSet() const { return set_; }

private:
  void deleteDataCells();

  size_t sizeKey_;
  int64_t lastUpdate_;

  size_t size_;

  DataCellSet set_;

  int error_;
  error_code::Value errorCode_;

  std::shared_ptr<DiskAdaptor> diskAdaptor_;
};

} // namespace aria2

#endif // D_WR_DISK_CACHE_ENTRY_H
