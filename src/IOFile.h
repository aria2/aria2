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
#ifndef D_IO_FILE_H
#define D_IO_FILE_H

#include "OutputFile.h"

#include <string>
#include <iosfwd>

namespace aria2 {

// This is a wrapper base class intended to provide
// fopen/fclose/fread/fwrite/fgets functionality.
class IOFile : public OutputFile {
private:
  typedef void (IOFile::*unspecified_bool_type)() const;
  void goodState() const {}

public:
  IOFile() {}
  virtual ~IOFile() = default;
  // Returns true if file is opened and ferror returns 0. Otherwise
  // returns false.
  operator unspecified_bool_type() const;
  // wrapper for fread. Using 1 for 2nd argument of fread.
  size_t read(void* ptr, size_t count);
  // wrapper for fwrite. Using 1 for 2nd argument of fwrite.
  size_t write(const void* ptr, size_t count);
  virtual size_t write(const char* str) CXX11_OVERRIDE;
  // wrapper for fgets
  char* gets(char* s, int size);
  // wrapper for fgets, but trailing '\n' is replaced with '\0'.
  char* getsn(char* s, int size);
  // Reads one line and returns it. The last '\n' is removed.
  std::string getLine();
  // wrapper for fclose
  int close();
  // wrapper for fflush
  int flush() CXX11_OVERRIDE;
  // Return true if file is opened && feof(fp_) != 0. Otherwise
  // returns false.
  bool eof();
  // Returns true if file supports ANSI color escape codes.
  bool supportsColor() CXX11_OVERRIDE;
  // Convenient method. Read data to end of file and write them into
  // given stream. Returns written size.
  size_t transfer(std::ostream& out);
  int vprintf(const char* format, va_list va) CXX11_OVERRIDE;
  // Mode for reading
  static const char READ[];
  // Mode for writing
  static const char WRITE[];
  // Mode for append
  static const char APPEND[];

protected:
  virtual size_t onRead(void* ptr, size_t count) = 0;
  virtual size_t onWrite(const void* ptr, size_t count) = 0;
  virtual char* onGets(char* s, int size) = 0;
  virtual int onVprintf(const char* format, va_list va) = 0;
  virtual int onFlush() = 0;
  virtual int onClose() = 0;
  virtual bool onSupportsColor() = 0;

  virtual bool isError() const = 0;
  virtual bool isEOF() const = 0;
  virtual bool isOpen() const = 0;
};

} // namespace aria2

#endif // D_IO_FILE_H
