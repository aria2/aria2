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
#ifndef _D_UTIL_H_
#define _D_UTIL_H_

#include "common.h"

#include <sys/time.h>

#include <string>
#include <utility>
#include <deque>
#include <iosfwd>
#include <ostream>
#include <numeric>
#include <map>
#include <iomanip>
#include <algorithm>

#include "SharedHandle.h"
#include "IntSequence.h"
#include "a2time.h"
#include "a2netcompat.h"
#include "a2functional.h"

namespace aria2 {

class Randomizer;
class BitfieldMan;
class BinaryStream;
class FileEntry;

#define STRTOLL(X) strtoll(X, (char**)0, 10)
#define STRTOULL(X) strtoull(X, (char**)0, 10)

#define START_INDEX(OFFSET, PIECE_LENGTH) ((OFFSET)/(PIECE_LENGTH))
#define END_INDEX(OFFSET, LENGTH, PIECE_LENGTH) (((OFFSET)+(LENGTH)-1)/(PIECE_LENGTH))

#define DIV_FLOOR(X,Y) ((X)/(Y)+((X)%(Y)? 1:0))

#ifdef WORDS_BIGENDIAN
inline uint64_t ntoh64(uint64_t x) { return x; }
inline uint64_t hton64(uint64_t x) { return x; }
#else // !WORDS_BIGENDIAN
inline uint64_t byteswap64(uint64_t x) {
  uint64_t v1 = ntohl(x & 0x00000000ffffffff);
  uint64_t v2 = ntohl(x >> 32);
  return (v1 << 32)|v2;
}
inline uint64_t ntoh64(uint64_t x) { return byteswap64(x); }
inline uint64_t hton64(uint64_t x) { return byteswap64(x); }
#endif // !WORDS_BIGENDIAN

namespace util {

void split(std::pair<std::string, std::string>& hp,
	   const std::string& src, char delim);

std::pair<std::string, std::string>
split(const std::string& src, const std::string& delims);

template<typename T>
std::string uitos(T value, bool comma = false)
{
  std::string str;
  if(value == 0) {
    str = "0";
    return str;
  }
  unsigned int count = 0;
  while(value) {
    ++count;
    char digit = value%10+'0';
    if(comma && count > 3 && count%3 == 1) {
      str += ',';
    }
    str += digit;
    value /= 10;
  }
  std::reverse(str.begin(), str.end());
  return str;
}

template<typename T>
std::string itos(T value, bool comma = false)
{
  bool flag = false;
  if(value < 0) {
    flag = true;
    value = -value;
  }
  std::string str = uitos(value, comma);
  if(flag) {
    str.insert(str.begin(), '-');
  }
  return str;
}

/**
 * Computes difference in micro-seconds between tv1 and tv2,
 * assuming tv1 is newer than tv2.
 * If tv1 is older than tv2, then this method returns 0.
 */
int64_t difftv(struct timeval tv1, struct timeval tv2);
int32_t difftvsec(struct timeval tv1, struct timeval tv2);

extern const std::string DEFAULT_TRIM_CHARSET;

std::string trim(const std::string& src,
		 const std::string& trimCharset = DEFAULT_TRIM_CHARSET);

void trimSelf(std::string& str,
	      const std::string& trimCharset = DEFAULT_TRIM_CHARSET);

bool startsWith(const std::string& target, const std::string& part);

bool endsWith(const std::string& target, const std::string& part);

std::string replace(const std::string& target, const std::string& oldstr, const std::string& newstr);

std::string urlencode(const unsigned char* target, size_t len);

std::string urlencode(const std::string& target);

bool inRFC3986ReservedChars(const char c);

bool inRFC3986UnreservedChars(const char c);

std::string urldecode(const std::string& target);

std::string torrentUrlencode(const unsigned char* target, size_t len);

std::string torrentUrlencode(const std::string& target);

std::string toHex(const unsigned char* src, size_t len);

std::string toHex(const char* src, size_t len);

std::string toHex(const std::string& src);

FILE* openFile(const std::string& filename, const std::string& mode);

bool isPowerOf(int num, int base);

std::string secfmt(time_t sec);

int32_t parseInt(const std::string& s, int32_t base = 10);

uint32_t parseUInt(const std::string& s, int base = 10);
  
bool parseUIntNoThrow(uint32_t& result, const std::string& s, int base = 10);

int64_t parseLLInt(const std::string& s, int32_t base = 10);

uint64_t parseULLInt(const std::string& s, int base = 10);

IntSequence parseIntRange(const std::string& src);

// this function temporarily put here
std::string getContentDispositionFilename(const std::string& header);

std::string randomAlpha(size_t length,
			const SharedHandle<Randomizer>& randomizer);
  
std::string toUpper(const std::string& src);

std::string toLower(const std::string& src);

bool isNumbersAndDotsNotation(const std::string& name);

void setGlobalSignalHandler(int signal, void (*handler)(int), int flags);

std::string getHomeDir();

int64_t getRealSize(const std::string& sizeWithUnit);

std::string abbrevSize(int64_t size);

template<typename InputIterator>
void toStream
(InputIterator first, InputIterator last, std::ostream& os)
{
  os << _("Files:") << "\n";
  os << "idx|path/length" << "\n";
  os << "===+===========================================================================" << "\n";
  int32_t count = 1;
  for(; first != last; ++first, ++count) {
    os << std::setw(3) << count << "|" << (*first)->getPath() << "\n";
    os << "   |" << util::abbrevSize((*first)->getLength()) << "B ("
       << util::uitos((*first)->getLength(), true) << ")\n";
    os << "---+---------------------------------------------------------------------------" << "\n";
  }
}

void sleep(long seconds);

void usleep(long microseconds);
  
bool isNumber(const std::string& what);
  
bool isLowercase(const std::string& what);
  
bool isUppercase(const std::string& what);
  
/**
 * Converts alphabets to unsigned int, assuming alphabets as a base 26
 * integer and 'a' or 'A' is 0.
 * This function assumes alphabets includes only a-z.
 * Upper case are allowed but all letters must be upper case.
 * If overflow occurs, returns 0.
 */
unsigned int alphaToNum(const std::string& alphabets);

void mkdirs(const std::string& dirpath);

void convertBitfield(BitfieldMan* dest, const BitfieldMan* src);

// binaryStream has to be opened before calling this function.
std::string toString(const SharedHandle<BinaryStream>& binaryStream);

#ifdef HAVE_POSIX_MEMALIGN
void* allocateAlignedMemory(size_t alignment, size_t size);
#endif // HAVE_POSIX_MEMALIGN

std::pair<std::string, uint16_t>
getNumericNameInfo(const struct sockaddr* sockaddr, socklen_t len);

std::string htmlEscape(const std::string& src);

// Joins path element specified in [first, last).  If ".." is found,
// it eats the previous element if it exists.  If "." is found, it
// is just ignored and it is not appeared in the result.
template<typename InputIterator>
std::string joinPath(InputIterator first, InputIterator last)
{
  std::deque<std::string> elements;
  for(;first != last; ++first) {
    if(*first == "..") {
      if(!elements.empty()) {
	elements.pop_back();
      }
    } else if(*first == ".") {
      // do nothing
    } else {
      elements.push_back(*first);
    }
  }
  return strjoin(elements.begin(), elements.end(), "/");
}

// Parses INDEX=PATH format string. INDEX must be an unsigned
// integer.
std::map<size_t, std::string>::value_type
parseIndexPath(const std::string& line);

std::map<size_t, std::string> createIndexPathMap(std::istream& i);

/**
 * Take a string src which is a deliminated list and add its elements
 * into result. result is stored in out.
 */
template<typename OutputIterator>
OutputIterator split(const std::string& src, OutputIterator out,
		     const std::string& delims, bool doTrim = false)
{
  std::string::size_type p = 0;
  while(1) {
    std::string::size_type np = src.find_first_of(delims, p);
    if(np == std::string::npos) {
      std::string term = src.substr(p);
      if(doTrim) {
	term = util::trim(term);
      }
      if(!term.empty()) {
	*out = term;
	++out;
      }
      break;
    }
    std::string term = src.substr(p, np-p);
    if(doTrim) {
      term = util::trim(term);
    }
    p = np+1;
    if(!term.empty()) {
      *out = term;
      ++out;
    }
  }
  return out;
}

void generateRandomData(unsigned char* data, size_t length);

} // namespace util

} // namespace aria2

#endif // _D_UTIL_H_
