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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Util.h"
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
#include <signal.h>
#include <limits.h>
#include <stdint.h>
#include <cerrno>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <ostream>
#include <algorithm>
#ifndef HAVE_SLEEP
# ifdef HAVE_WINSOCK_H
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif // HAVE_WINSOCK_H
#endif // HAVE_SLEEP

// For libc6 which doesn't define ULLONG_MAX properly because of broken limits.h
#ifndef ULLONG_MAX
# define ULLONG_MAX 18446744073709551615ULL
#endif // ULLONG_MAX

namespace aria2 {

const std::string Util::DEFAULT_TRIM_CHARSET("\r\n\t ");

std::string Util::trim(const std::string& src, const std::string& trimCharset)
{
  std::string temp(src);
  trimSelf(temp, trimCharset);
  return temp;
}

void Util::trimSelf(std::string& str, const std::string& trimCharset)
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

void Util::split(std::pair<std::string, std::string>& hp, const std::string& src, char delim)
{
  hp.first = A2STR::NIL;
  hp.second = A2STR::NIL;
  std::string::size_type p = src.find(delim);
  if(p == std::string::npos) {
    hp.first = src;
    hp.second = A2STR::NIL;
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
  }
}

std::pair<std::string, std::string> Util::split(const std::string& src, const std::string& delims)
{
  std::pair<std::string, std::string> hp;
  hp.first = A2STR::NIL;
  hp.second = A2STR::NIL;
  std::string::size_type p = src.find_first_of(delims);
  if(p == std::string::npos) {
    hp.first = src;
    hp.second = A2STR::NIL;
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
  }
  return hp;
}

int64_t Util::difftv(struct timeval tv1, struct timeval tv2) {
  if((tv1.tv_sec < tv2.tv_sec) ||
     ((tv1.tv_sec == tv2.tv_sec) && (tv1.tv_usec < tv2.tv_usec))) {
    return 0;
  }
  return ((int64_t)(tv1.tv_sec-tv2.tv_sec)*1000000+
	  tv1.tv_usec-tv2.tv_usec);
}

int32_t Util::difftvsec(struct timeval tv1, struct timeval tv2) {
  if(tv1.tv_sec < tv2.tv_sec) {
    return 0;
  }
  return tv1.tv_sec-tv2.tv_sec;
}

void Util::slice(std::deque<std::string>& result, const std::string& src, char delim, bool doTrim) {
  std::string::size_type p = 0;
  while(1) {
    std::string::size_type np = src.find(delim, p);
    if(np == std::string::npos) {
      std::string term = src.substr(p);
      if(doTrim) {
	term = trim(term);
      }
      if(term.size()) {
	result.push_back(term);
      }
      break;
    }
    std::string term = src.substr(p, np-p);
    if(doTrim) {
      term = trim(term);
    }
    p = np+1;
    if(term.size()) {
      result.push_back(term);
    }
  } 
}

bool Util::startsWith(const std::string& target, const std::string& part) {
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

bool Util::endsWith(const std::string& target, const std::string& part) {
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

std::string Util::replace(const std::string& target, const std::string& oldstr, const std::string& newstr) {
  if(target.empty() || oldstr.empty()) {
    return target;
  }
  std::string result;
  std::string::size_type p = 0;
  std::string::size_type np = target.find(oldstr);
  while(np != std::string::npos) {
    result += target.substr(p, np-p)+newstr;
    p = np+oldstr.size();
    np = target.find(oldstr, p);
  }
  result += target.substr(p);

  return result;
}

bool Util::shouldUrlencode(const char c)
{
  return !(// ALPHA
	   ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
	   // DIGIT
	   ('0' <= c && c <= '9') ||
	   // safe
	   '$' == c || '-' == c || '_' == c || '.' == c ||
	   // extra
	   '!' == c || '*' == c || '\'' == c ||'(' == c ||
	   ')' == c || ',' == c ||
	   // reserved
	   ';' == c || '/' == c || '?' == c  || ':' == c ||
	   '@' == c || '&' == c || '=' == c || '+' == c);	   
}

std::string Util::urlencode(const unsigned char* target, size_t len) {
  std::string dest;
  for(size_t i = 0; i < len; i++) {
    if(shouldUrlencode(target[i])) {
      dest.append(StringFormat("%%%02x", target[i]).str());
    } else {
      dest += target[i];
    }
  }
  return dest;
}

std::string Util::torrentUrlencode(const unsigned char* target, size_t len) {
  std::string dest;
  for(size_t i = 0; i < len; i++) {
    if(('0' <= target[i] && target[i] <= '9') ||
       ('A' <= target[i] && target[i] <= 'Z') ||
       ('a' <= target[i] && target[i] <= 'z')) {
      dest += target[i];
    } else {
      dest.append(StringFormat("%%%02x", target[i]).str());
    }
  }
  return dest;
}

std::string Util::urldecode(const std::string& target) {
  std::string result;
  for(std::string::const_iterator itr = target.begin();
      itr != target.end(); itr++) {
    if(*itr == '%') {
      if(itr+1 != target.end() && itr+2 != target.end() &&
	 isxdigit(*(itr+1)) && isxdigit(*(itr+2))) {
	result += Util::parseInt(std::string(itr+1, itr+3), 16);
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

std::string Util::toHex(const unsigned char* src, size_t len) {
  char* temp = new char[len*2+1];
  for(size_t i = 0; i < len; i++) {
    sprintf(temp+i*2, "%02x", src[i]);
  }
  temp[len*2] = '\0';
  std::string hex = temp;
  delete [] temp;
  return hex;
}

FILE* Util::openFile(const std::string& filename, const std::string& mode) {
  FILE* file = fopen(filename.c_str(), mode.c_str());
  return file;
}

void Util::fileCopy(const std::string& dest, const std::string& src) {
  File file(src);
  rangedFileCopy(dest, src, 0, file.size());
}

void Util::rangedFileCopy(const std::string& dest, const std::string& src, off_t srcOffset, uint64_t length)
{
  size_t bufSize = 4096;
  unsigned char buf[bufSize];
  DefaultDiskWriter srcdw;
  DefaultDiskWriter destdw;

  srcdw.openExistingFile(src);
  destdw.initAndOpenFile(dest);

  lldiv_t res = lldiv(length, bufSize);
  unsigned int x = res.quot;
  unsigned int r = res.rem;

  off_t initialOffset = srcOffset;
  for(unsigned int i = 0; i < x; ++i) {
    size_t readLength = 0;
    while(readLength < bufSize) {
      ssize_t ret = srcdw.readData(buf, bufSize-readLength, srcOffset);
      destdw.writeData(buf, ret, srcOffset-initialOffset);
      srcOffset += ret;
      readLength += ret;
    }
  }
  if(r > 0) {
    size_t readLength = 0;
    while(readLength < r) { 
      ssize_t ret = srcdw.readData(buf, r-readLength, srcOffset);
      destdw.writeData(buf, ret, srcOffset-initialOffset);
      srcOffset += ret;
      readLength += ret;
    }
  }     
}

bool Util::isPowerOf(int num, int base) {
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

std::string Util::secfmt(time_t sec) {
  std::string str;
  if(sec >= 3600) {
    str = itos(sec/3600)+"h";
    sec %= 3600;
  }
  if(sec >= 60) {
    int min = sec/60;
    if(min < 10) {
      str += "0";
    }
    str += itos(min)+"m";
    sec %= 60;
  }
  if(sec < 10) {
    str += "0";
  }
  str += itos(sec)+"s";
  return str;
}

size_t Util::expandBuffer(char** pbuf, size_t curLength, size_t newLength) {
  char* newbuf = new char[newLength];
  memcpy(newbuf, *pbuf, curLength);
  delete [] *pbuf;
  *pbuf = newbuf;
  return newLength;
}

int getNum(const char* buf, int offset, size_t length) {
  char* temp = new char[length+1];
  memcpy(temp, buf+offset, length);
  temp[length] = '\0';
  int x = strtol(temp, 0, 10);
  delete [] temp;
  return x;
}

void unfoldSubRange(const std::string& src, std::deque<int>& range) {
  if(src.empty()) {
    return;
  }
  std::string::size_type p = src.find_first_of(",-");
  if(p == 0) {
    return;
  } else if(p == std::string::npos) {
    range.push_back(atoi(src.c_str()));
  } else {
    if(src.at(p) == ',') {
      int num = getNum(src.c_str(), 0, p);
      range.push_back(num);
      unfoldSubRange(src.substr(p+1), range);
    } else if(src.at(p) == '-') {
      std::string::size_type rightNumBegin = p+1;
      std::string::size_type nextDelim = src.find_first_of(",", rightNumBegin);
      if(nextDelim == std::string::npos) {
	nextDelim = src.size();
      }
      int left = getNum(src.c_str(), 0, p);
      int right = getNum(src.c_str(), rightNumBegin, nextDelim-rightNumBegin);
      for(int i = left; i <= right; i++) {
	range.push_back(i);
      }
      if(src.size() > nextDelim) {
	unfoldSubRange(src.substr(nextDelim+1), range);
      }
    }
  }
}

void Util::unfoldRange(const std::string& src, std::deque<int>& range) {
  unfoldSubRange(src, range);
  std::sort(range.begin(), range.end());
  range.erase(std::unique(range.begin(), range.end()), range.end());
}

int32_t Util::parseInt(const std::string& s, int32_t base)
{
  std::string trimed = Util::trim(s);
  if(trimed.empty()) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 "empty string").str());
  }
  char* stop;
  errno = 0;
  long int v = strtol(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  } else if((((v == LONG_MIN) || (v == LONG_MAX)) && (errno == ERANGE)) ||
	    (v > INT32_MAX) ||
	    (v < INT32_MIN)) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  }
  return v;
}

uint32_t Util::parseUInt(const std::string& s, int base)
{
  std::string trimed = Util::trim(s);
  if(trimed.empty()) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 "empty string").str());
  }
  // We don't allow negative number.
  if(trimed[0] == '-') {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  }
  char* stop;
  errno = 0;
  unsigned long int v = strtoul(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  } else if(((v == ULONG_MAX) && (errno == ERANGE)) || (v > UINT32_MAX)) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  }
  return v;
}

int64_t Util::parseLLInt(const std::string& s, int32_t base)
{
  std::string trimed = Util::trim(s);
  if(trimed.empty()) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 "empty string").str());
  }
  char* stop;
  errno = 0;
  int64_t v = strtoll(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  } else if(((v == INT64_MIN) || (v == INT64_MAX)) && (errno == ERANGE)) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  }
  return v;
}

uint64_t Util::parseULLInt(const std::string& s, int base)
{
  std::string trimed = Util::trim(s);
  if(trimed.empty()) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 "empty string").str());
  }
  // We don't allow negative number.
  if(trimed[0] == '-') {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  }
  char* stop;
  errno = 0;
  uint64_t v = strtoull(trimed.c_str(), &stop, base);
  if(*stop != '\0') {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  } else if((v == ULLONG_MAX) && (errno == ERANGE)) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 trimed.c_str()).str());
  }
  return v;
}

IntSequence Util::parseIntRange(const std::string& src)
{
  IntSequence::Values values;
  std::string temp = src;
  while(temp.size()) {
    std::pair<std::string, std::string> p = Util::split(temp, ",");
    temp = p.second;
    if(p.first.empty()) {
      continue;
    }
    if(p.first.find("-") == std::string::npos) {
      int32_t v = Util::parseInt(p.first.c_str());
      values.push_back(IntSequence::Value(v, v+1));
    } else {
      std::pair<std::string, std::string> vp = Util::split(p.first.c_str(), "-");
      if(vp.first.empty() || vp.second.empty()) {
	throw DlAbortEx
	  (StringFormat(MSG_INCOMPLETE_RANGE, p.first.c_str()).str());
      }
      int32_t v1 = Util::parseInt(vp.first.c_str());
      int32_t v2 = Util::parseInt(vp.second.c_str());
      values.push_back(IntSequence::Value(v1, v2+1));
    } 
  }
  return values;
}

std::string Util::getContentDispositionFilename(const std::string& header) {
  static const std::string keyName = "filename=";
  std::string::size_type attributesp = header.find(keyName);
  if(attributesp == std::string::npos) {
    return A2STR::NIL;
  }
  std::string::size_type filenamesp = attributesp+keyName.size();
  std::string::size_type filenameep;
  if(filenamesp == header.size()) {
    return A2STR::NIL;
  }
  
  if(header[filenamesp] == '\'' || header[filenamesp] == '"') {
    char quoteChar = header[filenamesp];
    filenameep = header.find(quoteChar, filenamesp+1);
  } else {
    filenameep = header.find(';', filenamesp);
  }
  if(filenameep == std::string::npos) {
    filenameep = header.size();
  }
  static const std::string TRIMMED("\r\n '\"");
  return trim(header.substr(filenamesp, filenameep-filenamesp), TRIMMED);
}

static int nbits[] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8, 
};

unsigned int Util::countBit(uint32_t n) {
  return
    nbits[n&0xffu]+
    nbits[(n >> 8)&0xffu]+
    nbits[(n >> 16)&0xffu]+
    nbits[(n >> 24)&0xffu];
}

std::string Util::randomAlpha(size_t length, const RandomizerHandle& randomizer) {
  static const char *random_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string str;
  for(size_t i = 0; i < length; i++) {
    size_t index = randomizer->getRandomNumber(strlen(random_chars));
    str += random_chars[index];
  }
  return str;
}

std::string Util::toUpper(const std::string& src) {
  std::string temp = src;
  std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
  return temp;
}

std::string Util::toLower(const std::string& src) {
  std::string temp = src;
  std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
  return temp;
}

bool Util::isNumbersAndDotsNotation(const std::string& name) {
  struct sockaddr_in sockaddr;
  if(inet_aton(name.c_str(), &sockaddr.sin_addr)) {
    return true;
  } else {
    return false;
  }
}

void Util::setGlobalSignalHandler(int sig, void (*handler)(int), int flags) {
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

void Util::indexRange(size_t& startIndex, size_t& endIndex,
		      off_t offset, size_t srcLength, size_t destLength)
{
  int64_t _startIndex = offset/destLength;
  int64_t _endIndex = (offset+srcLength-1)/destLength;
  assert(_startIndex <= INT32_MAX);
  assert(_endIndex <= INT32_MAX);
  startIndex = _startIndex;
  endIndex = _endIndex;
}

std::string Util::getHomeDir()
{
  const char* p = getenv("HOME");
  if(p) {
    return p;
  } else {
    return A2STR::NIL;
  }
}

int64_t Util::getRealSize(const std::string& sizeWithUnit)
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
  int64_t v = Util::parseLLInt(size);

  if(v < 0) {
    throw DlAbortEx
      (StringFormat("Negative value detected: %s", sizeWithUnit.c_str()).str());
  } else if(INT64_MAX/mult < v) {
    throw DlAbortEx(StringFormat(MSG_STRING_INTEGER_CONVERSION_FAILURE,
				 "overflow/underflow").str());
  }
  return v*mult;
}

std::string Util::abbrevSize(int64_t size)
{
  if(size < 1024) {
    return Util::itos(size, true);
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
  return Util::itos(size, true)+"."+Util::itos(r*10/1024)+units[i]+"i";
}

time_t Util::httpGMT(const std::string& httpStdTime)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  char* r = strptime(httpStdTime.c_str(), "%a, %Y-%m-%d %H:%M:%S GMT", &tm);
  if(r != httpStdTime.c_str()+httpStdTime.size()) {
    return -1;
  }
  time_t thetime = timegm(&tm);
  if(thetime == -1) {
    if(tm.tm_year >= 2038-1900) {
      thetime = INT32_MAX;
    }
  }
  return thetime;
}

void Util::toStream(std::ostream& os, const FileEntries& fileEntries)
{
  os << _("Files:") << "\n";
  os << "idx|path/length" << "\n";
  os << "===+===========================================================================" << "\n";
  int32_t count = 1;
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); count++, itr++) {
    os << std::setw(3) << count << "|" << (*itr)->getPath() << "\n";
    os << "   |" << Util::abbrevSize((*itr)->getLength()) << "B" << "\n";
    os << "---+---------------------------------------------------------------------------" << "\n";
  }
}

void Util::sleep(long seconds) {
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

void Util::usleep(long microseconds) {
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

bool Util::isNumber(const std::string& what)
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

bool Util::isLowercase(const std::string& what)
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

bool Util::isUppercase(const std::string& what)
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

unsigned int Util::alphaToNum(const std::string& alphabets)
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

void Util::mkdirs(const std::string& dirpath)
{
  File dir(dirpath);
  if(dir.isDir()) {
    // do nothing
  } else if(dir.exists()) {
    throw DlAbortEx
      (StringFormat(EX_MAKE_DIR, dir.getPath().c_str(),
		    "File already exists.").str());
  } else if(!dir.mkdirs()) {
    throw DlAbortEx
      (StringFormat(EX_MAKE_DIR, dir.getPath().c_str(),
		    strerror(errno)).str());
  }
}

void Util::convertBitfield(BitfieldMan* dest, const BitfieldMan* src)
{
  size_t numBlock = dest->countBlock();
  for(size_t index = 0; index < numBlock; ++index) {
    if(src->isBitSetOffsetRange((uint64_t)index*dest->getBlockLength(),
				dest->getBlockLength())) {
      dest->setBit(index);
    }
  }
}

std::string Util::toString(const BinaryStreamHandle& binaryStream)
{
  std::stringstream strm;
  char data[2048];
  while(1) {
    int32_t dataLength = binaryStream->readData((unsigned char*)data, sizeof(data), strm.tellp());
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
void* Util::allocateAlignedMemory(size_t alignment, size_t size)
{
  void* buffer;
  int res;
  if((res = posix_memalign(&buffer, alignment, size)) != 0) {
    throw FatalException
      (StringFormat("Error in posix_memalign: %s", strerror(res)).str());
  }
  return buffer;
}
#endif // HAVE_POSIX_MEMALIGN

std::pair<std::string, uint16_t>
Util::getNumericNameInfo(const struct sockaddr* sockaddr, socklen_t len)
{
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  int s = getnameinfo(sockaddr, len, host, NI_MAXHOST, service, NI_MAXSERV,
		      NI_NUMERICHOST|NI_NUMERICSERV);
  if(s != 0) {
    throw DlAbortEx(StringFormat("Failed to get hostname and port. cause: %s",
				 gai_strerror(s)).str());
  }
  return std::pair<std::string, uint16_t>(host, atoi(service)); // TODO
}

} // namespace aria2
