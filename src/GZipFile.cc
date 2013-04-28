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

#include "GZipFile.h"

#include <algorithm>
#include <limits>

#include "a2io.h"
#include "util.h"

namespace aria2 {

GZipFile::GZipFile(const char* filename, const char* mode)
  : BufferedFile(0), fp_(0), open_(false),
    buflen_(1024), buf_(reinterpret_cast<char*>(malloc(buflen_)))
{
  FILE* fp =
#ifdef __MINGW32__
  a2fopen(utf8ToWChar(filename).c_str(), utf8ToWChar(mode).c_str());
#else // !__MINGW32__
  a2fopen(filename, mode);
#endif // !__MINGW32__

  open_  = fp;
  if (open_) {
    int fd = dup(fileno(fp));
    if ((open_ = fd) >= 0) {
      open_ = (fp_ = gzdopen(fd, mode));
      if (!open_) {
        ::close(fd);
      }
    }
    if (open_) {
#if HAVE_GZBUFFER
      gzbuffer(fp_, 1<<17);
#endif
#if HAVE_GZSETPARAMS
      gzsetparams(fp_, 2, Z_DEFAULT_STRATEGY);
#endif
    }
    fclose(fp);
  }
}

GZipFile::~GZipFile()
{
  close();
  free(buf_);
}

int GZipFile::close()
{
  if (open_) {
    open_ = false;
    return gzclose(fp_);
  }
  return 0;
}

bool GZipFile::isError() const
{
  int rv = 0;
  const char *e = gzerror(fp_, &rv);
  return (e != 0 && *e != 0) || rv != 0;
}

size_t GZipFile::read(void* ptr, size_t count)
{
  char *data = reinterpret_cast<char*>(ptr);
  size_t read = 0;
  while (count) {
    size_t len = std::min(count, (size_t)std::numeric_limits<unsigned>::max());
    int rv = gzread(fp_, data, len);
    if (rv <= 0) {
      break;
    }
    count -= rv;
    read += rv;
    data += rv;
  }
  return read;
}

size_t GZipFile::write(const void* ptr, size_t count)
{
  const char *data = reinterpret_cast<const char*>(ptr);
  size_t written = 0;
  while (count) {
    size_t len = std::min(count, (size_t)std::numeric_limits<unsigned>::max());
    int rv = gzwrite(fp_, data, len);
    if (rv <= 0) {
      break;
    }
    count -= rv;
    written += rv;
    data += rv;
  }
  return written;
}

char* GZipFile::gets(char* s, int size)
{
  return gzgets(fp_, s, size);
}

int GZipFile::flush()
{
  return gzflush(fp_, 0);
}

int GZipFile::vprintf(const char* format, va_list va)
{
  ssize_t len;
#ifdef __MINGW32__
  // Windows vsnprintf returns -1 when output is truncated, so we
  // cannot use same logic in non-MINGW32 code.
  len = _vscprintf(format, va);
  if(len == 0) {
    return 0;
  }
  // Include terminate null
  ++len;
  if(len > static_cast<ssize_t>(buflen_)) {
    while(static_cast<ssize_t>(buflen_) < len) {
      buflen_ *= 2;
    }
    buf_ = reinterpret_cast<char*>(realloc(buf_, buflen_));
  }
  len = vsnprintf(buf_, buflen_, format, va);
  if(len < 0) {
    return len;
  }
#else // !__MINGW32__
  for(;;) {
    len = vsnprintf(buf_, buflen_, format, va);
    // len does not include terminating null
    if(len >= static_cast<ssize_t>(buflen_)) {
      // Include terminate null
      ++len;
      // truncated; reallocate buf and try again
      while(static_cast<ssize_t>(buflen_) < len) {
        buflen_ *= 2;
      }
      buf_ = reinterpret_cast<char*>(realloc(buf_, buflen_));
    } else if(len < 0) {
      return len;
    } else {
      break;
    }
  }
#endif // !__MINGW32__
  return gzwrite(fp_, buf_, len);
}

} // namespace aria2
