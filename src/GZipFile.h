/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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

#ifndef D_GZIP_FILE_H
#define D_GZIP_FILE_H

#include <zlib.h>

#include "BufferedFile.h"

namespace aria2 {

class GZipFile: public BufferedFile {
public:
  GZipFile(const char* filename, const char* mode);
  virtual ~GZipFile() {}
  virtual int close();
  virtual size_t read(void* ptr, size_t count);
  virtual size_t write(const void* ptr, size_t count);
  virtual char* gets(char* s, int size);
  virtual int vprintf(const char* format, va_list va);
  virtual int flush();
private:
  // Don't allow copying
  GZipFile(const GZipFile&);
  GZipFile& operator=(const GZipFile&);

  gzFile fp_;
  bool open_;

protected:
  virtual bool isError() const;
  virtual bool isEOF() const { return gzeof(fp_); }
  virtual bool isOpen() const { return open_; }
};

} // namespace aria2

#endif // D_GZIP_FILE_H
