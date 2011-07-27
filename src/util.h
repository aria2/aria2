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
#ifndef D_UTIL_H
#define D_UTIL_H

#include "common.h"

#include <sys/time.h>

#include <cstdio>
#include <string>
#include <utility>
#include <iosfwd>
#include <ostream>
#include <numeric>
#include <map>
#include <iomanip>
#include <algorithm>
#include <vector>

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
class RequestGroup;
class Option;

#define STRTOLL(X) strtoll(X, reinterpret_cast<char**>(0), 10)
#define STRTOULL(X) strtoull(X, reinterpret_cast<char**>(0), 10)

#define START_INDEX(OFFSET, PIECE_LENGTH) ((OFFSET)/(PIECE_LENGTH))
#define END_INDEX(OFFSET, LENGTH, PIECE_LENGTH) (((OFFSET)+(LENGTH)-1)/(PIECE_LENGTH))

#define DIV_FLOOR(X,Y) ((X)/(Y)+((X)%(Y)? 1:0))

#ifdef WORDS_BIGENDIAN
inline uint64_t ntoh64(uint64_t x) { return x; }
inline uint64_t hton64(uint64_t x) { return x; }
#else // !WORDS_BIGENDIAN
inline uint64_t byteswap64(uint64_t x) {
  uint64_t v1 = ntohl(x & 0x00000000ffffffffllu);
  uint64_t v2 = ntohl(x >> 32);
  return (v1 << 32)|v2;
}
inline uint64_t ntoh64(uint64_t x) { return byteswap64(x); }
inline uint64_t hton64(uint64_t x) { return byteswap64(x); }
#endif // !WORDS_BIGENDIAN

namespace util {

void divide
(std::pair<std::string, std::string>& hp, const std::string& src, char delim);

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

std::string itos(int64_t value, bool comma = false);

/**
 * Computes difference in micro-seconds between tv1 and tv2,
 * assuming tv1 is newer than tv2.
 * If tv1 is older than tv2, then this method returns 0.
 */
int64_t difftv(struct timeval tv1, struct timeval tv2);
int32_t difftvsec(struct timeval tv1, struct timeval tv2);

extern const std::string DEFAULT_STRIP_CHARSET;

template<typename InputIterator>
std::string stripIter
(InputIterator first, InputIterator last,
 const std::string& chars = DEFAULT_STRIP_CHARSET)
{
  if(std::distance(first, last) == 0) {
    return A2STR::NIL;
  }
  for(; first != last &&
        std::find(chars.begin(), chars.end(), *first) != chars.end(); ++first);
  if(first == last) {
    return A2STR::NIL;
  }
  InputIterator left = last-1;
  for(; left != first &&
        std::find(chars.begin(), chars.end(), *left) != chars.end(); --left);
  return std::string(first, left+1);
}

std::string strip
(const std::string& str, const std::string& chars = DEFAULT_STRIP_CHARSET);

bool startsWith(const std::string& target, const std::string& part);

bool endsWith(const std::string& target, const std::string& part);

std::string replace(const std::string& target, const std::string& oldstr, const std::string& newstr);

std::string percentEncode(const unsigned char* target, size_t len);

std::string percentEncode(const std::string& target);

bool inRFC3986ReservedChars(const char c);

bool inRFC3986UnreservedChars(const char c);

bool isUtf8(const std::string& str);

std::string percentDecode(const std::string& target);

std::string torrentPercentEncode(const unsigned char* target, size_t len);

std::string torrentPercentEncode(const std::string& target);

std::string toHex(const unsigned char* src, size_t len);

std::string toHex(const char* src, size_t len);

std::string toHex(const std::string& src);

// Converts hexadecimal ascii string 'src' into packed binary form and
// return the result. If src is not well formed, then empty string is
// returned.
std::string fromHex(const std::string& src);

FILE* openFile(const std::string& filename, const std::string& mode);

bool isPowerOf(int num, int base);

std::string secfmt(time_t sec);

int32_t parseInt(const std::string& s, int32_t base = 10);

bool parseIntNoThrow(int32_t& result, const std::string& s, int base = 10);

uint32_t parseUInt(const std::string& s, int base = 10);
  
bool parseUIntNoThrow(uint32_t& result, const std::string& s, int base = 10);

int64_t parseLLInt(const std::string& s, int32_t base = 10);

bool parseLLIntNoThrow(int64_t& result, const std::string& s, int base = 10);

uint64_t parseULLInt(const std::string& s, int base = 10);

IntSequence parseIntRange(const std::string& src);

// Parses string which specifies the range of piece index for higher
// priority and appends those indexes into result.  The input string
// src can contain 2 keywords "head" and "tail".  To include both
// keywords, they must be separated by comma.  "head" means the pieces
// where the first byte of each file sits.  "tail" means the pieces
// where the last byte of each file sits.  These keywords can take one
// parameter, SIZE. For example, if "head=SIZE" is specified, pieces
// in the range of first SIZE bytes of each file get higher
// priority. SIZE can include K or M(1K = 1024, 1M = 1024K).
// If SIZE is omitted, SIZE=defaultSize is used.
//
// sample: head=512K,tail=512K
void parsePrioritizePieceRange
(std::vector<size_t>& result, const std::string& src,
 const std::vector<SharedHandle<FileEntry> >& fileEntries,
 size_t pieceLength,
 uint64_t defaultSize = 1048576 /* 1MiB */);

// Converts ISO/IEC 8859-1 string src to utf-8.
std::string iso8859ToUtf8(const std::string& src);

std::string getContentDispositionFilename(const std::string& header);

std::string randomAlpha(size_t length,
                        const SharedHandle<Randomizer>& randomizer);
  
std::string toUpper(const std::string& src);

std::string toLower(const std::string& src);

void uppercase(std::string& s);

void lowercase(std::string& s);

bool isNumericHost(const std::string& name);

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

bool isDigit(const char c);

bool isHexDigit(const char c);

bool isHexDigit(const std::string& s);
  
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
  std::vector<std::string> elements;
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
 * Take a string src which is a delimited list and add its elements
 * into result. result is stored in out.
 */
template<typename OutputIterator>
OutputIterator split(const std::string& src, OutputIterator out,
                     const std::string& delims, bool doStrip = false,
                     bool allowEmpty = false)
{
  std::string::const_iterator first = src.begin();
  std::string::const_iterator last = src.end();
  for(std::string::const_iterator i = first; i != last;) {
    std::string::const_iterator j = i;
    for(; j != last &&
          std::find(delims.begin(), delims.end(), *j) == delims.end(); ++j);
    std::string t = doStrip?util::stripIter(i, j):std::string(i, j);
    if(allowEmpty || !t.empty()) {
      *out++ = t;
    }
    i = j;
    if(j != last) {
      ++i;
    }
  }
  if(allowEmpty &&
     (src.empty() ||
      std::find(delims.begin(), delims.end(),
                src[src.size()-1]) != delims.end())) {
    *out++ = A2STR::NIL;
  }
  return out;
}

void generateRandomData(unsigned char* data, size_t length);

// Saves data to file whose name is filename. If overwrite is true,
// existing file is overwritten. Otherwise, this function doesn't do
// nothing.  If data is saved successfully, return true. Otherwise
// returns false.
bool saveAs
(const std::string& filename, const std::string& data, bool overwrite=false);

// Prepend dir to relPath. If dir is empty, it prepends "." to relPath.
//
// dir = "/dir", relPath = "foo" => "/dir/foo"
// dir = "",     relPath = "foo" => "./foo"
// dir = "/",    relPath = "foo" => "/foo"
std::string applyDir(const std::string& dir, const std::string& relPath);

// In HTTP/FTP, file name is file component in URI. In HTTP, filename
// may be a value of Content-Disposition header.  They are likely
// percent encoded. If they contains, for example, %2F, when decoded,
// basename contains dir component. This should be avoided.  This
// function is created to fix these issues.  This function expects src
// should be non-percent-encoded basename.  Currently, this function
// replaces '/' with '_' and result string is passed to escapePath()
// function and its result is returned.
std::string fixTaintedBasename(const std::string& src);

// Generates 20 bytes random key and store it to the address pointed
// by key.  Caller must allocate at least 20 bytes for generated key.
void generateRandomKey(unsigned char* key);

// Returns true is given numeric ipv4addr is in Private Address Space.
bool inPrivateAddress(const std::string& ipv4addr);

// Returns true if s contains directory traversal path component such
// as '..' or it contains null or control character which may fool
// user.
bool detectDirTraversal(const std::string& s);

// Replaces null(0x00) and control character(0x01-0x1f) with '_'. If
// __MINGW32__ is defined, following characters are also replaced with
// '_': '"', '*', ':', '<', '>', '?', '\', '|'.
std::string escapePath(const std::string& s);

// Stores network address of numeric IPv4 address ip using CIDR bits
// into in.  On success, returns true. Otherwise returns false.
bool getCidrPrefix(struct in_addr& in, const std::string& ip, int bits);

// Returns true if ip1 and ip2 are in the same CIDR block.
bool inSameCidrBlock(const std::string& ip1, const std::string& ip2, int bits);

void removeMetalinkContentTypes(const SharedHandle<RequestGroup>& group);
void removeMetalinkContentTypes(RequestGroup* group);

// No throw
void executeHookByOptName
(const SharedHandle<RequestGroup>& group, const Option* option,
 const std::string& opt);

// No throw
void executeHookByOptName
(const RequestGroup* group, const Option* option, const std::string& opt);

std::string createSafePath(const std::string& dir, const std::string& filename);

std::string encodeNonUtf8(const std::string& s);

// Create string safely. If str is NULL, returns empty string.
// Otherwise, returns std::string(str).
std::string makeString(const char* str);

// This function is basically the same with strerror(errNum) but when
// strerror returns NULL, this function returns empty string.
std::string safeStrerror(int errNum);

// Parses sequence [first, last) and find name=value pair delimited by
// delim character. If name(and optionally value) is found, returns
// pair of iterator which can use as first parameter of next call of
// this function, and true. If no name is found, returns the pair of
// last and false.
template<typename Iterator>
std::pair<Iterator, bool>
nextParam
(std::string& name,
 std::string& value,
 Iterator first,
 Iterator last,
 char delim)
{
  Iterator end = last;
  while(first != end) {
    last = first;
    Iterator parmnameFirst = first;
    Iterator parmnameLast = first;
    bool eqFound = false;
    for(; last != end; ++last) {
      if(*last == delim) {
        break;
      } else if(!eqFound && *last == '=') {
        eqFound = true;
        parmnameFirst = first;
        parmnameLast = last;
      }
    }
    std::string tname, tvalue;
    if(parmnameFirst == parmnameLast) {
      if(!eqFound) {
        parmnameFirst = first;
        parmnameLast = last;
        tname = util::stripIter(parmnameFirst, parmnameLast);
      }
    } else {
      first = parmnameLast+1;
      tname = util::stripIter(parmnameFirst, parmnameLast);
      tvalue = util::stripIter(first, last);
    }
    if(last != end) {
      ++last;
    }
    if(!tname.empty()) {
      name.swap(tname);
      value.swap(tvalue);
      return std::make_pair(last, true);
    }
    first = last;
  }
  return std::make_pair(end, false);
}

} // namespace util

} // namespace aria2

#endif // D_UTIL_H
