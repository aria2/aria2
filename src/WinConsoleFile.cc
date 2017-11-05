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
#include <vector>

#include "a2io.h"
#include "util.h"

namespace {

#define FOREGROUND_BLACK 0
#define FOREGROUND_WHITE FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE

#define BACKGROUND_BLACK 0
#define BACKGROUND_WHITE BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE

const WORD kForeground[] = {
    FOREGROUND_BLACK,                   // black
    FOREGROUND_RED,                     // red
    FOREGROUND_GREEN,                   // green
    FOREGROUND_RED | FOREGROUND_GREEN,  // yellow
    FOREGROUND_BLUE,                    // blue
    FOREGROUND_BLUE | FOREGROUND_RED,   // magenta
    FOREGROUND_BLUE | FOREGROUND_GREEN, // cyan
    FOREGROUND_WHITE                    // white
};

const int kForegroundSize = sizeof(kForeground) / sizeof(kForeground[0]);

const WORD kBackground[] = {
    BACKGROUND_BLACK,                   // black
    BACKGROUND_RED,                     // red
    BACKGROUND_GREEN,                   // green
    BACKGROUND_RED | BACKGROUND_GREEN,  // yellow
    BACKGROUND_BLUE,                    // blue
    BACKGROUND_BLUE | BACKGROUND_RED,   // magenta
    BACKGROUND_BLUE | BACKGROUND_GREEN, // cyan
    BACKGROUND_WHITE                    // white
};

const int kBackgroundSize = sizeof(kBackground) / sizeof(kBackground[0]);

} // namespace

namespace aria2 {

WinConsoleFile::WinConsoleFile(DWORD stdHandle)
    : stdHandle_(stdHandle),
      bold_(false),
      underline_(false),
      reverse_(false),
      fg_(7),
      bg_(0)
{
  if (supportsColor()) {
    int color;
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(handle(), &info);
    bold_ = info.wAttributes & FOREGROUND_INTENSITY;
    underline_ = info.wAttributes & BACKGROUND_INTENSITY;
    color = info.wAttributes &
            (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
    for (int fg = 0; fg < kForegroundSize; fg++) {
      if (kForeground[fg] == color) {
        fg_ = fg;
        break;
      }
    }
    color = info.wAttributes &
            (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
    for (int bg = 0; bg < kBackgroundSize; bg++) {
      if (kBackground[bg] == color) {
        bg_ = bg;
        break;
      }
    }
  }

  defbold_ = bold_;
  defunderline_ = underline_;
  deffg_ = fg_;
  defbg_ = bg_;
}

bool WinConsoleFile::supportsColor()
{
  DWORD mode;
  return GetConsoleMode(handle(), &mode);
}

size_t WinConsoleFile::write(const char* str)
{
  if (!supportsColor()) {
    DWORD written = 0;
    WriteFile(handle(), str, strlen(str), &written, 0);
    return written;
  }

  auto msg = utf8ToWChar(str);
  return writeColorful(msg);
}

int WinConsoleFile::vprintf(const char* format, va_list va)
{
  ssize_t r = vsnprintf(NULL, 0, format, va);
  if (r <= 0) {
    return 0;
  }
  auto buf = make_unique<char[]>(++r);
  r = vsnprintf(buf.get(), r, format, va);
  if (r < 0) {
    return 0;
  }
  return write(buf.get());
}

size_t WinConsoleFile::writeColorful(const std::wstring& str)
{
  size_t written = 0;
  DWORD cw;

  wchar_t suffix;
  int arg = 0;
  std::vector<int> args;
  std::vector<wchar_t> buffer;
  buffer.reserve(str.length());

  enum state_ { ePrefix, ePreFin, eNum0, eNum } state = ePrefix;

  for (const wchar_t ch : str) {
    if (state == ePrefix) {
      if (ch == '\033') {
        state = ePreFin;
      }
      else {
        buffer.push_back(ch);
        continue;
      }
    }
    else if (state == ePreFin) {
      if (ch == '\033')
        ;
      else if (ch == '[') {
        state = eNum0;
      }
      else {
        state = ePrefix;
      }
    }
    else if (state == eNum0 || state == eNum) {
      if (isdigit(ch)) {
        arg = (arg * 10) + (ch - '0');
        state = eNum;
      }
      else if (ch == ';') {
        args.push_back(arg);
        arg = 0;
        state = eNum0;
      }
      else if (ch != '?') {
        if (state == eNum) {
          args.push_back(arg);
        }
        suffix = ch;
        goto out;
      }
    }

    ++written;
    continue;

  out:
    cw = 0;
    if (!buffer.empty()) {
      WriteConsoleW(handle(), buffer.data(), buffer.size(), &cw, nullptr);
    }
    written += cw;

    if (suffix == 'm') {
      if (args.empty()) {
        args.push_back(0);
      }
      for (const int a : args) {
        if (a == 0) {
          bold_ = defbold_;
          underline_ = defunderline_;
          reverse_ = false;
          fg_ = deffg_;
          bg_ = defbg_;
        }
        else if (30 <= a && a <= 37) {
          fg_ = a - 30;
        }
        else if (40 <= a && a <= 47) {
          bg_ = a - 40;
        }
        else if (a == 1 || a == 21) {
          bold_ = a == 1;
        }
        else if (a == 4 || a == 24) {
          underline_ = a == 4;
        }
        else if (a == 7 || a == 27) {
          reverse_ = a == 7;
        }
      }
      WORD attribute = 0;
      if (reverse_) {
        attribute = kForeground[bg_] | kBackground[fg_];
      }
      else {
        attribute = kForeground[fg_] | kBackground[bg_];
      }
      if (bold_) {
        attribute |= FOREGROUND_INTENSITY;
      }
      if (underline_) {
        attribute |= BACKGROUND_INTENSITY;
      }
      SetConsoleTextAttribute(handle(), attribute);
    }

    suffix = 0;
    state = ePrefix;
    arg = 0;
    args.clear();
    buffer.clear();
    ++written;
  }

  if (!buffer.empty()) {
    cw = 0;
    WriteConsoleW(handle(), buffer.data(), buffer.size(), &cw, nullptr);
    written += cw;
  }

  return written;
}

} // namespace aria2
