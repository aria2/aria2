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

#ifndef D_COLORIZED_STREAM_H
#define D_COLORIZED_STREAM_H

#include <string>
#include <sstream>
#include <deque>

#include "config.h"

namespace aria2 {

namespace colors {

class Color {
private:
  std::string str_;

public:
  explicit Color(const char* str) : str_(std::string("\033[") + str + "m") {}

  const std::string& str() const { return str_; }
};

extern const Color black;
extern const Color red;
extern const Color green;
extern const Color yellow;
extern const Color blue;
extern const Color magenta;
extern const Color cyan;
extern const Color white;

extern const Color lightred;
extern const Color lightgreen;
extern const Color lightyellow;
extern const Color lightblue;
extern const Color lightmagenta;
extern const Color lightcyan;
extern const Color lightwhite;

extern const Color clear;

} // namespace colors

typedef std::char_traits<char> traits_t;
class ColorizedStreamBuf : public std::basic_streambuf<char, traits_t> {
  enum part_t { eColor, eString };
  typedef std::pair<part_t, std::string> elem_t;
  typedef std::deque<elem_t> elems_t;
  elems_t elems;

public:
  ColorizedStreamBuf() { elems.push_back(std::make_pair(eString, "")); }

  void setColor(const colors::Color& color)
  {
    elems.push_back(std::make_pair(eColor, color.str()));
    elems.push_back(std::make_pair(eString, ""));
  }

  traits_t::int_type overflow(traits_t::int_type c) CXX11_OVERRIDE
  {
    elems.back().second.push_back((char)c);
    return std::char_traits<char>::not_eof(c);
  }

  void append(const std::string& str) { elems.back().second += str; }

  void append(const char* str) { elems.back().second += str; }

  std::string str(bool color) const;
  std::string str(bool color, size_t max) const;
};

class ColorizedStream : public std::basic_ostream<char, traits_t> {
public:
  ColorizedStream()
      : std::basic_ios<char, traits_t>(&buf),
        std::basic_ostream<char, traits_t>(&buf)
  {
    init(&buf);
  }

  void setColor(const colors::Color& color) { buf.setColor(color); }
  void append(const std::string& str) { buf.append(str); }
  void append(const char* str) { buf.append(str); }

  std::string str(bool colors) const { return buf.str(colors); }
  std::string str(bool colors, size_t max) const
  {
    return buf.str(colors, max);
  }

private:
  ColorizedStreamBuf buf;
};

inline ColorizedStream& operator<<(ColorizedStream& stream,
                                   const std::string& str)
{
  stream.append(str);
  return stream;
}

inline ColorizedStream& operator<<(ColorizedStream& stream, const char* str)
{
  stream.append(str);
  return stream;
}

inline ColorizedStream& operator<<(ColorizedStream& stream,
                                   const colors::Color& c)
{
  stream.setColor(c);
  return stream;
}

} // namespace aria2

#endif // D_COLORIZED_STREAM_H
