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
#include "WinConsoleFile.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <string>

#ifdef __MINGW32__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif // __MINGW32__

#include "a2io.h"
#include "util.h"

namespace aria2 {

WinConsoleFile::WinConsoleFile() {}

WinConsoleFile::~WinConsoleFile() {}

namespace {
bool console()
{
  DWORD mode;
  return GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode);
}
} // namespace

size_t WinConsoleFile::write(const char* str)
{
  DWORD written;
  if(console()) {
    std::wstring msg = utf8ToWChar(str);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),
                  msg.c_str(), msg.size(), &written, 0);
  } else {
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
              str, strlen(str), &written, 0);
  }
  return written;
}

int WinConsoleFile::printf(const char* format, ...)
{
  char buf[1024];
  va_list ap;
  va_start(ap, format);
  int r = vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);
  if(r <= 0) {
    return 0;
  }
  DWORD written;
  if(console()) {
    std::wstring msg = utf8ToWChar(buf);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),
                  msg.c_str(), msg.size(), &written, 0);
  } else {
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
              buf, r, &written, 0);
  }
  return written;
}

int WinConsoleFile::flush()
{
  return 0;
}

} // namespace aria2
