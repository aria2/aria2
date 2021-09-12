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
#ifndef D_DISK_WRITER_H
#define D_DISK_WRITER_H

#include "BinaryStream.h"

namespace aria2 {

/**
 * Interface for writing to a binary stream of bytes.
 *
 */
class DiskWriter : public BinaryStream {
public:
  DiskWriter() {}

  virtual ~DiskWriter() = default;
  /**
   * Opens file. If the file exists, then it is truncated to 0 length.
   */
  virtual void initAndOpenFile(int64_t totalLength = 0) = 0;

  virtual void openFile(int64_t totalLength = 0) = 0;

  /**
   * Closes this output stream.
   */
  // TODO we have to examine the return value of close()
  virtual void closeFile() = 0;

  /**
   * Opens a file.  If the file does not exist, an exception may be
   * thrown.
   */
  virtual void openExistingFile(int64_t totalLength = 0) = 0;

  // Returns file length
  virtual int64_t size() = 0;

  // Enables read-only mode. After this call, openExistingFile() opens
  // file in read-only mode. This is an optional functionality. The
  // default implementation is do nothing.
  virtual void enableReadOnly() {}

  // Disables read-only mode. After this call, openExistingFile()
  // opens file in read/write mode. This is an optional
  // functionality. The default implementation is do noting.
  virtual void disableReadOnly() {}

  // Enables mmap.
  virtual void enableMmap() {}

  // Drops cache in range [offset, offset + len)
  virtual void dropCache(int64_t len, int64_t offset) {}

  // Force physical write of data from OS buffer cache.
  virtual void flushOSBuffers() {}
};

} // namespace aria2

#endif // D_DISK_WRITER_H
