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
#include <numeric>
#include <map>

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

class Util {
public:
  static void split(std::pair<std::string, std::string>& hp,
		    const std::string& src, char delim);

  static std::pair<std::string, std::string>
  split(const std::string& src, const std::string& delims);

  template<typename T>
  static std::string uitos(T value, bool comma = false)
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
      str.insert(str.begin(), digit);
      value /= 10;
      if(comma && count > 3 && count%3 == 1) {
	str.insert(str.begin()+1, ',');
      }
    }
    return str;
  }

  template<typename T>
  static std::string itos(T value, bool comma = false)
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
  static int64_t difftv(struct timeval tv1, struct timeval tv2);
  static int32_t difftvsec(struct timeval tv1, struct timeval tv2);
  /**
   * Take a string src which is a deliminated list and add its elements
   * into result. result is not cleared before conversion begins.
   */
  static void slice(std::deque<std::string>& result, const std::string& src,
		    char delim, bool trim = false);
  
  static const std::string DEFAULT_TRIM_CHARSET;

  static std::string trim(const std::string& src,
			  const std::string& trimCharset = DEFAULT_TRIM_CHARSET);

  static void trimSelf(std::string& str,
		       const std::string& trimCharset = DEFAULT_TRIM_CHARSET);

  static bool startsWith(const std::string& target, const std::string& part);

  static bool endsWith(const std::string& target, const std::string& part);

  static std::string replace(const std::string& target, const std::string& oldstr, const std::string& newstr);

  static std::string urlencode(const unsigned char* target, size_t len);

  static std::string urlencode(const std::string& target)
  {
    return urlencode((const unsigned char*)target.c_str(), target.size());
  }

  static bool inRFC3986ReservedChars(const char c);

  static bool inRFC3986UnreservedChars(const char c);

  static std::string urldecode(const std::string& target);

  static std::string torrentUrlencode(const unsigned char* target, size_t len);

  static std::string torrentUrlencode(const std::string& target)
  {
    return torrentUrlencode(reinterpret_cast<const unsigned char*>(target.c_str()),
			    target.size());
  }

  static std::string toHex(const unsigned char* src, size_t len);

  static std::string toHex(const char* src, size_t len)
  {
    return toHex(reinterpret_cast<const unsigned char*>(src), len);
  }

  static std::string toHex(const std::string& src)
  {
    return toHex(reinterpret_cast<const unsigned char*>(src.c_str()), src.size());
  }

  static FILE* openFile(const std::string& filename, const std::string& mode);

  static void fileCopy(const std::string& destFile, const std::string& src);

  static void rangedFileCopy(const std::string& destFile, const std::string& src, off_t srcOffset, uint64_t length);

  static bool isPowerOf(int num, int base);

  static std::string secfmt(time_t sec);

  static size_t expandBuffer(char** pbuf, size_t curLength, size_t newLength);

  static void unfoldRange(const std::string& src, std::deque<int>& range);

  static int32_t parseInt(const std::string& s, int32_t base = 10);

  static uint32_t parseUInt(const std::string& s, int base = 10);
  
  static int64_t parseLLInt(const std::string& s, int32_t base = 10);

  static uint64_t parseULLInt(const std::string& s, int base = 10);

  static IntSequence parseIntRange(const std::string& src);

  // this function temporarily put here
  static std::string getContentDispositionFilename(const std::string& header);

  static unsigned int countBit(uint32_t n);
  
  static std::string randomAlpha(size_t length,
				 const SharedHandle<Randomizer>& randomizer);
  
  static std::string toUpper(const std::string& src);

  static std::string toLower(const std::string& src);

  static bool isNumbersAndDotsNotation(const std::string& name);

  static void setGlobalSignalHandler(int signal, void (*handler)(int), int flags);

  static void indexRange(size_t& startIndex, size_t& endIndex,
			 off_t offset,
			 size_t srcLength, size_t destLength);

  static std::string getHomeDir();

  static int64_t getRealSize(const std::string& sizeWithUnit);

  static std::string abbrevSize(int64_t size);

  /**
   * Parses given httpTimeFormat and returns seconds ellapsed since epoc.
   * The available format is "%a, %Y-%m-%d %H:%M:%S GMT".
   * If specified date is later than "Tue, 2038-01-19 3:14:7 GMT",
   * this function returns INT32_MAX.
   * This function also cannot handle prior 1900-1-1 0:0:0 GMT.
   * If parse operation is failed, then return -1.
   */
  static time_t httpGMT(const std::string& httpTimeFormat);

  static void toStream(std::ostream& os,
		       const std::deque<SharedHandle<FileEntry> >& entries);

  static void sleep(long seconds);

  static void usleep(long microseconds);
  
  static bool isNumber(const std::string& what);
  
  static bool isLowercase(const std::string& what);
  
  static bool isUppercase(const std::string& what);
  
  /**
   * Converts alphabets to unsigned int, assuming alphabets as a base 26
   * integer and 'a' or 'A' is 0.
   * This function assumes alphabets includes only a-z.
   * Upper case are allowed but all letters must be upper case.
   * If overflow occurs, returns 0.
   */
  static unsigned int alphaToNum(const std::string& alphabets);

  static void mkdirs(const std::string& dirpath);

  static void convertBitfield(BitfieldMan* dest, const BitfieldMan* src);

  // binaryStream has to be opened before calling this function.
  static std::string toString(const SharedHandle<BinaryStream>& binaryStream);

#ifdef HAVE_POSIX_MEMALIGN
  static void* allocateAlignedMemory(size_t alignment, size_t size);
#endif // HAVE_POSIX_MEMALIGN

  static std::pair<std::string, uint16_t>
  getNumericNameInfo(const struct sockaddr* sockaddr, socklen_t len);

  static std::string htmlEscape(const std::string& src);

  // Joins path element specified in [first, last).  If ".." is found,
  // it eats the previous element if it exists.  If "." is found, it
  // is just ignored and it is not appeared in the result.
  template<typename InputIterator>
  static std::string joinPath(InputIterator first, InputIterator last)
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
    if(elements.empty()) {
      return "";
    }
    return std::accumulate(elements.begin()+1, elements.end(), elements[0],
			   Concat("/"));
  }

  // Parses INDEX=PATH format string. INDEX must be an unsigned
  // integer.
  static std::map<size_t, std::string>::value_type
  parseIndexPath(const std::string& line);

  static std::map<size_t, std::string> createIndexPathMap(std::istream& i);
};

} // namespace aria2

#endif // _D_UTIL_H_
