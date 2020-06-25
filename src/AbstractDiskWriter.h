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
#ifndef D_ABSTRACT_DISK_WRITER_H
#define D_ABSTRACT_DISK_WRITER_H

#include "DiskWriter.h"
#include <string>

namespace aria2 {

class AbstractDiskWriter : public DiskWriter {
private:
  std::string filename_;

#ifdef __MINGW32__
  HANDLE fd_;
  // The handle for memory mapped file. mmap equivalent in Windows.
  HANDLE mapView_;
#else  // !__MINGW32__
  int fd_;
#endif // !__MINGW32__

  bool readOnly_;

  bool enableMmap_;
  unsigned char* mapaddr_;
  int64_t maplen_;

  ssize_t writeDataInternal(const unsigned char* data, size_t len,
                            int64_t offset);
  ssize_t readDataInternal(unsigned char* data, size_t len, int64_t offset);

  void seek(int64_t offset);

  void ensureMmapWrite(size_t len, int64_t offset);

protected:
  void createFile(int addFlags = 0);

public:
  AbstractDiskWriter(const std::string& filename);
  virtual ~AbstractDiskWriter();

  virtual void openFile(int64_t totalLength = 0) CXX11_OVERRIDE;

  virtual void closeFile() CXX11_OVERRIDE;

  virtual void openExistingFile(int64_t totalLength = 0) CXX11_OVERRIDE;

  virtual void writeData(const unsigned char* data, size_t len,
                         int64_t offset) CXX11_OVERRIDE;

  virtual ssize_t readData(unsigned char* data, size_t len,
                           int64_t offset) CXX11_OVERRIDE;

  virtual void truncate(int64_t length) CXX11_OVERRIDE;

  // File must be opened before calling this function.
  virtual void allocate(int64_t offset, int64_t length,
                        bool sparse) CXX11_OVERRIDE;

  virtual int64_t size() CXX11_OVERRIDE;

  virtual void enableReadOnly() CXX11_OVERRIDE;

  virtual void disableReadOnly() CXX11_OVERRIDE;

  virtual void enableMmap() CXX11_OVERRIDE;

  virtual void dropCache(int64_t len, int64_t offset) CXX11_OVERRIDE;

  virtual void flushOSBuffers() CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_ABSTRACT_DISK_WRITER_H
