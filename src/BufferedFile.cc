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
#include "BufferedFile.h"

#include <cstring>
#include <cstdarg>
#include <ostream>

#include "a2io.h"
#include "util.h"

namespace aria2 {

BufferedFile::BufferedFile(const char* filename, const char* mode)
    : fp_(strcmp(DEV_STDIN, filename) == 0
              ? stdin
              :
#ifdef __MINGW32__
              a2fopen(utf8ToWChar(filename).c_str(), utf8ToWChar(mode).c_str())
#else  // !__MINGW32__
              a2fopen(filename, mode)
#endif // !__MINGW32__
              ),
      supportsColor_(fp_ ? isatty(fileno(fp_)) : false)
{
}

BufferedFile::BufferedFile(FILE* fp)
    : fp_(fp), supportsColor_(fp_ ? isatty(fileno(fp_)) : false)
{
}

BufferedFile::~BufferedFile() { close(); }

size_t BufferedFile::onRead(void* ptr, size_t count)
{
  return fread(ptr, 1, count, fp_);
}

size_t BufferedFile::onWrite(const void* ptr, size_t count)
{
  return fwrite(ptr, 1, count, fp_);
}

char* BufferedFile::onGets(char* s, int size) { return fgets(s, size, fp_); }

int BufferedFile::onClose()
{
  int rv = 0;
  if (fp_) {
    fflush(fp_);
#ifndef __MINGW32__
    fsync(fileno(fp_));
#else  // __MINGW32__
    _commit(fileno(fp_));
#endif // __MINGW32__
    if (fp_ != stdin && fp_ != stderr) {
      rv = fclose(fp_);
    }
    fp_ = nullptr;
  }
  return rv;
}

int BufferedFile::onVprintf(const char* format, va_list va)
{
  return vfprintf(fp_, format, va);
}

int BufferedFile::onFlush() { return fflush(fp_); }

bool BufferedFile::onSupportsColor() { return supportsColor_; }

bool BufferedFile::isError() const { return ferror(fp_); }

bool BufferedFile::isEOF() const { return feof(fp_); }

bool BufferedFile::isOpen() const { return fp_; }

} // namespace aria2
