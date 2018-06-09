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
#include <limits.h>
#include <stdint.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <iosfwd>
#include <ostream>
#include <numeric>
#include <map>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <memory>

#include "a2time.h"
#include "a2netcompat.h"
#include "a2functional.h"
#include "SegList.h"
#include "a2iterator.h"
#include "message.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "prefs.h"

#ifdef ENABLE_SSL
#  include "TLSContext.h"
#endif // ENABLE_SSL

#ifndef HAVE_SIGACTION
#  define sigset_t int
#endif // HAVE_SIGACTION

namespace aria2 {

class Randomizer;
class BitfieldMan;
class BinaryStream;
class FileEntry;
class RequestGroup;
class Option;

#ifdef WORDS_BIGENDIAN
inline uint64_t ntoh64(uint64_t x) { return x; }
inline uint64_t hton64(uint64_t x) { return x; }
#else  // !WORDS_BIGENDIAN
inline uint64_t byteswap64(uint64_t x)
{
  uint64_t v1 = ntohl(x & 0x00000000ffffffffllu);
  uint64_t v2 = ntohl(x >> 32);
  return (v1 << 32) | v2;
}
inline uint64_t ntoh64(uint64_t x) { return byteswap64(x); }
inline uint64_t hton64(uint64_t x) { return byteswap64(x); }
#endif // !WORDS_BIGENDIAN

#ifdef __MINGW32__
std::wstring utf8ToWChar(const std::string& src);

std::wstring utf8ToWChar(const char* str);

std::string wCharToUtf8(const std::wstring& wsrc);

// replace any backslash '\' in |src| with '/' and returns it.
std::string toForwardSlash(const std::string& src);
#else // !__MINGW32__
#  define utf8ToWChar(src) src
#  define utf8ToNative(src) src
#endif // !__MINGW32__

namespace util {

extern const char DEFAULT_STRIP_CHARSET[];

template <typename InputIterator>
std::pair<InputIterator, InputIterator>
stripIter(InputIterator first, InputIterator last,
          const char* chars = DEFAULT_STRIP_CHARSET)
{
  for (; first != last && strchr(chars, *first) != 0; ++first)
    ;
  if (first == last) {
    return std::make_pair(first, last);
  }
  InputIterator left = last - 1;
  for (; left != first && strchr(chars, *left) != 0; --left)
    ;
  return std::make_pair(first, left + 1);
}

template <typename InputIterator>
InputIterator lstripIter(InputIterator first, InputIterator last, char ch)
{
  for (; first != last && *first == ch; ++first)
    ;
  return first;
}

template <typename InputIterator>
InputIterator lstripIter(InputIterator first, InputIterator last,
                         const char* chars)
{
  for (; first != last && strchr(chars, *first) != 0; ++first)
    ;
  return first;
}

template <typename InputIterator>
InputIterator lstripIter(InputIterator first, InputIterator last)
{
  return lstripIter(first, last, DEFAULT_STRIP_CHARSET);
}

std::string strip(const std::string& str,
                  const char* chars = DEFAULT_STRIP_CHARSET);

template <typename InputIterator>
std::pair<std::pair<InputIterator, InputIterator>,
          std::pair<InputIterator, InputIterator>>
divide(InputIterator first, InputIterator last, char delim, bool strip = true)
{
  auto dpos = std::find(first, last, delim);
  if (dpos == last) {
    if (strip) {
      return {stripIter(first, last), {last, last}};
    }
    return {{first, last}, {last, last}};
  }

  if (strip) {
    return {stripIter(first, dpos), stripIter(dpos + 1, last)};
  }
  return {{first, dpos}, {dpos + 1, last}};
}

template <typename T> std::string uitos(T n, bool comma = false)
{
  std::string res;
  if (n == 0) {
    res = "0";
    return res;
  }
  int i = 0;
  T t = n;
  for (; t; t /= 10, ++i)
    ;
  if (comma) {
    i += (i - 1) / 3;
  }
  res.resize(i);
  --i;
  for (int j = 0; n; --i, ++j, n /= 10) {
    res[i] = (n % 10) + '0';
    if (comma && i > 1 && (j + 1) % 3 == 0) {
      res[--i] = ',';
    }
  }
  return res;
}

std::string itos(int64_t value, bool comma = false);

/**
 * Computes difference in micro-seconds between tv1 and tv2,
 * assuming tv1 is newer than tv2.
 * If tv1 is older than tv2, then this method returns 0.
 */
int64_t difftv(struct timeval tv1, struct timeval tv2);
int32_t difftvsec(struct timeval tv1, struct timeval tv2);

std::string replace(const std::string& target, const std::string& oldstr,
                    const std::string& newstr);

std::string percentEncode(const unsigned char* target, size_t len);

std::string percentEncode(const std::string& target);

std::string percentEncodeMini(const std::string& target);

bool inRFC3986ReservedChars(const char c);

bool inRFC3986UnreservedChars(const char c);

bool inRFC2978MIMECharset(const char c);

bool inRFC2616HttpToken(const char c);

bool inRFC5987AttrChar(const char c);

// Returns true if |c| is in ISO/IEC 8859-1 character set.
bool isIso8859p1(unsigned char c);

bool isUtf8(const std::string& str);

std::string percentDecode(std::string::const_iterator first,
                          std::string::const_iterator last);

std::string torrentPercentEncode(const unsigned char* target, size_t len);

std::string torrentPercentEncode(const std::string& target);

std::string toHex(const unsigned char* src, size_t len);

std::string toHex(const char* src, size_t len);

std::string toHex(const std::string& src);

unsigned int hexCharToUInt(unsigned char ch);

// Converts hexadecimal ascii characters in [first, last) into packed
// binary form and return the result. If characters in given range is
// not well formed, then empty string is returned.
template <typename InputIterator>
std::string fromHex(InputIterator first, InputIterator last)
{
  std::string dest;
  size_t len = last - first;
  if (len % 2) {
    return dest;
  }
  for (; first != last; first += 2) {
    unsigned char high = hexCharToUInt(*first);
    unsigned char low = hexCharToUInt(*(first + 1));
    if (high == 255 || low == 255) {
      dest.clear();
      return dest;
    }
    dest += (high * 16 + low);
  }
  return dest;
}

std::string secfmt(time_t sec);

bool parseIntNoThrow(int32_t& res, const std::string& s, int base = 10);

// Valid range: [0, INT32_MAX]
bool parseUIntNoThrow(uint32_t& res, const std::string& s, int base = 10);

bool parseLLIntNoThrow(int64_t& res, const std::string& s, int base = 10);

// Parses |s| as floating point number, and stores the result into
// |res|.  This function returns true if it succeeds.
bool parseDoubleNoThrow(double& res, const std::string& s);

SegList<int> parseIntSegments(const std::string& src);

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
void parsePrioritizePieceRange(
    std::vector<size_t>& result, const std::string& src,
    const std::vector<std::shared_ptr<FileEntry>>& fileEntries,
    size_t pieceLength, int64_t defaultSize = 1048576 /* 1MiB */);

// Converts ISO/IEC 8859-1 string src to utf-8.
std::string iso8859p1ToUtf8(const char* src, size_t len);
std::string iso8859p1ToUtf8(const std::string& src);

// Parses Content-Disposition header field value |in| with its length
// |len| in a manner conforming to RFC 6266 and extracts filename
// value and copies it to the region pointed by |dest|. The |destlen|
// specifies the capacity of the |dest|. This function does not store
// NUL character after filename in |dest|. This function does not
// support RFC 2231 Continuation. If the function sees RFC 2231/5987
// encoding and charset, it stores its first pointer to |*charsetp|
// and its length in |*charsetlenp|. Otherwise, they are NULL and 0
// respectively.  In RFC 2231/5987 encoding, percent-encoded string
// will be decoded to original form and stored in |dest|.
//
// This function returns the number of written bytes in |dest| if it
// succeeds, or -1. If there is enough room to store filename in
// |dest|, this function returns -1. If this function returns -1, the
// |dest|, |*charsetp| and |*charsetlenp| are undefined.
//
// It will handle quoted string(as in RFC 7230 section 3.2.6) as
// ISO-8859-1 by default, or UTF-8 if defaultUTF8 == true.
ssize_t parse_content_disposition(char* dest, size_t destlen,
                                  const char** charsetp, size_t* charsetlenp,
                                  const char* in, size_t len, bool defaultUTF8);

std::string getContentDispositionFilename(const std::string& header,
                                          bool defaultUTF8);

std::string toUpper(std::string src);

std::string toLower(std::string src);

void uppercase(std::string& s);

void lowercase(std::string& s);

char toUpperChar(char c);

char toLowerChar(char c);

bool isNumericHost(const std::string& name);

typedef void (*signal_handler_t)(int);
void setGlobalSignalHandler(int signal, sigset_t* mask,
                            signal_handler_t handler, int flags);

std::string getHomeDir();

std::string getXDGDir(const std::string& environmentVariable,
                      const std::string& fallbackDirectory);

std::string getConfigFile();

std::string getDHTFile(bool ipv6);

int64_t getRealSize(const std::string& sizeWithUnit);

std::string abbrevSize(int64_t size);

template <typename InputIterator, typename Output>
void toStream(InputIterator first, InputIterator last, Output& os)
{
  os.printf("%s\n"
            "idx|path/length\n"
            "===+=============================================================="
            "=============\n",
            _("Files:"));
  int32_t count = 1;
  for (; first != last; ++first, ++count) {
    os.printf("%3d|%s\n"
              "   |%sB (%s)\n"
              "---+------------------------------------------------------------"
              "---------------\n",
              count, (*first)->getPath().c_str(),
              util::abbrevSize((*first)->getLength()).c_str(),
              util::uitos((*first)->getLength(), true).c_str());
  }
}

void sleep(long seconds);

void usleep(long microseconds);

template <typename InputIterator>
bool isNumber(InputIterator first, InputIterator last)
{
  if (first == last) {
    return false;
  }
  for (; first != last; ++first) {
    if ('0' > *first || *first > '9') {
      return false;
    }
  }
  return true;
}

bool isAlpha(const char c);

bool isDigit(const char c);

bool isHexDigit(const char c);

bool isHexDigit(const std::string& s);

bool isLws(const char c);

bool isCRLF(const char c);

template <typename InputIterator>
bool isLowercase(InputIterator first, InputIterator last)
{
  if (first == last) {
    return false;
  }
  for (; first != last; ++first) {
    if ('a' > *first || *first > 'z') {
      return false;
    }
  }
  return true;
}

template <typename InputIterator>
bool isUppercase(InputIterator first, InputIterator last)
{
  if (first == last) {
    return false;
  }
  for (; first != last; ++first) {
    if ('A' > *first || *first > 'Z') {
      return false;
    }
  }
  return true;
}

void mkdirs(const std::string& dirpath);

void convertBitfield(BitfieldMan* dest, const BitfieldMan* src);

// binaryStream has to be opened before calling this function.
std::string toString(const std::shared_ptr<BinaryStream>& binaryStream);

#ifdef HAVE_POSIX_MEMALIGN
void* allocateAlignedMemory(size_t alignment, size_t size);
#endif // HAVE_POSIX_MEMALIGN

Endpoint getNumericNameInfo(const struct sockaddr* sockaddr, socklen_t len);

std::string htmlEscape(const std::string& src);

// Joins path element specified in [first, last).  If ".." is found,
// it eats the previous element if it exists.  If "." is found, it
// is just ignored and it is not appeared in the result.
template <typename InputIterator>
std::string joinPath(InputIterator first, InputIterator last)
{
  std::vector<std::string> elements;
  for (; first != last; ++first) {
    if (*first == "..") {
      if (!elements.empty()) {
        elements.pop_back();
      }
    }
    else if (*first == ".") {
      // do nothing
    }
    else {
      elements.push_back(*first);
    }
  }
  return strjoin(elements.begin(), elements.end(), "/");
}

// Parses INDEX=PATH format string. INDEX must be an unsigned
// integer.
std::pair<size_t, std::string> parseIndexPath(const std::string& line);

std::vector<std::pair<size_t, std::string>> createIndexPaths(std::istream& i);

/**
 * Take a string [first, last) which is a delimited list and add its
 * elements into result as iterator pair. result is stored in out.
 */
template <typename InputIterator, typename OutputIterator>
OutputIterator splitIter(InputIterator first, InputIterator last,
                         OutputIterator out, char delim, bool doStrip = false,
                         bool allowEmpty = false)
{
  for (InputIterator i = first; i != last;) {
    InputIterator j = std::find(i, last, delim);
    std::pair<InputIterator, InputIterator> p(i, j);
    if (doStrip) {
      p = stripIter(i, j);
    }
    if (allowEmpty || p.first != p.second) {
      *out++ = p;
    }
    i = j;
    if (j != last) {
      ++i;
    }
  }
  if (allowEmpty && (first == last || *(last - 1) == delim)) {
    *out++ = std::make_pair(last, last);
  }
  return out;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator splitIterM(InputIterator first, InputIterator last,
                          OutputIterator out, const char* delims,
                          bool doStrip = false, bool allowEmpty = false)
{
  size_t numDelims = strlen(delims);
  const char* dlast = delims + numDelims;
  for (InputIterator i = first; i != last;) {
    InputIterator j = i;
    for (; j != last && std::find(delims, dlast, *j) == dlast; ++j)
      ;
    std::pair<InputIterator, InputIterator> p(i, j);
    if (doStrip) {
      p = stripIter(i, j);
    }
    if (allowEmpty || p.first != p.second) {
      *out++ = p;
    }
    i = j;
    if (j != last) {
      ++i;
    }
  }
  if (allowEmpty &&
      (first == last || std::find(delims, dlast, *(last - 1)) != dlast)) {
    *out++ = std::make_pair(last, last);
  }
  return out;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator split(InputIterator first, InputIterator last,
                     OutputIterator out, char delim, bool doStrip = false,
                     bool allowEmpty = false)
{
  for (InputIterator i = first; i != last;) {
    InputIterator j = std::find(i, last, delim);
    std::pair<InputIterator, InputIterator> p(i, j);
    if (doStrip) {
      p = stripIter(i, j);
    }
    if (allowEmpty || p.first != p.second) {
      *out++ = std::string(p.first, p.second);
    }
    i = j;
    if (j != last) {
      ++i;
    }
  }
  if (allowEmpty && (first == last || *(last - 1) == delim)) {
    *out++ = std::string(last, last);
  }
  return out;
}

template <typename InputIterator1, typename InputIterator2>
bool streq(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
           InputIterator2 last2)
{
  if (last1 - first1 != last2 - first2) {
    return false;
  }
  return std::equal(first1, last1, first2);
}

template <typename InputIterator>
bool streq(InputIterator first, InputIterator last, const char* b)
{
  for (; first != last && *b != '\0'; ++first, ++b) {
    if (*first != *b) {
      return false;
    }
  }
  return first == last && *b == '\0';
}

struct CaseCmp {
  bool operator()(char lhs, char rhs) const
  {
    if ('A' <= lhs && lhs <= 'Z') {
      lhs += 'a' - 'A';
    }
    if ('A' <= rhs && rhs <= 'Z') {
      rhs += 'a' - 'A';
    }
    return lhs == rhs;
  }
};

template <typename InputIterator1, typename InputIterator2>
InputIterator1 strifind(InputIterator1 first1, InputIterator1 last1,
                        InputIterator2 first2, InputIterator2 last2)
{
  return std::search(first1, last1, first2, last2, CaseCmp());
}

template <typename InputIterator1, typename InputIterator2>
bool strieq(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
            InputIterator2 last2)
{
  if (last1 - first1 != last2 - first2) {
    return false;
  }
  return std::equal(first1, last1, first2, CaseCmp());
}

template <typename InputIterator>
bool strieq(InputIterator first, InputIterator last, const char* b)
{
  CaseCmp cmp;
  for (; first != last && *b != '\0'; ++first, ++b) {
    if (!cmp(*first, *b)) {
      return false;
    }
  }
  return first == last && *b == '\0';
}

bool strieq(const std::string& a, const char* b);
bool strieq(const std::string& a, const std::string& b);

template <typename InputIterator1, typename InputIterator2>
bool startsWith(InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, InputIterator2 last2)
{
  if (last1 - first1 < last2 - first2) {
    return false;
  }
  return std::equal(first2, last2, first1);
}

template <typename InputIterator>
bool startsWith(InputIterator first, InputIterator last, const char* b)
{
  for (; first != last && *b != '\0'; ++first, ++b) {
    if (*first != *b) {
      return false;
    }
  }
  return *b == '\0';
}

bool startsWith(const std::string& a, const char* b);
bool startsWith(const std::string& a, const std::string& b);

template <typename InputIterator1, typename InputIterator2>
bool istartsWith(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2)
{
  if (last1 - first1 < last2 - first2) {
    return false;
  }
  return std::equal(first2, last2, first1, CaseCmp());
}

template <typename InputIterator>
bool istartsWith(InputIterator first, InputIterator last, const char* b)
{
  CaseCmp cmp;
  for (; first != last && *b != '\0'; ++first, ++b) {
    if (!cmp(*first, *b)) {
      return false;
    }
  }
  return *b == '\0';
}

bool istartsWith(const std::string& a, const char* b);
bool istartsWith(const std::string& a, const std::string& b);

template <typename InputIterator1, typename InputIterator2>
bool endsWith(InputIterator1 first1, InputIterator1 last1,
              InputIterator2 first2, InputIterator2 last2)
{
  if (last1 - first1 < last2 - first2) {
    return false;
  }
  return std::equal(first2, last2, last1 - (last2 - first2));
}

bool endsWith(const std::string& a, const char* b);
bool endsWith(const std::string& a, const std::string& b);

template <typename InputIterator1, typename InputIterator2>
bool iendsWith(InputIterator1 first1, InputIterator1 last1,
               InputIterator2 first2, InputIterator2 last2)
{
  if (last1 - first1 < last2 - first2) {
    return false;
  }
  return std::equal(first2, last2, last1 - (last2 - first2), CaseCmp());
}

bool iendsWith(const std::string& a, const char* b);
bool iendsWith(const std::string& a, const std::string& b);

// Returns true if strcmp(a, b) < 0
bool strless(const char* a, const char* b);

void generateRandomData(unsigned char* data, size_t length);

// Saves data to file whose name is filename. If overwrite is true,
// existing file is overwritten. Otherwise, this function doesn't do
// nothing.  If data is saved successfully, return true. Otherwise
// returns false.
bool saveAs(const std::string& filename, const std::string& data,
            bool overwrite = false);

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

// Returns true if ip1 and ip2 are in the same CIDR block.  ip1 and
// ip2 must be numeric IPv4 or IPv6 address. If either of them or both
// of them is not valid numeric address, then returns false. bits is
// prefix bits. If bits is out of range, then bits is set to the
// length of binary representation of the address*8.
bool inSameCidrBlock(const std::string& ip1, const std::string& ip2,
                     size_t bits);

// No throw
void executeHookByOptName(const std::shared_ptr<RequestGroup>& group,
                          const Option* option, PrefPtr pref);

// No throw
void executeHookByOptName(const RequestGroup* group, const Option* option,
                          PrefPtr pref);

std::string createSafePath(const std::string& dir, const std::string& filename);

std::string createSafePath(const std::string& filename);

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
template <typename Iterator>
std::pair<Iterator, bool> nextParam(std::string& name, std::string& value,
                                    Iterator first, Iterator last, char delim)
{
  Iterator end = last;
  while (first != end) {
    last = first;
    Iterator parmnameFirst = first;
    Iterator parmnameLast = first;
    bool eqFound = false;
    for (; last != end; ++last) {
      if (*last == delim) {
        break;
      }
      else if (!eqFound && *last == '=') {
        eqFound = true;
        parmnameFirst = first;
        parmnameLast = last;
      }
    }
    std::pair<std::string::const_iterator, std::string::const_iterator> namep =
        std::make_pair(last, last);
    std::pair<std::string::const_iterator, std::string::const_iterator> valuep =
        std::make_pair(last, last);
    if (parmnameFirst == parmnameLast) {
      if (!eqFound) {
        parmnameFirst = first;
        parmnameLast = last;
        namep = stripIter(parmnameFirst, parmnameLast);
      }
    }
    else {
      first = parmnameLast + 1;
      namep = stripIter(parmnameFirst, parmnameLast);
      valuep = stripIter(first, last);
    }
    if (last != end) {
      ++last;
    }
    if (namep.first != namep.second) {
      name.assign(namep.first, namep.second);
      value.assign(valuep.first, valuep.second);
      return std::make_pair(last, true);
    }
    first = last;
  }
  return std::make_pair(end, false);
}

template <typename T> std::shared_ptr<T> copy(const std::shared_ptr<T>& a)
{
  return std::make_shared<T>(*a.get());
}

// This is a bit different from cookie_helper::domainMatch().  If
// hostname is numeric host, then returns true if domain == hostname.
// That is if domain starts with ".", then returns true if domain is a
// suffix of hostname.  If domain does not start with ".", then
// returns true if domain == hostname.  Otherwise returns true.
// For example,
//
// * noProxyDomainMatch("aria2.sf.net", ".sf.net") returns true.
// * noProxyDomainMatch("sf.net", ".sf.net") returns false.
bool noProxyDomainMatch(const std::string& hostname, const std::string& domain);

// Checks hostname matches pattern as described in RFC 6125.
bool tlsHostnameMatch(const std::string& pattern, const std::string& hostname);

#ifdef ENABLE_SSL
TLSVersion toTLSVersion(const std::string& ver);
#endif // ENABLE_SSL

#ifdef __MINGW32__
// Formats error message for error code errNum, which is the return
// value of GetLastError().  On error, this function returns empty
// string.
std::string formatLastError(int errNum);
#endif // __MINGW32__

// Sets file descriptor file FD_CLOEXEC to |fd|.  This function is
// noop for Mingw32 build, since we disable inheritance in
// CreateProcess call.
void make_fd_cloexec(int fd);

#ifdef __MINGW32__
bool gainPrivilege(LPCTSTR privName);
#endif // __MINGW32__

} // namespace util

} // namespace aria2

#endif // D_UTIL_H
