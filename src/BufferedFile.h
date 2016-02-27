/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#ifndef D_BUFFERED_FILE_H
#define D_BUFFERED_FILE_H

#include "IOFile.h"

#include <cstdio>

namespace aria2 {

// IOFILE implementation using standard I/O functions.
class BufferedFile : public IOFile {
public:
  BufferedFile(const char* filename, const char* mode);
  BufferedFile(FILE* fp);
  virtual ~BufferedFile();

protected:
  // wrapper for fread. Using 1 for 2nd argument of fread.
  virtual size_t onRead(void* ptr, size_t count) CXX11_OVERRIDE;
  // wrapper for fwrite. Using 1 for 2nd argument of fwrite.
  virtual size_t onWrite(const void* ptr, size_t count) CXX11_OVERRIDE;
  // wrapper for fgets
  virtual char* onGets(char* s, int size) CXX11_OVERRIDE;
  virtual int onVprintf(const char* format, va_list va) CXX11_OVERRIDE;
  // wrapper for fflush
  virtual int onFlush() CXX11_OVERRIDE;
  // wrapper for fclose
  virtual int onClose() CXX11_OVERRIDE;
  virtual bool onSupportsColor() CXX11_OVERRIDE;
  virtual bool isError() const CXX11_OVERRIDE;
  virtual bool isEOF() const CXX11_OVERRIDE;
  virtual bool isOpen() const CXX11_OVERRIDE;

private:
  // Don't allow copying;
  BufferedFile(const BufferedFile&);
  BufferedFile& operator=(const BufferedFile&);

  FILE* fp_;
  bool supportsColor_;
};

} // namespace aria2

#endif // D_BUFFERED_FILE_H
