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
#ifndef _D_ABSTRACT_DISK_WRITER_H_
#define _D_ABSTRACT_DISK_WRITER_H_

#include "DiskWriter.h"
#include <string>

namespace aria2 {

class Logger;

class AbstractDiskWriter : public DiskWriter {
protected:
  std::string filename;
  int fd;

  bool _readOnly;

  Logger* logger;

  void createFile(const std::string& filename, int addFlags = 0);

private:
  ssize_t writeDataInternal(const unsigned char* data, size_t len);
  ssize_t readDataInternal(unsigned char* data, size_t len);

  void seek(off_t offset);
public:
  AbstractDiskWriter();
  virtual ~AbstractDiskWriter();

  virtual void openFile(const std::string& filename, uint64_t totalLength = 0);

  virtual void closeFile();

  virtual void openExistingFile(const std::string& filename, uint64_t totalLength = 0);

  virtual void writeData(const unsigned char* data, size_t len, off_t offset);

  virtual ssize_t readData(unsigned char* data, size_t len, off_t offset);

  virtual void truncate(uint64_t length);

#ifdef HAVE_POSIX_FALLOCATE
  // File must be opened before calling this function.
  virtual void allocate(off_t offset, uint64_t length);
#endif // HAVE_POSIX_FALLOCATE

  virtual uint64_t size();
  
  virtual void enableDirectIO();

  virtual void disableDirectIO();

  virtual void enableReadOnly();

  virtual void disableReadOnly();
};

} // namespace aria2

#endif // _D_ABSTRACT_DISK_WRITER_H_
