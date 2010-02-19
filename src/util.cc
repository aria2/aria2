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

#include <signal.h>
#include <limits.h>
#include <stdint.h>

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
#ifndef HAVE_SLEEP
# ifdef HAVE_WINSOCK_H
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif // HAVE_WINSOCK_H
#endif // HAVE_SLEEP

#ifdef HAVE_LIBGCRYPT
# include <gcrypt.h>
#elif HAVE_LIBSSL
# include <openssl/rand.h>
# include "SimpleRandomizer.h"
#endif // HAVE_LIBSSL

#include "File.h"
#include "message.h"
#include "Randomizer.h"
#include "a2netcompat.h"
#include "DlAbortEx.h"
#include "BitfieldMan.h"
#include "DefaultDiskWriter.h"
#include "FatalException.h"
#include "FileEntry.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "array_fun.h"
#include "a2functional.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

// For libc6 which doesn't define ULLONG_MAX properly because of broken limits.h
#ifndef ULLONG_MAX
# define ULLONG_MAX 18446744073709551615ULL
#endif // ULLONG_MAX

namespace aria2 {

namespace util {

const std::string DEFAULT_TRIM_CHARSET("\r\n\t ");

std::string trim(const std::string& src, const std::string& trimCharset)
{
  std::string temp(src);
  trimSelf(temp, trimCharset);
  return temp;
}

void trimSelf(std::string& str, const std::string& trimCharset)
{
  std::string::size_type first = str.find_first_not_of(trimCharset);
  if(first == std::string::npos) {
    str.clear();
  } else {
    std::string::size_type last = str.find_last_not_of(trimCharset)+1;
    str.erase(last);
    str.erase(0, first);
  }
}

void split(std::pair<std::string, std::string>& hp, const std::string& src, char delim)
{
  hp.first = A2STR::NIL;
  hp.second = A2STR::NIL;
  std::string::size_type p = src.find(delim);
  if(p == std::string::npos) {
    hp.first = trim(src);
    hp.second = A2STR::NIL;
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
  }
}

std::pair<std::string, std::string> split(const std::string& src, const std::string& delims)
{
  std::pair<std::string, std::string> hp;
  hp.first = A2STR::NIL;
  hp.second = A2STR::NIL;
  std::string::size_type p = src.find_first_of(delims);
  if(p == std::string::npos) {
    hp.first = trim(src);
    hp.second = A2STR::NIL;
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
  }
  return hp;
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

bool startsWith(const std::string& target, const std::string& part) {
  if(target.size() < part.size()) {
    return false;
  }
  if(part.empty()) {
    return true;
  }
  if(target.find(part) == 0) {
    return true;
  } else {
    return false;
  }
}

bool endsWith(const std::string& target, const std::string& part) {
  if(target.size() < part.size()) {
    return false;
  }
  if(part.empty()) {
    return true;
  }
  if(target.rfind(part) == target.size()-part.size()) {
    return true;
  } else {
    return false;
  }
}

std::string replace(const std::string& target, const std::string& oldstr, const std::string& newstr) {
  if(target.empty() || oldstr.empty()) {
    return target;
  }
  std::string result;
  std::string::size_type p = 0;
  std::string::size_type np = target.find(oldstr);
  while(np != std::string::npos) {
    result += target.substr(p, np-p);
    result += newstr;
    p = np+oldstr.size();
    np = target.find(oldstr, p);
  }
  result += target.substr(p);

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

bool inRFC3986ReservedChars(const char c)
{
  static const char reserved[] = {
    ':' , '/' , '?' , '#' , '[' , ']' , '@',
    '!' , '$' , '&' , '\'' , '(' , ')',
    '*' , '+' , ',' , ';' , '=' };
  return std::find(&reserved[0], &reserved[arrayLength(reserved)], c) !=
    &reserved[arrayLength(reserved)];
}

bool inRFC3986UnreservedChars(const char c)
{
  static const char unreserved[] = { '-', '.', '_', '~' };
  return isAlpha(c) || isDigit(c) ||
    std::find(&unreserved[0], &unreserved[arrayLength(unreserved)], c) !=
    &unreserved[arrayLength(unreserved)];
}

bool inRFC2978MIMECharset(const char c)
{
  static const char chars[] = {
    '!', '#', '$', '%', '&',
    '\'', '+', '-', '^', '_',
    '`', '{', '}', '~'
  };
  return isAlpha(c) || isDigit(c) ||
    std::find(&chars[0], &chars[arrayLength(chars)], c) !=
    &chars[arrayLength(chars)];
}

bool inRFC2616HttpToken(const char c)
{
  static const char chars[] = {
    '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.',
    '^', '_', '`', '|', '~'
  };
  return isAlpha(c) || isDigit(c) ||
    std::find(&chars[0], &chars[arrayLength(chars)], c) !=
    &chars[arrayLength(chars)];
}

std::string urlencode(const unsigned char* target, size_t len) {
  std::string dest;
  for(size_t i = 0; i < len; ++i) {
    if(!inRFC3986UnreservedChars(target[i])) {
      dest.append(StringFormat("%%%02X", target[i]).str());
    } else {
      dest += target[i];
    }
  }
  return dest;
}

std::string urlencode(const std::string& target)
{
  return urlencode(reinterpret_cast<const unsigned char*>(target.c_str()),
                   target.size());
}

std::string torrentUrlencode(const unsigned char* target, size_t len) {
  std::string dest;
  for(size_t i = 0; i < len; ++i) {
    if(isAlpha(target[i]) || isDigit(target[i])) {
      dest += target[i];
    } else {
      dest.append(StringFormat("%%%02X", target[i]).str());
    }
  }
  return dest;
}

std::string torrentUrlencode(const std::string& target)
{
  return torrentUrlencode
    (reinterpret_cast<const unsigned char*>(target.c_str()), target.size());
}

std::string urldecode(const std::string& target) {
  std::string result;
  for(std::string::const_iterator itr = target.begin();
      itr != target.end(); ++itr) {
    if(*itr == '%') {
      if(itr+1 != target.end() && itr+2 != target.end() &&
         isHexDigit(*(itr+1)) && isHexDigit(*(itr+2))) {
        result += parseInt(std::string(itr+1, itr+3), 16);
        itr += 2;
      } else {
        result += *itr;
      }
    } else {
      result += *itr;
    }
  }
  return result;
}

std::string toHex(const unsigned char* src, size_t len) {
  char* temp = new char[len*2+1];
  for(size_t i = 0; i < len; ++i) {
    sprintf(temp+i*2, "%02x", src[i]);
  }
  temp[len*2] = '\0';
  std::string hex = temp;
  delete [] temp;
  return hex;
}

std::string toHex(const char* src, size_t len)
{
  return toHex(reinterpret_cast<const unsigned char*>(src), len);
}

std::string toHex(const std::string& src)
{
  return toHex(reinterpret_cast<const unsigned char*>(src.c_str()), src.size());
}

static unsigned int hexCharToUInt(unsigned char ch)
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

std::string fromHex(const std::string& src)
{
  std::string dest;
  if(src.size()%2) {
    return dest;
  }
  for(size_t i = 0; i < src.size(); i += 2) {
    unsigned char high = hexCharToUInt(src[i]);
    unsigned char low = hexCharToUInt(src[i+1]);
    if(high == 255 || low == 255) {
      dest.clear();
      return dest;
    }
    dest += (high*16+low);
  }
  return dest;
}

FILE* openFile(const std::string& filename, const std::string& mode) {
  FILE* file = fopen(filename.c_str(), mode.c_str());
  return file;
}

bool isPowerOf(int num, int base) {
  if(base <= 0) { return false; }
  if(base == 1) { return true; }

  while(num%base == 0) {
    num /= base;
    if(num == 1) {
      return true;
    }
  }
  return false;
}

std::string secfmt(time_t sec) {
  std::string str;
  if(sec >= 3600) {
    str = itos(sec/3600);
    str += "h";
    sec %= 3600;
  }
  if(sec >= 60) {
    int min = sec/60;
    if(min < 10) {
      str += "0";
    }
    str += itos(min);
    str += "m";
    sec %= 60;
  }
  if(sec < 10) {
    str += "0";
  }
  str += itos(sec);
  str += "s";
  return str;
}

int getNum(const char* buf, int offset, size_t length) {
  char* temp = new char[length+1];
  memcpy(temp, buf+offset, length);
  temp[length] = '\0';
  int x = strtol(temp, 0, 10);
  delete [] temp;
  return x;
}

int32_t parseInt(const std::string& s, int32_t base)
{
  std::string trimed = trim(s);
  if(trimed.empty()) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   "empty string").str());
  }
  char* stop;
  errno = 0;
  long int v = strtol(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  } else if((((v == LONG_MIN) || (v == LONG_MAX)) && (errno == ERANGE)) ||
            (v > INT32_MAX) ||
            (v < INT32_MIN)) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  }
  return v;
}

uint32_t parseUInt(const std::string& s, int base)
{
  std::string trimed = trim(s);
  if(trimed.empty()) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   "empty string").str());
  }
  // We don't allow negative number.
  if(trimed[0] == '-') {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  }
  char* stop;
  errno = 0;
  unsigned long int v = strtoul(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  } else if(((v == ULONG_MAX) && (errno == ERANGE)) || (v > UINT32_MAX)) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  }
  return v;
}

bool parseUIntNoThrow(uint32_t& result, const std::string& s, int base)
{
  std::string trimed = trim(s);
  if(trimed.empty()) {
    return false;
  }
  // We don't allow negative number.
  if(trimed[0] == '-') {
    return false;
  }
  char* stop;
  errno = 0;
  unsigned long int v = strtoul(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    return false;
  } else if(((v == ULONG_MAX) && (errno == ERANGE)) || (v > UINT32_MAX)) {
    return false;
  }
  result = v;
  return true;
}

int64_t parseLLInt(const std::string& s, int32_t base)
{
  std::string trimed = trim(s);
  if(trimed.empty()) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   "empty string").str());
  }
  char* stop;
  errno = 0;
  int64_t v = strtoll(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  } else if(((v == INT64_MIN) || (v == INT64_MAX)) && (errno == ERANGE)) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  }
  return v;
}

uint64_t parseULLInt(const std::string& s, int base)
{
  std::string trimed = trim(s);
  if(trimed.empty()) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   "empty string").str());
  }
  // We don't allow negative number.
  if(trimed[0] == '-') {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  }
  char* stop;
  errno = 0;
  uint64_t v = strtoull(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  } else if((v == ULLONG_MAX) && (errno == ERANGE)) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   trimed.c_str()).str());
  }
  return v;
}

IntSequence parseIntRange(const std::string& src)
{
  IntSequence::Values values;
  std::string temp = src;
  while(temp.size()) {
    std::pair<std::string, std::string> p = split(temp, ",");
    temp = p.second;
    if(p.first.empty()) {
      continue;
    }
    if(p.first.find("-") == std::string::npos) {
      int32_t v = parseInt(p.first.c_str());
      values.push_back(IntSequence::Value(v, v+1));
    } else {
      std::pair<std::string, std::string> vp = split(p.first.c_str(), "-");
      if(vp.first.empty() || vp.second.empty()) {
        throw DL_ABORT_EX
          (StringFormat(MSG_INCOMPLETE_RANGE, p.first.c_str()).str());
      }
      int32_t v1 = parseInt(vp.first.c_str());
      int32_t v2 = parseInt(vp.second.c_str());
      values.push_back(IntSequence::Value(v1, v2+1));
    } 
  }
  return values;
}

static void computeHeadPieces
(std::vector<size_t>& indexes,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 uint64_t head)
{
  if(head == 0) {
    return;
  }
  for(std::vector<SharedHandle<FileEntry> >::const_iterator fi =
        fileEntries.begin(); fi != fileEntries.end(); ++fi) {
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

static void computeTailPieces
(std::vector<size_t>& indexes,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 uint64_t tail)
{
  if(tail == 0) {
    return;
  }
  for(std::vector<SharedHandle<FileEntry> >::const_iterator fi =
        fileEntries.begin(); fi != fileEntries.end(); ++fi) {
    if((*fi)->getLength() == 0) {
      continue;
    }
    uint64_t endOffset = (*fi)->getLastOffset();
    size_t fromIndex =
      (endOffset-1-(std::min(tail, (*fi)->getLength())-1))/pieceLength;
    for(size_t index = fromIndex; index <= (endOffset-1)/pieceLength;
        ++index) {
      indexes.push_back(index);
    }
  }
}

void parsePrioritizePieceRange
(std::vector<size_t>& result, const std::string& src,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 uint64_t defaultSize)
{
  std::vector<size_t> indexes;
  std::vector<std::string> parts;
  split(src, std::back_inserter(parts), ",", true);
  for(std::vector<std::string>::const_iterator i = parts.begin();
      i != parts.end(); ++i) {
    if((*i) == "head") {
      computeHeadPieces(indexes, fileEntries, pieceLength, defaultSize);
    } else if(util::startsWith(*i, "head=")) {
      std::string sizestr = std::string((*i).begin()+(*i).find("=")+1,
                                        (*i).end());
      computeHeadPieces(indexes, fileEntries, pieceLength,
                        std::max((int64_t)0, getRealSize(sizestr)));
    } else if((*i) == "tail") {
      computeTailPieces(indexes, fileEntries, pieceLength, defaultSize);
    } else if(util::startsWith(*i, "tail=")) {
      std::string sizestr = std::string((*i).begin()+(*i).find("=")+1,
                                        (*i).end());
      computeTailPieces(indexes, fileEntries, pieceLength,
                        std::max((int64_t)0, getRealSize(sizestr)));
    } else {
      throw DL_ABORT_EX
        (StringFormat("Unrecognized token %s", (*i).c_str()).str());
    }
  }
  std::sort(indexes.begin(), indexes.end());
  indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());
  result.insert(result.end(), indexes.begin(), indexes.end());
}

static std::string trimBasename(const std::string& src)
{
  static const std::string TRIMMED("\r\n\t '\"");
  std::string fn = File(trim(src, TRIMMED)).getBasename();
  std::string::iterator enditer = std::remove(fn.begin(), fn.end(), '\\');
  fn = std::string(fn.begin(), enditer);
  if(fn == ".." || fn == A2STR::DOT_C) {
    fn = A2STR::NIL;
  }
  return fn;
}

// Converts ISO/IEC 8859-1 string to UTF-8 string.  If there is a
// character not in ISO/IEC 8859-1, returns empty string.
std::string iso8859ToUtf8(const std::string& src)
{
  std::string dest;
  for(std::string::const_iterator itr = src.begin(); itr != src.end(); ++itr) {
    unsigned char c = *itr;
    if(0xa0 <= c && c <= 0xff) {
      if(c <= 0xbf) {
        dest += 0xc2;
      } else {
        dest += 0xc3;
      }
      dest += c&(~0x40);
    } else if(0x80 <= c && c <= 0x9f) {
      return A2STR::NIL;
    } else {
      dest += c;
    }
  }
  return dest;
}

std::string getContentDispositionFilename(const std::string& header)
{
  std::string filename;
  std::vector<std::string> params;
  split(header, std::back_inserter(params), A2STR::SEMICOLON_C, true);
  for(std::vector<std::string>::iterator i = params.begin();
      i != params.end(); ++i) {
    std::string& param = *i;
    static const std::string keyName = "filename";
    if(!startsWith(toLower(param), keyName) || param.size() == keyName.size()) {
      continue;
    }
    std::string::iterator markeritr = param.begin()+keyName.size();
    if(*markeritr == '*') {
      // See RFC2231 Section4 and draft-reschke-rfc2231-in-http.
      // Please note that this function doesn't do charset conversion
      // except that if iso-8859-1 is specified, it is converted to
      // utf-8.
      ++markeritr;
      for(; markeritr != param.end() && *markeritr == ' '; ++markeritr);
      if(markeritr == param.end() || *markeritr != '=') {
        continue;
      }
      std::pair<std::string, std::string> paramPair;
      split(paramPair, param, '=');
      std::string value = paramPair.second;
      std::vector<std::string> extValues;
      split(value, std::back_inserter(extValues), "'", false, true);
      if(extValues.size() != 3) {
        continue;
      }
      bool bad = false;
      const std::string& charset = extValues[0];
      for(std::string::const_iterator j = charset.begin(); j != charset.end();
          ++j) {
        // Since we first split parameter by ', we can safely assume
        // that ' is not included in charset.
        if(!inRFC2978MIMECharset(*j)) {
          bad = true;
          break;
        }
      }
      if(bad) {
        continue;
      }
      bad = false;
      value = extValues[2];
      for(std::string::const_iterator j = value.begin(); j != value.end(); ++j){
        if(*j == '%') {
          if(j+1 != value.end() && isHexDigit(*(j+1)) &&
             j+2 != value.end() && isHexDigit(*(j+2))) {
            j += 2;
          } else {
            bad = true;
            break;
          }
        } else {
          if(*j == '*' || *j == '\'' || !inRFC2616HttpToken(*j)) {
            bad = true;
            break;
          }
        }
      }
      if(bad) {
        continue;
      }
      value = trimBasename(urldecode(value));
      if(toLower(extValues[0]) == "iso-8859-1") {
        value = iso8859ToUtf8(value);
      }
      filename = value;
      break;
    } else {
      for(; markeritr != param.end() && *markeritr == ' '; ++markeritr);
      if(markeritr == param.end() || *markeritr != '=') {
        continue;
      }
      std::pair<std::string, std::string> paramPair;
      split(paramPair, param, '=');
      std::string value = paramPair.second;
      if(value.empty()) {
        continue;
      }
      std::string::iterator filenameLast;
      if(*value.begin() == '\'' || *value.begin() == '"') {
        char qc = *value.begin();
        for(filenameLast = value.begin()+1;
            filenameLast != value.end() && *filenameLast != qc;
            ++filenameLast);
      } else {
        filenameLast = value.end();
      }
      value = trimBasename(urldecode(std::string(value.begin(), filenameLast)));
      filename = value;
      // continue because there is a chance we can find filename*=...
    }
  }
  return filename;
}

std::string randomAlpha(size_t length, const RandomizerHandle& randomizer) {
  static const char *random_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string str;
  for(size_t i = 0; i < length; ++i) {
    size_t index = randomizer->getRandomNumber(strlen(random_chars));
    str += random_chars[index];
  }
  return str;
}

std::string toUpper(const std::string& src) {
  std::string temp = src;
  std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
  return temp;
}

std::string toLower(const std::string& src) {
  std::string temp = src;
  std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
  return temp;
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

void setGlobalSignalHandler(int sig, void (*handler)(int), int flags) {
#ifdef HAVE_SIGACTION
  struct sigaction sigact;
  sigact.sa_handler = handler;
  sigact.sa_flags = flags;
  sigemptyset(&sigact.sa_mask);
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
    return A2STR::NIL;
  }
}

int64_t getRealSize(const std::string& sizeWithUnit)
{
  std::string::size_type p = sizeWithUnit.find_first_of("KM");
  std::string size;
  int32_t mult = 1;
  if(p == std::string::npos) {
    size = sizeWithUnit;
  } else {
    if(sizeWithUnit[p] == 'K') {
      mult = 1024;
    } else if(sizeWithUnit[p] == 'M') {
      mult = 1024*1024;
    }
    size = sizeWithUnit.substr(0, p);
  }
  int64_t v = parseLLInt(size);

  if(v < 0) {
    throw DL_ABORT_EX
      (StringFormat("Negative value detected: %s", sizeWithUnit.c_str()).str());
  } else if(INT64_MAX/mult < v) {
    throw DL_ABORT_EX(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
                                   "overflow/underflow").str());
  }
  return v*mult;
}

std::string abbrevSize(int64_t size)
{
  if(size < 1024) {
    return itos(size, true);
  }
  char units[] = { 'K', 'M' };
  size_t numUnit = sizeof(units)/sizeof(char);
  size_t i = 0;
  int r = size&0x3ff;
  size >>= 10;
  for(; i < numUnit-1 && size >= 1024; ++i) {
    r = size&0x3ff;
    size >>= 10;
  }
  std::string result = itos(size, true);
  result += A2STR::DOT_C;
  result += itos(r*10/1024);
  result += units[i];
  result += "i";
  return result;
}

void sleep(long seconds) {
#ifdef HAVE_SLEEP
  ::sleep(seconds);
#elif defined(HAVE_USLEEP)
  ::usleep(seconds * 1000000);
#elif defined(HAVE_WINSOCK2_H)
  ::Sleep(seconds * 1000);
#else
#error no sleep function is available (nanosleep?)
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

bool isNumber(const std::string& what)
{
  if(what.empty()) {
    return false;
  }
  for(uint32_t i = 0; i < what.size(); ++i) {
    if(!isdigit(what[i])) {
      return false;
    }
  }
  return true;
}

bool isLowercase(const std::string& what)
{
  if(what.empty()) {
    return false;
  }
  for(uint32_t i = 0; i < what.size(); ++i) {
    if(!('a' <= what[i] && what[i] <= 'z')) {
      return false;
    }
  }
  return true;
}

bool isUppercase(const std::string& what)
{
  if(what.empty()) {
    return false;
  }
  for(uint32_t i = 0; i < what.size(); ++i) {
    if(!('A' <= what[i] && what[i] <= 'Z')) {
      return false;
    }
  }
  return true;
}

unsigned int alphaToNum(const std::string& alphabets)
{
  if(alphabets.empty()) {
    return 0;
  }
  char base;
  if(islower(alphabets[0])) {
    base = 'a';
  } else {
    base = 'A';
  }
  uint64_t num = 0;
  for(size_t i = 0; i < alphabets.size(); ++i) {
    unsigned int v = alphabets[i]-base;
    num = num*26+v;
    if(num > UINT32_MAX) {
      return 0;
    }
  }
  return num;
}

void mkdirs(const std::string& dirpath)
{
  File dir(dirpath);
  if(dir.isDir()) {
    // do nothing
  } else if(dir.exists()) {
    throw DL_ABORT_EX
      (StringFormat(EX_MAKE_DIR, dir.getPath().c_str(),
                    "File already exists.").str());
  } else if(!dir.mkdirs()) {
    throw DL_ABORT_EX
      (StringFormat(EX_MAKE_DIR, dir.getPath().c_str(),
                    strerror(errno)).str());
  }
}

void convertBitfield(BitfieldMan* dest, const BitfieldMan* src)
{
  size_t numBlock = dest->countBlock();
  for(size_t index = 0; index < numBlock; ++index) {
    if(src->isBitSetOffsetRange((uint64_t)index*dest->getBlockLength(),
                                dest->getBlockLength())) {
      dest->setBit(index);
    }
  }
}

std::string toString(const BinaryStreamHandle& binaryStream)
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
      (StringFormat("Error in posix_memalign: %s", strerror(res)).str());
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
    throw DL_ABORT_EX(StringFormat("Failed to get hostname and port. cause: %s",
                                   gai_strerror(s)).str());
  }
  return std::pair<std::string, uint16_t>(host, atoi(service)); // TODO
}

std::string htmlEscape(const std::string& src)
{
  std::string dest;
  for(std::string::const_iterator i = src.begin(); i != src.end(); ++i) {
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

std::map<size_t, std::string>::value_type
parseIndexPath(const std::string& line)
{
  std::pair<std::string, std::string> p = split(line, "=");
  size_t index = parseUInt(p.first);
  if(p.second.empty()) {
    throw DL_ABORT_EX(StringFormat("Path with index=%u is empty.",
                                   static_cast<unsigned int>(index)).str());
  }
  return std::map<size_t, std::string>::value_type(index, p.second);
}

std::map<size_t, std::string> createIndexPathMap(std::istream& i)
{
  std::map<size_t, std::string> indexPathMap;
  std::string line;
  while(getline(i, line)) {
    indexPathMap.insert(indexPathMap.begin(), parseIndexPath(line));
  }
  return indexPathMap;
}

void generateRandomData(unsigned char* data, size_t length)
{
#ifdef HAVE_LIBGCRYPT
  gcry_randomize(data, length, GCRY_STRONG_RANDOM);
#elif HAVE_LIBSSL
  if(RAND_bytes(data, length) != 1) {
    for(size_t i = 0; i < length; ++i) {
      data[i] = SimpleRandomizer::getInstance()->getRandomNumber(UINT8_MAX+1);
    }
  }
#else
  std::ifstream i("/dev/urandom", std::ios::binary);
  i.read(reinterpret_cast<char*>(data), length);
#endif // HAVE_LIBSSL
}

bool saveAs
(const std::string& filename, const std::string& data, bool overwrite)
{
  if(!overwrite && File(filename).exists()) {
    return false;
  }
  std::string tempFilename = strconcat(filename, "__temp");
  std::ofstream out(tempFilename.c_str(), std::ios::binary);
  if(!out) {
    return false;
  }
  out << data;
  out.flush();
  if(!out) {
    return false;
  }
  return File(tempFilename).renameTo(filename);
}

std::string applyDir(const std::string& dir, const std::string& relPath)
{
  if(dir.empty()) {
    return strconcat(A2STR::DOT_C, A2STR::SLASH_C, relPath);
  } else if(dir == A2STR::SLASH_C) {
    return strconcat(A2STR::SLASH_C, relPath);
  } else {
    return strconcat(dir, A2STR::SLASH_C, relPath);
  }
}

std::string fixTaintedBasename(const std::string& src)
{
  return replace(replace(src, A2STR::SLASH_C, A2STR::UNDERSCORE_C),
                 A2STR::BACK_SLASH_C, A2STR::UNDERSCORE_C);
}

void generateRandomKey(unsigned char* key)
{
#ifdef ENABLE_MESSAGE_DIGEST
  unsigned char bytes[40];
  generateRandomData(bytes, sizeof(bytes));
  MessageDigestHelper::digest
    (key, 20, MessageDigestContext::SHA1, bytes, sizeof(bytes));
#else // !ENABLE_MESSAGE_DIGEST
  generateRandomData(key, 20);
#endif // !ENABLE_MESSAGE_DIGEST
}

} // namespace util

} // namespace aria2
