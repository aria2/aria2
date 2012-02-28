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

#include "OutputFile.h"

#include <cstdio>
#include <string>
#include <iosfwd>

namespace aria2 {

// This is a wrapper class for fopen/fclose/fread/fwrite/fgets.
class BufferedFile:public OutputFile {
private:
  typedef void (BufferedFile::*unspecified_bool_type)() const;
  void good_state() const {}
public:
  BufferedFile(const std::string& filename, const std::string& mode);
  BufferedFile(FILE* fp);
  virtual ~BufferedFile();
  // Returns true if file is opened and ferror returns 0. Otherwise
  // returns false.
  operator unspecified_bool_type() const;
  // wrapper for fread. Using 1 for 2nd argument of fread.
  size_t read(void* ptr, size_t count);
  // wrapper for fwrite. Using 1 for 2nd argument of fwrite.
  size_t write(const void* ptr, size_t count);
  virtual size_t write(const char* str);
  // wrapper for fgets
  char* gets(char* s, int size);
  // wrapper for fgets, but trailing '\n' is replaced with '\0'.
  char* getsn(char* s, int size);
  // Reads one line and returns it. The last '\n' is removed.
  std::string getLine();
  // wrapper for fclose
  int close();
  // Return true if open_ && feof(fp_) != 0. Otherwise returns false.
  bool eof();
  // Convenient method. Read data to end of file and write them into
  // given stream. Returns written size.
  size_t transfer(std::ostream& out);
  // wrapper for fprintf
  virtual int printf(const char* format, ...);
  // wrapper for fflush
  virtual int flush();
  // Mode for reading
  static const std::string READ;
  // Mode for writing
  static const std::string WRITE;
  // Mode for append
  static const std::string APPEND;
private:
  // Don't allow copying
  BufferedFile(const BufferedFile&);
  BufferedFile& operator=(const BufferedFile&);

  FILE* fp_;
  // true when file has been opened.
  bool open_;
};

} // namespace aria2

#endif // D_BUFFERED_FILE_H
