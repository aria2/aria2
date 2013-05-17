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
#include "util.h"

#ifdef __sun
// For opensolaris, just include signal.h which includes sys/signal.h
#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H
#else // !__sun
#ifdef HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif // HAVE_SYS_SIGNAL_H
#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H
#endif // !__sun

#include <sys/types.h>
#ifdef HAVE_PWD_H
#  include <pwd.h>
#endif // HAVE_PWD_H

#include <cerrno>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <ostream>
#include <algorithm>
#include <fstream>
#include <iomanip>

#include "SimpleRandomizer.h"
#include "File.h"
#include "Randomizer.h"
#include "a2netcompat.h"
#include "BitfieldMan.h"
#include "DefaultDiskWriter.h"
#include "FatalException.h"
#include "FileEntry.h"
#include "A2STR.h"
#include "array_fun.h"
#include "bitfield.h"
#include "DownloadHandlerConstants.h"
#include "RequestGroup.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Option.h"
#include "DownloadContext.h"
#include "BufferedFile.h"
#include "SocketCore.h"
#include "prefs.h"

#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigest.h"
# include "message_digest_helper.h"
#endif // ENABLE_MESSAGE_DIGEST

// For libc6 which doesn't define ULLONG_MAX properly because of broken limits.h
#ifndef ULLONG_MAX
# define ULLONG_MAX 18446744073709551615ULL
#endif // ULLONG_MAX

namespace aria2 {

#ifdef __MINGW32__
namespace {
int utf8ToWChar(wchar_t* out, size_t outLength, const char* src)
{
  return MultiByteToWideChar(CP_UTF8, 0, src, -1, out, outLength);
}
} // namespace

namespace {
int ansiToWChar(wchar_t* out, size_t outLength, const char* src)
{
  return MultiByteToWideChar(CP_ACP, 0, src, -1, out, outLength);
}
} // namespace

namespace {
int wCharToUtf8(char* out, size_t outLength, const wchar_t* src)
{
  return WideCharToMultiByte(CP_UTF8, 0, src, -1, out, outLength, 0, 0);
}
} // namespace

namespace {
int wCharToAnsi(char* out, size_t outLength, const wchar_t* src)
{
  return WideCharToMultiByte(CP_ACP, 0, src, -1, out, outLength, 0, 0);
}
} // namespace

std::wstring utf8ToWChar(const char* src)
{
  int len = utf8ToWChar(0, 0, src);
  if(len == 0) {
    abort();
  }
  array_ptr<wchar_t> buf(new wchar_t[len]);
  len = utf8ToWChar(buf, len, src);
  if(len == 0) {
    abort();
  } else {
    std::wstring dest(buf);
    return dest;
  }
}

std::wstring utf8ToWChar(const std::string& src)
{
  return utf8ToWChar(src.c_str());
}

std::string utf8ToNative(const std::string& src)
{
  std::wstring wsrc = utf8ToWChar(src);
  int len = wCharToAnsi(0, 0, wsrc.c_str());
  if(len == 0) {
    abort();
  }
  array_ptr<char> buf(new char[len]);
  len = wCharToAnsi(buf, len, wsrc.c_str());
  if(len == 0) {
    abort();
  } else {
    std::string dest(buf);
    return dest;
  }
}

std::string wCharToUtf8(const std::wstring& wsrc)
{
  int len = wCharToUtf8(0, 0, wsrc.c_str());
  if(len == 0) {
    abort();
  }
  array_ptr<char> buf(new char[len]);
  len = wCharToUtf8(buf, len, wsrc.c_str());
  if(len == 0) {
    abort();
  } else {
    std::string dest(buf);
    return dest;
  }
}

std::string nativeToUtf8(const std::string& src)
{
  int len = ansiToWChar(0, 0, src.c_str());
  if(len == 0) {
    abort();
  }
  array_ptr<wchar_t> buf(new wchar_t[len]);
  len = ansiToWChar(buf, len, src.c_str());
  if(len == 0) {
    abort();
  } else {
    return wCharToUtf8(std::wstring(buf));
  }
}
#endif // __MINGW32__

namespace util {

const char DEFAULT_STRIP_CHARSET[] = "\r\n\t ";

std::string strip(const std::string& str, const char* chars)
{
  std::pair<std::string::const_iterator,
            std::string::const_iterator> p =
    stripIter(str.begin(), str.end(), chars);
  return std::string(p.first, p.second);
}

std::string itos(int64_t value, bool comma)
{
  bool flag = false;
  std::string str;
  if(value < 0) {
    if(value == INT64_MIN) {
      if(comma) {
        str = "-9,223,372,036,854,775,808";
      } else {
        str = "-9223372036854775808";
      }
      return str;
    }
    flag = true;
    value = -value;
  }
  str = uitos(value, comma);
  if(flag) {
    str.insert(str.begin(), '-');
  }
  return str;
}

int64_t difftv(struct timeval tv1, struct timeval tv2) {
  if((tv1.tv_sec < tv2.tv_sec) ||
     ((tv1.tv_sec == tv2.tv_sec) && (tv1.tv_usec < tv2.tv_usec))) {
    return 0;
  }
  return ((int64_t)(tv1.tv_sec-tv2.tv_sec)*1000000+
          tv1.tv_usec-tv2.tv_usec);
}

int32_t difftvsec(struct timeval tv1, struct timeval tv2) {
  if(tv1.tv_sec < tv2.tv_sec) {
    return 0;
  }
  return tv1.tv_sec-tv2.tv_sec;
}

std::string replace(const std::string& target, const std::string& oldstr, const std::string& newstr) {
  if(target.empty() || oldstr.empty()) {
    return target;
  }
  std::string result;
  std::string::size_type p = 0;
  std::string::size_type np = target.find(oldstr);
  while(np != std::string::npos) {
    result.append(target.begin()+p, target.begin()+np);
    result += newstr;
    p = np+oldstr.size();
    np = target.find(oldstr, p);
  }
  result.append(target.begin()+p, target.end());
  return result;
}

bool isAlpha(const char c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool isDigit(const char c)
{
  return '0' <= c && c <= '9';
}

bool isHexDigit(const char c)
{
  return isDigit(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

bool isHexDigit(const std::string& s)
{
  for(std::string::const_iterator i = s.begin(), eoi = s.end(); i != eoi; ++i) {
    if(!isHexDigit(*i)) {
      return false;
    }
  }
  return true;
}

bool inRFC3986ReservedChars(const char c)
{
  static const char reserved[] = {
    ':' , '/' , '?' , '#' , '[' , ']' , '@',
    '!' , '$' , '&' , '\'' , '(' , ')',
    '*' , '+' , ',' , ';' , '=' };
  return std::find(vbegin(reserved), vend(reserved), c) != vend(reserved);
}

bool inRFC3986UnreservedChars(const char c)
{
  static const char unreserved[] = { '-', '.', '_', '~' };
  return isAlpha(c) || isDigit(c) ||
    std::find(vbegin(unreserved), vend(unreserved), c) != vend(unreserved);
}

bool inRFC2978MIMECharset(const char c)
{
  static const char chars[] = {
    '!', '#', '$', '%', '&',
    '\'', '+', '-', '^', '_',
    '`', '{', '}', '~'
  };
  return isAlpha(c) || isDigit(c) ||
    std::find(vbegin(chars), vend(chars), c) != vend(chars);
}

bool inRFC2616HttpToken(const char c)
{
  static const char chars[] = {
    '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.',
    '^', '_', '`', '|', '~'
  };
  return isAlpha(c) || isDigit(c) ||
    std::find(vbegin(chars), vend(chars), c) != vend(chars);
}

bool inRFC5987AttrChar(const char c)
{
  return inRFC2616HttpToken(c) && c != '*' && c != '\'' && c != '%';
}

// Returns nonzero if |c| is in ISO/IEC 8859-1 character set.
bool isIso8859p1(unsigned char c)
{
  return (0x20u <= c && c <= 0x7eu) || 0xa0u <= c;
}

bool isLws(const char c)
{
  return c == ' ' || c == '\t';
}
bool isCRLF(const char c)
{
  return c == '\r' || c == '\n';
}

namespace {
bool isUtf8Tail(unsigned char ch)
{
  return in(ch, 0x80u, 0xbfu);
}
} // namespace

bool isUtf8(const std::string& str)
{
  for(std::string::const_iterator s = str.begin(), eos = str.end(); s != eos;
      ++s) {
    unsigned char firstChar = *s;
    // See ABNF in http://tools.ietf.org/search/rfc3629#section-4
    if(in(firstChar, 0x20u, 0x7eu) ||
       firstChar == 0x08u || // \b
       firstChar == 0x09u || // \t
       firstChar == 0x0au || // \n
       firstChar == 0x0cu || // \f
       firstChar == 0x0du    // \r
       ) {
      // UTF8-1 (without ctrl chars)
    } else if(in(firstChar, 0xc2u, 0xdfu)) {
      // UTF8-2
      if(++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else if(0xe0u == firstChar) {
      // UTF8-3
      if(++s == eos || !in(static_cast<unsigned char>(*s), 0xa0u, 0xbfu) ||
         ++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else if(in(firstChar, 0xe1u, 0xecu) || in(firstChar, 0xeeu, 0xefu)) {
      // UTF8-3
      if(++s == eos || !isUtf8Tail(*s) ||
         ++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else if(0xedu == firstChar) {
      // UTF8-3
      if(++s == eos || !in(static_cast<unsigned char>(*s), 0x80u, 0x9fu) ||
         ++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else if(0xf0u == firstChar) {
      // UTF8-4
      if(++s == eos || !in(static_cast<unsigned char>(*s), 0x90u, 0xbfu) ||
         ++s == eos || !isUtf8Tail(*s) ||
         ++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else if(in(firstChar, 0xf1u, 0xf3u)) {
      // UTF8-4
      if(++s == eos || !isUtf8Tail(*s) ||
         ++s == eos || !isUtf8Tail(*s) ||
         ++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else if(0xf4u == firstChar) {
      // UTF8-4
      if(++s == eos || !in(static_cast<unsigned char>(*s), 0x80u, 0x8fu) ||
         ++s == eos || !isUtf8Tail(*s) ||
         ++s == eos || !isUtf8Tail(*s)) {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

std::string percentEncode(const unsigned char* target, size_t len)
{
  std::string dest;
  for(size_t i = 0; i < len; ++i) {
    if(inRFC3986UnreservedChars(target[i])) {
      dest += target[i];
    } else {
      dest.append(fmt("%%%02X", target[i]));
    }
  }
  return dest;
}

std::string percentEncode(const std::string& target)
{
  return percentEncode(reinterpret_cast<const unsigned char*>(target.c_str()),
                       target.size());
}

std::string percentEncodeMini(const std::string& src)
{
  std::string result;
  for(std::string::const_iterator i = src.begin(), eoi = src.end(); i != eoi;
      ++i) {
    // Non-Printable ASCII and non-ASCII chars + some ASCII chars.
    unsigned char c = *i;
    if(in(c, 0x00u, 0x20u) || c >= 0x7fu ||
       // Chromium escapes following characters. Firefox4 escapes
       // more.
       c == '"' || c == '<' || c == '>') {
      result += fmt("%%%02X", c);
    } else {
      result += c;
    }
  }
  return result;
}

std::string torrentPercentEncode(const unsigned char* target, size_t len) {
  std::string dest;
  for(size_t i = 0; i < len; ++i) {
    if(isAlpha(target[i]) || isDigit(target[i])) {
      dest += target[i];
    } else {
      dest.append(fmt("%%%02X", target[i]));
    }
  }
  return dest;
}

std::string torrentPercentEncode(const std::string& target)
{
  return torrentPercentEncode
    (reinterpret_cast<const unsigned char*>(target.c_str()), target.size());
}

std::string percentDecode
(std::string::const_iterator first, std::string::const_iterator last)
{
  std::string result;
  for(; first != last; ++first) {
    if(*first == '%') {
      if(first+1 != last && first+2 != last &&
         isHexDigit(*(first+1)) && isHexDigit(*(first+2))) {
        result += hexCharToUInt(*(first+1))*16+hexCharToUInt(*(first+2));
        first += 2;
      } else {
        result += *first;
      }
    } else {
      result += *first;
    }
  }
  return result;
}

std::string toHex(const unsigned char* src, size_t len) {
  std::string out(len*2, '\0');
  std::string::iterator o = out.begin();
  const unsigned char* last = src+len;
  for(const unsigned char* i = src; i != last; ++i) {
    *o = (*i >> 4);
    *(o+1) = (*i)&0x0fu;
    for(int j = 0; j < 2; ++j) {
      if(*o < 10) {
        *o += '0';
      } else {
        *o += 'a'-10;
      }
      ++o;
    }
  }
  return out;
}

std::string toHex(const char* src, size_t len)
{
  return toHex(reinterpret_cast<const unsigned char*>(src), len);
}

std::string toHex(const std::string& src)
{
  return toHex(reinterpret_cast<const unsigned char*>(src.c_str()), src.size());
}

unsigned int hexCharToUInt(unsigned char ch)
{
  if('a' <= ch && ch <= 'f') {
    ch -= 'a';
    ch += 10;
  } else if('A' <= ch && ch <= 'F') {
    ch -= 'A';
    ch += 10;
  } else if('0' <= ch && ch <= '9') {
    ch -= '0';
  } else {
    ch = 255;
  }
  return ch;
}

std::string secfmt(time_t sec) {
  time_t tsec = sec;
  std::string str;
  if(sec >= 3600) {
    str = fmt("%" PRId64 "h", static_cast<int64_t>(sec/3600));
    sec %= 3600;
  }
  if(sec >= 60) {
    str += fmt("%dm", static_cast<int>(sec/60));
    sec %= 60;
  }
  if(sec || tsec == 0) {
    str += fmt("%ds", static_cast<int>(sec));
  }
  return str;
}

namespace {
template<typename T, typename F>
bool parseLong(T& res, F f, const std::string& s, int base)
{
  if(s.empty()) {
    return false;
  }
  char* endptr;
  errno = 0;
  res = f(s.c_str(), &endptr, base);
  if(errno == ERANGE) {
    return false;
  }
  if(*endptr != '\0') {
    for(const char* i = endptr, *eoi = s.c_str()+s.size(); i < eoi; ++i) {
      if(!isspace(*i)) {
        return false;
      }
    }
  }
  return true;
}
} // namespace

bool parseIntNoThrow(int32_t& res, const std::string& s, int base)
{
  long int t;
  if(parseLong(t, strtol, s, base) &&
     t >= std::numeric_limits<int32_t>::min() &&
     t <= std::numeric_limits<int32_t>::max()) {
    res = t;
    return true;
  } else {
    return false;
  }
}

bool parseUIntNoThrow(uint32_t& res, const std::string& s, int base)
{
  long int t;
  if(parseLong(t, strtol, s, base) &&
     t >= 0 &&
     t <= std::numeric_limits<int32_t>::max()) {
    res = t;
    return true;
  } else {
    return false;
  }
}

bool parseLLIntNoThrow(int64_t& res, const std::string& s, int base)
{
  long long int t;
  if(parseLong(t, strtoll, s, base) &&
     t >= std::numeric_limits<int64_t>::min() &&
     t <= std::numeric_limits<int64_t>::max()) {
    res = t;
    return true;
  } else {
    return false;
  }
}

void parseIntSegments(SegList<int>& sgl, const std::string& src)
{
  for(std::string::const_iterator i = src.begin(), eoi = src.end(); i != eoi;) {
    std::string::const_iterator j = std::find(i, eoi, ',');
    if(j == i) {
      ++i;
      continue;
    }
    std::string::const_iterator p = std::find(i, j, '-');
    if(p == j) {
      int a;
      if(parseIntNoThrow(a, std::string(i, j))) {
        sgl.add(a, a+1);
      } else {
        throw DL_ABORT_EX(fmt("Bad range %s", std::string(i, j).c_str()));
      }
    } else if(p == i || p+1 == j) {
      throw DL_ABORT_EX(fmt(MSG_INCOMPLETE_RANGE, std::string(i, j).c_str()));
    } else {
      int a, b;
      if(parseIntNoThrow(a, std::string(i, p)) &&
         parseIntNoThrow(b, (std::string(p+1, j)))) {
        sgl.add(a, b+1);
      } else {
        throw DL_ABORT_EX(fmt("Bad range %s", std::string(i, j).c_str()));
      }
    }
    if(j == eoi) {
      break;
    }
    i = j+1;
  }
}

namespace {
void computeHeadPieces
(std::vector<size_t>& indexes,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 int64_t head)
{
  if(head == 0) {
    return;
  }
  for(std::vector<SharedHandle<FileEntry> >::const_iterator fi =
        fileEntries.begin(), eoi = fileEntries.end(); fi != eoi; ++fi) {
    if((*fi)->getLength() == 0) {
      continue;
    }
    size_t lastIndex =
      ((*fi)->getOffset()+std::min(head, (*fi)->getLength())-1)/pieceLength;
    for(size_t index = (*fi)->getOffset()/pieceLength;
        index <= lastIndex; ++index) {
      indexes.push_back(index);
    }
  }
}
} // namespace

namespace {
void computeTailPieces
(std::vector<size_t>& indexes,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 int64_t tail)
{
  if(tail == 0) {
    return;
  }
  for(std::vector<SharedHandle<FileEntry> >::const_iterator fi =
        fileEntries.begin(), eoi = fileEntries.end(); fi != eoi; ++fi) {
    if((*fi)->getLength() == 0) {
      continue;
    }
    int64_t endOffset = (*fi)->getLastOffset();
    size_t fromIndex =
      (endOffset-1-(std::min(tail, (*fi)->getLength())-1))/pieceLength;
    for(size_t index = fromIndex; index <= (endOffset-1)/pieceLength;
        ++index) {
      indexes.push_back(index);
    }
  }
}
} // namespace

void parsePrioritizePieceRange
(std::vector<size_t>& result, const std::string& src,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 int64_t defaultSize)
{
  std::vector<size_t> indexes;
  std::vector<Scip> parts;
  splitIter(src.begin(), src.end(), std::back_inserter(parts), ',', true);
  for(std::vector<Scip>::const_iterator i = parts.begin(),
        eoi = parts.end(); i != eoi; ++i) {
    if(util::streq((*i).first, (*i).second, "head")) {
      computeHeadPieces(indexes, fileEntries, pieceLength, defaultSize);
    } else if(util::startsWith((*i).first, (*i).second, "head=")) {
      std::string sizestr((*i).first+5, (*i).second);
      computeHeadPieces(indexes, fileEntries, pieceLength,
                        std::max((int64_t)0, getRealSize(sizestr)));
    } else if(util::streq((*i).first, (*i).second, "tail")) {
      computeTailPieces(indexes, fileEntries, pieceLength, defaultSize);
    } else if(util::startsWith((*i).first, (*i).second, "tail=")) {
      std::string sizestr((*i).first+5, (*i).second);
      computeTailPieces(indexes, fileEntries, pieceLength,
                        std::max((int64_t)0, getRealSize(sizestr)));
    } else {
      throw DL_ABORT_EX(fmt("Unrecognized token %s",
                            std::string((*i).first, (*i).second).c_str()));
    }
  }
  std::sort(indexes.begin(), indexes.end());
  indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());
  result.insert(result.end(), indexes.begin(), indexes.end());
}

// Converts ISO/IEC 8859-1 string to UTF-8 string.  If there is a
// character not in ISO/IEC 8859-1, returns empty string.
std::string iso8859p1ToUtf8(const char* src, size_t len)
{
  std::string dest;
  for(const char* p = src, *last = src+len; p != last; ++p) {
    unsigned char c = *p;
    if(0xa0u <= c) {
      if(c <= 0xbfu) {
        dest += 0xc2u;
      } else {
        dest += 0xc3u;
      }
      dest += c&(~0x40u);
    } else if(0x80u <= c && c <= 0x9fu) {
      return "";
    } else {
      dest += c;
    }
  }
  return dest;
}

std::string iso8859p1ToUtf8(const std::string& src)
{
  return iso8859p1ToUtf8(src.c_str(), src.size());
}

/* Start of utf8 dfa */
/* Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
 * See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
 *
 * Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const uint8_t utf8d[] = {
  /*
   * The first part of the table maps bytes to character classes that
   * to reduce the size of the transition table and create bitmasks.
   */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

   /*
    * The second part is a transition table that maps a combination
    * of a state of the automaton and a character class to a state.
    */
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12,
};

static uint32_t
utf8dfa(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

/* End of utf8 dfa */

typedef enum {
  CD_BEFORE_DISPOSITION_TYPE,
  CD_AFTER_DISPOSITION_TYPE,
  CD_DISPOSITION_TYPE,
  CD_BEFORE_DISPOSITION_PARM_NAME,
  CD_AFTER_DISPOSITION_PARM_NAME,
  CD_DISPOSITION_PARM_NAME,
  CD_BEFORE_VALUE,
  CD_AFTER_VALUE,
  CD_QUOTED_STRING,
  CD_TOKEN,
  CD_BEFORE_EXT_VALUE,
  CD_CHARSET,
  CD_LANGUAGE,
  CD_VALUE_CHARS,
  CD_VALUE_CHARS_PCT_ENCODED1,
  CD_VALUE_CHARS_PCT_ENCODED2
} content_disposition_parse_state;

typedef enum {
  CD_FILENAME_FOUND = 1,
  CD_EXT_FILENAME_FOUND = 1 << 1
} content_disposition_parse_flag;

typedef enum {
  CD_ENC_UNKNOWN,
  CD_ENC_UTF8,
  CD_ENC_ISO_8859_1
} content_disposition_charset;

ssize_t parse_content_disposition(char *dest, size_t destlen,
                                  const char **charsetp, size_t *charsetlenp,
                                  const char *in, size_t len)
{
  const char *p = in, *eop = in + len, *mark_first = NULL, *mark_last = NULL;
  int state = CD_BEFORE_DISPOSITION_TYPE;
  int in_file_parm = 0;
  int flags = 0;
  int quoted_seen = 0;
  int charset = 0;
  /* To suppress warnings */
  char *dp = dest;
  size_t dlen = destlen;
  uint32_t dfa_state = 0;
  uint32_t dfa_code = 0;
  uint8_t pctval = 0;

  *charsetp = NULL;
  *charsetlenp = 0;

  for(; p != eop; ++p) {
    switch(state) {
    case CD_BEFORE_DISPOSITION_TYPE:
      if(inRFC2616HttpToken(*p)) {
        state = CD_DISPOSITION_TYPE;
      } else if(!isLws(*p)) {
        return -1;
      }
      break;
    case CD_AFTER_DISPOSITION_TYPE:
    case CD_DISPOSITION_TYPE:
      if(*p == ';') {
        state = CD_BEFORE_DISPOSITION_PARM_NAME;
      } else if(isLws(*p)) {
        state = CD_AFTER_DISPOSITION_TYPE;
      } else if(state == CD_AFTER_DISPOSITION_TYPE ||
                !inRFC2616HttpToken(*p)) {
        return -1;
      }
      break;
    case CD_BEFORE_DISPOSITION_PARM_NAME:
      if(inRFC2616HttpToken(*p)) {
        mark_first = p;
        state = CD_DISPOSITION_PARM_NAME;
      } else if(!isLws(*p)) {
        return -1;
      }
      break;
    case CD_AFTER_DISPOSITION_PARM_NAME:
    case CD_DISPOSITION_PARM_NAME:
      if(*p == '=') {
        if(state == CD_DISPOSITION_PARM_NAME) {
          mark_last = p;
        }
        in_file_parm = 0;
        if(strieq(mark_first, mark_last, "filename*")) {
          if((flags & CD_EXT_FILENAME_FOUND) == 0) {
            in_file_parm = 1;
          } else {
            return -1;
          }
          state = CD_BEFORE_EXT_VALUE;
        } else if(strieq(mark_first, mark_last, "filename")) {
          if(flags & CD_FILENAME_FOUND) {
            return -1;
          }
          if((flags & CD_EXT_FILENAME_FOUND) == 0) {
            in_file_parm = 1;
          }
          state = CD_BEFORE_VALUE;
        } else {
          /* ext-token must be characters in token, followed by "*" */
          if(mark_first != mark_last-1 && *(mark_last-1) == '*') {
            state = CD_BEFORE_EXT_VALUE;
          } else {
            state = CD_BEFORE_VALUE;
          }
        }
        if(in_file_parm) {
          dp = dest;
          dlen = destlen;
        }
      } else if(isLws(*p)) {
        mark_last = p;
        state = CD_AFTER_DISPOSITION_PARM_NAME;
      } else if(state == CD_AFTER_DISPOSITION_PARM_NAME ||
                !inRFC2616HttpToken(*p)) {
        return -1;
      }
      break;
    case CD_BEFORE_VALUE:
      if(*p == '"') {
        quoted_seen = 0;
        state = CD_QUOTED_STRING;
      } else if(inRFC2616HttpToken(*p)) {
        if(in_file_parm) {
          if(dlen == 0) {
            return -1;
          } else {
            *dp++ = *p;
            --dlen;
          }
        }
        state = CD_TOKEN;
      } else if(!isLws(*p)) {
        return -1;
      }
      break;
    case CD_AFTER_VALUE:
      if(*p == ';') {
        state = CD_BEFORE_DISPOSITION_PARM_NAME;
      } else if(!isLws(*p)) {
        return -1;
      }
      break;
    case CD_QUOTED_STRING:
      if(*p == '\\' && quoted_seen == 0) {
        quoted_seen = 1;
      } else if(*p == '"' && quoted_seen == 0) {
        if(in_file_parm) {
          flags |= CD_FILENAME_FOUND;
        }
        state = CD_AFTER_VALUE;
      } else {
        /* TEXT which is OCTET except CTLs, but including LWS. We only
           accept ISO-8859-1 chars. */
        quoted_seen = 0;
        if(!isIso8859p1(*p)) {
          return -1;
        }
        if(in_file_parm) {
          if(dlen == 0) {
            return -1;
          } else {
            *dp++ = *p;
            --dlen;
          }
        }
      }
      break;
    case CD_TOKEN:
      if(inRFC2616HttpToken(*p)) {
        if(in_file_parm) {
          if(dlen == 0) {
            return -1;
          } else {
            *dp++ = *p;
            --dlen;
          }
        }
      } else if(*p == ';') {
        if(in_file_parm) {
          flags |= CD_FILENAME_FOUND;
        }
        state = CD_BEFORE_DISPOSITION_PARM_NAME;
      } else if(isLws(*p)) {
        if(in_file_parm) {
          flags |= CD_FILENAME_FOUND;
        }
        state = CD_AFTER_VALUE;
      } else {
        return -1;
      }
      break;
    case CD_BEFORE_EXT_VALUE:
      if(*p == '\'') {
        /* Empty charset is not allowed */
        return -1;
      } else if(inRFC2978MIMECharset(*p)) {
        mark_first = p;
        state = CD_CHARSET;
      } else if(!isLws(*p)) {
        return -1;
      }
      break;
    case CD_CHARSET:
      if(*p == '\'') {
        mark_last = p;
        *charsetp = mark_first;
        *charsetlenp = mark_last - mark_first;
        if(strieq(mark_first, mark_last, "utf-8")) {
          charset = CD_ENC_UTF8;
          dfa_state = UTF8_ACCEPT;
          dfa_code = 0;
        } else if(strieq(mark_first, mark_last, "iso-8859-1")) {
          charset = CD_ENC_ISO_8859_1;
        } else {
          charset = CD_ENC_UNKNOWN;
        }
        state = CD_LANGUAGE;
      } else if(!inRFC2978MIMECharset(*p)) {
        return -1;
      }
      break;
    case CD_LANGUAGE:
      if(*p == '\'') {
        if(in_file_parm) {
          dp = dest;
          dlen = destlen;
        }
        state = CD_VALUE_CHARS;
      } else if(*p != '-' && !isAlpha(*p) && !isDigit(*p)) {
        return -1;
      }
      break;
    case CD_VALUE_CHARS:
      if(inRFC5987AttrChar(*p)) {
        if(charset == CD_ENC_UTF8) {
          if(utf8dfa(&dfa_state, &dfa_code, *p) == UTF8_REJECT) {
            return -1;
          }
        }
        if(in_file_parm) {
          if(dlen == 0) {
            return -1;
          } else {
            *dp++ = *p;
            --dlen;
          }
        }
      } else if(*p == '%') {
        if(in_file_parm) {
          if(dlen == 0) {
            return -1;
          }
        }
        pctval = 0;
        state = CD_VALUE_CHARS_PCT_ENCODED1;
      } else if(*p == ';' || isLws(*p)) {
        if(charset == CD_ENC_UTF8 && dfa_state != UTF8_ACCEPT) {
          return -1;
        }
        if(in_file_parm) {
          flags |= CD_EXT_FILENAME_FOUND;
        }
        if(*p == ';') {
          state = CD_BEFORE_DISPOSITION_PARM_NAME;
        } else {
          state = CD_AFTER_VALUE;
        }
      } else if(!inRFC5987AttrChar(*p)) {
        return -1;
      }
      break;
    case CD_VALUE_CHARS_PCT_ENCODED1:
      if(isHexDigit(*p)) {
        pctval |= hexCharToUInt(*p) << 4;
        state = CD_VALUE_CHARS_PCT_ENCODED2;
      } else {
        return -1;
      }
      break;
    case CD_VALUE_CHARS_PCT_ENCODED2:
      if(isHexDigit(*p)) {
        pctval |= hexCharToUInt(*p);
        if(charset == CD_ENC_UTF8) {
          if(utf8dfa(&dfa_state, &dfa_code, pctval) == UTF8_REJECT) {
            return -1;
          }
        } else if(charset == CD_ENC_ISO_8859_1) {
          if(!isIso8859p1(pctval)) {
            return -1;
          }
        }
        if(in_file_parm) {
          *dp++ = pctval;
          --dlen;
        }
        state = CD_VALUE_CHARS;
      } else {
        return -1;
      }
      break;
    }
  }
  switch(state) {
  case CD_BEFORE_DISPOSITION_TYPE:
  case CD_AFTER_DISPOSITION_TYPE:
  case CD_DISPOSITION_TYPE:
  case CD_AFTER_VALUE:
  case CD_TOKEN:
    return destlen-dlen;
  case CD_VALUE_CHARS:
    if(charset == CD_ENC_UTF8 && dfa_state != UTF8_ACCEPT) {
      return -1;
    }
    return destlen - dlen;
  default:
    return -1;
  }
}

std::string getContentDispositionFilename(const std::string& header)
{
  char cdval[1024];
  size_t cdvallen = sizeof(cdval);
  const char* charset;
  size_t charsetlen;
  ssize_t rv = parse_content_disposition(cdval, cdvallen,
                                         &charset, &charsetlen,
                                         header.c_str(), header.size());
  if(rv == -1) {
    return "";
  } else {
    std::string res;
    if(!charset || strieq(charset, charset+charsetlen, "iso-8859-1")) {
      res = iso8859p1ToUtf8(cdval, rv);
    } else {
      res.assign(cdval, rv);
    }
    if(!detectDirTraversal(res) &&
       res.find_first_of("/\\") == std::string::npos) {
      return res;
    } else {
      return "";
    }
  }
}

std::string toUpper(const std::string& src) {
  std::string temp = src;
  uppercase(temp);
  return temp;
}

std::string toLower(const std::string& src) {
  std::string temp = src;
  lowercase(temp);
  return temp;
}

void uppercase(std::string& s)
{
  std::transform(s.begin(), s.end(), s.begin(), toUpperChar);
}

void lowercase(std::string& s)
{
  std::transform(s.begin(), s.end(), s.begin(), toLowerChar);
}

char toUpperChar(char c)
{
  if('a' <= c && c <= 'z') {
    c += 'A'-'a';
  }
  return c;
}

char toLowerChar(char c)
{
  if('A' <= c && c <= 'Z') {
    c += 'a'-'A';
  }
  return c;
}

bool isNumericHost(const std::string& name)
{
  struct addrinfo hints;
  struct addrinfo* res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_NUMERICHOST;
  if(getaddrinfo(name.c_str(), 0, &hints, &res)) {
    return false;
  }
  freeaddrinfo(res);
  return true;
}

void setGlobalSignalHandler(int sig, sigset_t* mask, void (*handler)(int),
                            int flags) {
#ifdef HAVE_SIGACTION
  struct sigaction sigact;
  sigact.sa_handler = handler;
  sigact.sa_flags = flags;
  sigact.sa_mask = *mask;
  sigaction(sig, &sigact, NULL);
#else
  signal(sig, handler);
#endif // HAVE_SIGACTION
}

std::string getHomeDir()
{
  const char* p = getenv("HOME");
  if(p) {
    return p;
  } else {
#ifdef __MINGW32__
    p = getenv("USERPROFILE");
    if(p) {
      return p;
    } else {
      p = getenv("HOMEDRIVE");
      if(p) {
        std::string homeDir = p;
        p = getenv("HOMEPATH");
        if(p) {
          homeDir += p;
          return homeDir;
        }
      }
    }
#elif HAVE_PWD_H
    passwd* pw = getpwuid(geteuid());
    if(pw && pw->pw_dir) {
      return pw->pw_dir;
    }
#endif // HAVE_PWD_H
    return A2STR::NIL;
  }
}

int64_t getRealSize(const std::string& sizeWithUnit)
{
  std::string::size_type p = sizeWithUnit.find_first_of("KMkm");
  std::string size;
  int32_t mult = 1;
  if(p == std::string::npos) {
    size = sizeWithUnit;
  } else {
    switch(sizeWithUnit[p]) {
    case 'K':
    case 'k':
      mult = 1024;
      break;
    case 'M':
    case 'm':
      mult = 1024*1024;
      break;
    }
    size.assign(sizeWithUnit.begin(), sizeWithUnit.begin()+p);
  }
  int64_t v;
  if(!parseLLIntNoThrow(v, size) || v < 0) {
    throw DL_ABORT_EX(fmt("Bad or negative value detected: %s",
                          sizeWithUnit.c_str()));
  } else if(INT64_MAX/mult < v) {
    throw DL_ABORT_EX(fmt(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                          "overflow/underflow"));
  }
  return v*mult;
}

std::string abbrevSize(int64_t size)
{
  static const char* UNITS[] = { "", "Ki", "Mi", "Gi" };
  int64_t t = size;
  size_t uidx = 0;
  int r = 0;
  while(t >= 1024 && uidx+1 < sizeof(UNITS)/sizeof(UNITS[0])) {
    lldiv_t d = lldiv(t, 1024);
    t = d.quot;
    r = d.rem;
    ++uidx;
  }
  if(uidx+1 < sizeof(UNITS)/sizeof(UNITS[0]) && t >= 922) {
    ++uidx;
    r = t;
    t = 0;
  }
  std::string res;
  res += itos(t, true);
  if(t < 10 && uidx > 0) {
    res += ".";
    res += itos(r*10/1024);
  }
  res += UNITS[uidx];
  return res;
}

void sleep(long seconds) {
#if defined(HAVE_WINSOCK2_H)
  ::Sleep(seconds * 1000);
#elif HAVE_SLEEP
  ::sleep(seconds);
#elif defined(HAVE_USLEEP)
  ::usleep(seconds * 1000000);
#else
#  error no sleep function is available (nanosleep?)
#endif
}

void usleep(long microseconds) {
#ifdef HAVE_USLEEP
  ::usleep(microseconds);
#elif defined(HAVE_WINSOCK2_H)

  LARGE_INTEGER current, freq, end;

  static enum {GET_FREQUENCY, GET_MICROSECONDS, SKIP_MICROSECONDS} state = GET_FREQUENCY;

  if (state == GET_FREQUENCY) {
    if (QueryPerformanceFrequency(&freq))
      state = GET_MICROSECONDS;
    else
      state = SKIP_MICROSECONDS;
  }

  long msec = microseconds / 1000;
  microseconds %= 1000;

  if (state == GET_MICROSECONDS && microseconds) {
    QueryPerformanceCounter(&end);

    end.QuadPart += (freq.QuadPart * microseconds) / 1000000;

    while (QueryPerformanceCounter(&current) && (current.QuadPart <= end.QuadPart))
      /* noop */ ;
  }

  if (msec)
    Sleep(msec);
#else
#error no usleep function is available (nanosleep?)
#endif
}

void mkdirs(const std::string& dirpath)
{
  File dir(dirpath);
  if(!dir.mkdirs()) {
    int errNum = errno;
    if(!dir.isDir()) {
      throw DL_ABORT_EX3
        (errNum,
         fmt(EX_MAKE_DIR, dir.getPath().c_str(),
             safeStrerror(errNum).c_str()),
         error_code::DIR_CREATE_ERROR);
    }
  }
}

void convertBitfield(BitfieldMan* dest, const BitfieldMan* src)
{
  size_t numBlock = dest->countBlock();
  for(size_t index = 0; index < numBlock; ++index) {
    if(src->isBitSetOffsetRange((int64_t)index*dest->getBlockLength(),
                                dest->getBlockLength())) {
      dest->setBit(index);
    }
  }
}

std::string toString(const SharedHandle<BinaryStream>& binaryStream)
{
  std::stringstream strm;
  char data[2048];
  while(1) {
    int32_t dataLength = binaryStream->readData
      (reinterpret_cast<unsigned char*>(data), sizeof(data), strm.tellp());
    strm.write(data, dataLength);
    if(dataLength == 0) {
      break;
    }
  }
  return strm.str();
}

#ifdef HAVE_POSIX_MEMALIGN
/**
 * In linux 2.6, alignment and size should be a multiple of 512.
 */
void* allocateAlignedMemory(size_t alignment, size_t size)
{
  void* buffer;
  int res;
  if((res = posix_memalign(&buffer, alignment, size)) != 0) {
    throw FATAL_EXCEPTION
      (fmt("Error in posix_memalign: %s",
           util::safeStrerror(res).c_str()));
  }
  return buffer;
}
#endif // HAVE_POSIX_MEMALIGN

std::pair<std::string, uint16_t>
getNumericNameInfo(const struct sockaddr* sockaddr, socklen_t len)
{
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  int s = getnameinfo(sockaddr, len, host, NI_MAXHOST, service, NI_MAXSERV,
                      NI_NUMERICHOST|NI_NUMERICSERV);
  if(s != 0) {
    throw DL_ABORT_EX(fmt("Failed to get hostname and port. cause: %s",
                          gai_strerror(s)));
  }
  return std::pair<std::string, uint16_t>(host, atoi(service)); // TODO
}

std::string htmlEscape(const std::string& src)
{
  std::string dest;
  for(std::string::const_iterator i = src.begin(), eoi = src.end();
      i != eoi; ++i) {
    char ch = *i;
    if(ch == '<') {
      dest += "&lt;";
    } else if(ch == '>') {
      dest += "&gt;";
    } else if(ch == '&') {
      dest += "&amp;";
    } else if(ch == '\'') {
      dest += "&#39;";
    } else if(ch == '"') {
      dest += "&quot;";
    } else {
      dest += ch;
    }
  }
  return dest;
}

std::pair<size_t, std::string>
parseIndexPath(const std::string& line)
{
  std::pair<Scip, Scip> p;
  divide(p, line.begin(), line.end(), '=');
  uint32_t index;
  if(!parseUIntNoThrow(index, std::string(p.first.first, p.first.second))) {
    throw DL_ABORT_EX("Bad path index");
  }
  if(p.second.first == p.second.second) {
    throw DL_ABORT_EX(fmt("Path with index=%u is empty.", index));
  }
  return std::make_pair(index, std::string(p.second.first, p.second.second));
}

std::vector<std::pair<size_t, std::string> > createIndexPaths(std::istream& i)
{
  std::vector<std::pair<size_t, std::string> > indexPaths;
  std::string line;
  while(getline(i, line)) {
    indexPaths.push_back(parseIndexPath(line));
  }
  return indexPaths;
}

namespace {
void generateRandomDataRandom(unsigned char* data, size_t length)
{
  const SharedHandle<SimpleRandomizer>& rd = SimpleRandomizer::getInstance();
  for(size_t i = 0; i < length; ++i) {
    data[i] = static_cast<unsigned long>(rd->getRandomNumber(256));
  }
}
} // namespace

namespace {
void generateRandomDataUrandom
(unsigned char* data, size_t length, std::ifstream& devUrand)
{
  devUrand.read(reinterpret_cast<char*>(data), length);
}
} // namespace

void generateRandomData(unsigned char* data, size_t length)
{
#ifdef __MINGW32__
  generateRandomDataRandom(data, length);
#else // !__MINGW32__
  static int method = -1;
  static std::ifstream devUrand;
  if(method == 0) {
    generateRandomDataUrandom(data, length, devUrand);
  } else if(method == 1) {
    generateRandomDataRandom(data, length);
  } else {
    devUrand.open("/dev/urandom");
    if(devUrand) {
      method = 0;
    } else {
      method = 1;
    }
    generateRandomData(data, length);
  }
#endif // !__MINGW32__
}

bool saveAs
(const std::string& filename, const std::string& data, bool overwrite)
{
  if(!overwrite && File(filename).exists()) {
    return false;
  }
  std::string tempFilename = filename;
  tempFilename += "__temp";
  {
    BufferedFile fp(tempFilename.c_str(), BufferedFile::WRITE);
    if(!fp) {
      return false;
    }
    if(fp.write(data.data(), data.size()) != data.size()) {
      return false;
    }
    if(fp.close() == EOF) {
      return false;
    }
  }
  return File(tempFilename).renameTo(filename);
}

std::string applyDir(const std::string& dir, const std::string& relPath)
{
  std::string s;
  if(dir.empty()) {
    s = "./";
    s += relPath;
  } else {
    s = dir;
    if(dir == "/") {
      s += relPath;
    } else {
      s += "/";
      s += relPath;
    }
  }
#ifdef __MINGW32__
  for(std::string::iterator i = s.begin(), eoi = s.end(); i != eoi; ++i) {
    if(*i == '\\') {
      *i = '/';
    }
  }
#endif // __MINGW32__
  return s;
}

std::string fixTaintedBasename(const std::string& src)
{
  return escapePath(replace(src, "/", "%2F"));
}

void generateRandomKey(unsigned char* key)
{
#ifdef ENABLE_MESSAGE_DIGEST
  unsigned char bytes[40];
  generateRandomData(bytes, sizeof(bytes));
  message_digest::digest(key, 20, MessageDigest::sha1(), bytes, sizeof(bytes));
#else // !ENABLE_MESSAGE_DIGEST
  generateRandomData(key, 20);
#endif // !ENABLE_MESSAGE_DIGEST
}

// Returns true is given numeric ipv4addr is in Private Address Space.
//
// From Section.3 RFC1918
// 10.0.0.0        -   10.255.255.255  (10/8 prefix)
// 172.16.0.0      -   172.31.255.255  (172.16/12 prefix)
// 192.168.0.0     -   192.168.255.255 (192.168/16 prefix)
bool inPrivateAddress(const std::string& ipv4addr)
{
  if(util::startsWith(ipv4addr, "10.") ||
     util::startsWith(ipv4addr, "192.168.")) {
    return true;
  }
  if(util::startsWith(ipv4addr, "172.")) {
    for(int i = 16; i <= 31; ++i) {
      std::string t(fmt("%d.", i));
      if(util::startsWith(ipv4addr.begin()+4, ipv4addr.end(),
                          t.begin(), t.end())) {
        return true;
      }
    }
  }
  return false;
}

bool detectDirTraversal(const std::string& s)
{
  if(s.empty()) {
    return false;
  }
  for(std::string::const_iterator i = s.begin(), eoi = s.end(); i != eoi; ++i) {
    unsigned char c = *i;
    if(in(c, 0x00u, 0x1fu) || c == 0x7fu) {
      return true;
    }
  }
  return s == "." || s == ".." || s[0] == '/' ||
    util::startsWith(s, "./") || util::startsWith(s, "../") ||
    s.find("/../") != std::string::npos ||
    s.find("/./") != std::string::npos ||
    s[s.size()-1] == '/' ||
    util::endsWith(s, "/.") || util::endsWith(s, "/..");
}

std::string escapePath(const std::string& s)
{
  // We don't escape '/' because we use it as a path separator.
#ifdef __MINGW32__
  static const char WIN_INVALID_PATH_CHARS[] =
    { '"', '*', ':', '<', '>', '?', '\\', '|' };
#endif // __MINGW32__
  std::string d;
  for(std::string::const_iterator i = s.begin(), eoi = s.end(); i != eoi; ++i) {
    unsigned char c = *i;
    if(in(c, 0x00u, 0x1fu) || c == 0x7fu
#ifdef __MINGW32__
       || std::find(vbegin(WIN_INVALID_PATH_CHARS),
                    vend(WIN_INVALID_PATH_CHARS),
                    c) != vend(WIN_INVALID_PATH_CHARS)
#endif // __MINGW32__
       ){
      d += fmt("%%%02X", c);
    } else {
      d += *i;
    }
  }
  return d;
}

bool inSameCidrBlock
(const std::string& ip1, const std::string& ip2, size_t bits)
{
  unsigned char s1[16], s2[16];
  size_t len1, len2;
  if((len1 = net::getBinAddr(s1, ip1)) == 0 ||
     (len2 = net::getBinAddr(s2, ip2)) == 0 ||
     len1 != len2) {
    return false;
  }
  if(bits == 0) {
    return true;
  }
  if(bits > 8*len1) {
    bits = 8*len1;
  }
  int last = (bits-1)/8;
  for(int i = 0; i < last; ++i) {
    if(s1[i] != s2[i]) {
      return false;
    }
  }
  unsigned char mask = bitfield::lastByteMask(bits);
  return (s1[last] & mask) == (s2[last] & mask);
}

namespace {

void executeHook
(const std::string& command,
 a2_gid_t gid,
 size_t numFiles,
 const std::string& firstFilename)
{
  const std::string gidStr = GroupId::toHex(gid);
  const std::string numFilesStr = util::uitos(numFiles);
#ifndef __MINGW32__
  A2_LOG_INFO(fmt("Executing user command: %s %s %s %s",
                  command.c_str(),
                  gidStr.c_str(),
                  numFilesStr.c_str(),
                  firstFilename.c_str()));
  pid_t cpid = fork();
  if(cpid == -1) {
    A2_LOG_ERROR("fork() failed. Cannot execute user command.");
  } else if(cpid == 0) {
    execlp(command.c_str(),
           command.c_str(),
           gidStr.c_str(),
           numFilesStr.c_str(),
           firstFilename.c_str(),
           reinterpret_cast<char*>(0));
    perror(("Could not execute user command: "+command).c_str());
    _exit(EXIT_FAILURE);
  }
#else
  PROCESS_INFORMATION pi;
  STARTUPINFOW si;

  memset(&si, 0, sizeof (si));
  si.cb = sizeof(STARTUPINFO);
  memset(&pi, 0, sizeof (pi));
  bool batch = util::iendsWith(command, ".bat");
  std::string cmdline;
  std::string cmdexe;
  if(batch) {
    const char* p = getenv("windir");
    if(p) {
      cmdexe = p;
      cmdexe += "\\system32\\cmd.exe";
    } else {
      A2_LOG_INFO("Failed to get windir environment variable."
                  " Executing batch file will fail.");
      // TODO Might be useless.
      cmdexe = "cmd.exe";
    }
    cmdline += "/C \"";
  }
  cmdline += "\"";
  cmdline += command;
  cmdline += "\"";
  cmdline += " ";
  cmdline += gidStr;
  cmdline += " ";
  cmdline += numFilesStr;
  cmdline += " \"";
  cmdline += firstFilename;
  cmdline += "\"";
  if(batch) {
    cmdline += "\"";
  }
  int cmdlineLen = utf8ToWChar(0, 0, cmdline.c_str());
  assert(cmdlineLen > 0);
  array_ptr<wchar_t> wcharCmdline(new wchar_t[cmdlineLen]);
  cmdlineLen = utf8ToWChar(wcharCmdline, cmdlineLen, cmdline.c_str());
  assert(cmdlineLen > 0);
  A2_LOG_INFO(fmt("Executing user command: %s", cmdline.c_str()));
  DWORD rc = CreateProcessW(batch ? utf8ToWChar(cmdexe).c_str() : NULL,
                            wcharCmdline,
                            NULL,
                            NULL,
                            true,
                            0,
                            NULL,
                            0,
                            &si,
                            &pi);

  if(!rc) {
    A2_LOG_ERROR("CreateProcess() failed. Cannot execute user command.");
  }
#endif
}

} // namespace

void executeHookByOptName
(const SharedHandle<RequestGroup>& group, const Option* option,
 const Pref* pref)
{
  executeHookByOptName(group.get(), option, pref);
}

void executeHookByOptName
(const RequestGroup* group, const Option* option, const Pref* pref)
{
  const std::string& cmd = option->get(pref);
  if(!cmd.empty()) {
    const SharedHandle<DownloadContext> dctx = group->getDownloadContext();
    std::string firstFilename;
    size_t numFiles = 0;
    if(!group->inMemoryDownload()) {
      SharedHandle<FileEntry> file = dctx->getFirstRequestedFileEntry();
      if(file) {
        firstFilename = file->getPath();
      }
      numFiles = dctx->countRequestedFileEntry();
    }
    executeHook(cmd, group->getGID(), numFiles, firstFilename);
  }
}

std::string createSafePath
(const std::string& dir, const std::string& filename)
{
  return util::applyDir
    (dir,
     util::isUtf8(filename)?
     util::fixTaintedBasename(filename):
     util::escapePath(util::percentEncode(filename)));
}

std::string encodeNonUtf8(const std::string& s)
{
  return util::isUtf8(s)?s:util::percentEncode(s);
}

std::string makeString(const char* str)
{
  if(str) {
    return str;
  } else {
    return A2STR::NIL;
  }
}

std::string safeStrerror(int errNum)
{
  return makeString(strerror(errNum));
}

bool noProxyDomainMatch
(const std::string& hostname,
 const std::string& domain)
{
  if(!domain.empty() && domain[0] == '.' && !util::isNumericHost(hostname)) {
    return util::endsWith(hostname, domain);
  } else {
    return hostname == domain;
  }
}

bool tlsHostnameMatch(const std::string& pattern, const std::string& hostname)
{
  std::string::const_iterator ptWildcard = std::find(pattern.begin(),
                                                     pattern.end(),
                                                     '*');
  if(ptWildcard == pattern.end()) {
    return strieq(pattern.begin(), pattern.end(),
                  hostname.begin(), hostname.end());
  }
  std::string::const_iterator ptLeftLabelEnd = std::find(pattern.begin(),
                                                         pattern.end(),
                                                         '.');
  bool wildcardEnabled = true;
  // Do case-insensitive match. At least 2 dots are required to enable
  // wildcard match. Also wildcard must be in the left-most label.
  // Don't attempt to match a presented identifier where the wildcard
  // character is embedded within an A-label.
  if(ptLeftLabelEnd == pattern.end() ||
     std::find(ptLeftLabelEnd+1, pattern.end(), '.') == pattern.end() ||
     ptLeftLabelEnd < ptWildcard ||
     istartsWith(pattern, "xn--")) {
    wildcardEnabled = false;
  }
  if(!wildcardEnabled) {
    return strieq(pattern.begin(), pattern.end(),
                  hostname.begin(), hostname.end());
  }
  std::string::const_iterator hnLeftLabelEnd = std::find(hostname.begin(),
                                                         hostname.end(),
                                                         '.');
  if(!strieq(ptLeftLabelEnd, pattern.end(), hnLeftLabelEnd, hostname.end())) {
    return false;
  }
  // Perform wildcard match. Here '*' must match at least one
  // character.
  if(hnLeftLabelEnd - hostname.begin() < ptLeftLabelEnd - pattern.begin()) {
    return false;
  }
  return
    istartsWith(hostname.begin(), hnLeftLabelEnd,
                pattern.begin(), ptWildcard) &&
    iendsWith(hostname.begin(), hnLeftLabelEnd,
              ptWildcard+1, ptLeftLabelEnd);
}

bool strieq(const std::string& a, const char* b)
{
  return strieq(a.begin(), a.end(), b);
}

bool strieq(const std::string& a, const std::string& b)
{
  return strieq(a.begin(), a.end(), b.begin(), b.end());
}

bool startsWith(const std::string& a, const char* b)
{
  return startsWith(a.begin(), a.end(), b);
}

bool startsWith(const std::string& a, const std::string& b)
{
  return startsWith(a.begin(), a.end(), b.begin(), b.end());
}

bool istartsWith(const std::string& a, const char* b)
{
  return istartsWith(a.begin(), a.end(), b);
}

bool endsWith(const std::string& a, const char* b)
{
  return endsWith(a.begin(), a.end(), b, b+strlen(b));
}

bool endsWith(const std::string& a, const std::string& b)
{
  return endsWith(a.begin(), a.end(), b.begin(), b.end());
}

bool iendsWith(const std::string& a, const char* b)
{
  return iendsWith(a.begin(), a.end(), b, b+strlen(b));
}

bool iendsWith(const std::string& a, const std::string& b)
{
  return iendsWith(a.begin(), a.end(), b.begin(), b.end());
}

bool strless(const char* a, const char* b)
{
  return strcmp(a, b) < 0;
}

} // namespace util

} // namespace aria2
