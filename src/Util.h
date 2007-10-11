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
#include "DlAbortEx.h"
#include "a2time.h"
#include "FileEntry.h"
#include <utility>
#include <deque>
#include <ostream>

#define STRTOLL(X) strtoll(X, (char**)NULL, 10);

class Util {
public:
  static void split(pair<string, string>& hp, const string& src, char delim);
  static pair<string, string> split(const string& src, const string& delims);
  static string llitos(int64_t value, bool comma = false);
  static string ullitos(uint64_t value, bool comma = false);
  static string itos(int32_t value, bool comma = false);
  static string uitos(uint32_t value, bool comma = false);
  static string itos(int16_t value, bool comma = false);
  static string uitos(uint16_t value, bool comma = false);

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
  static void slice(Strings& result, const string& src, char delim, bool trim = false);
  
  static string trim(const string& src, const string& trimCharset = "\r\n\t ");

  static bool startsWith(const string& target, const string& part);

  static bool endsWith(const string& target, const string& part);

  static string replace(const string& target, const string& oldstr, const string& newstr);

  static string urlencode(const unsigned char* target, int32_t len);

  static string urlencode(const string& target)
  {
    return urlencode((const unsigned char*)target.c_str(), target.size());
  }

  static string urldecode(const string& target);

  static string torrentUrlencode(const unsigned char* target, int32_t len);

  static string toHex(const unsigned char* src, int32_t len);

  static FILE* openFile(const string& filename, const string& mode);

  static void fileCopy(const string& destFile, const string& src);

  static void rangedFileCopy(const string& destFile, const string& src, int64_t srcOffset, int64_t length) throw(DlAbortEx*);

  static bool isPowerOf(int32_t num, int32_t base);

  static string secfmt(int32_t sec);

  static int32_t expandBuffer(char** pbuf, int32_t curLength, int32_t newLength);

  static void unfoldRange(const string& src, Integers& range);

  // this function temporarily put here
  static string getContentDispositionFilename(const string& header);

  static int32_t countBit(uint32_t n);
  
  static string randomAlpha(int32_t length);
  
  static string toUpper(const string& src);

  static string toLower(const string& src);

  static bool isNumbersAndDotsNotation(const string& name);

  static void setGlobalSignalHandler(int32_t signal, void (*handler)(int), int32_t flags);

  static void indexRange(int32_t& startIndex, int32_t& endIndex,
			 int64_t offset,
			 int32_t srcLength, int32_t destLength);

  static string getHomeDir();

  static int64_t getRealSize(const string& sizeWithUnit);

  static string abbrevSize(int64_t size);

  static time_t httpGMT(const string& httpTimeFormat);

  static void toStream(ostream& os, const FileEntries& entries);

  static void sleep(long seconds);

  static void usleep(long microseconds);

  static bool isNumber(const string& what);

  static bool isLowercase(const string& what);

  static bool isUppercase(const string& what);

  static int32_t alphaToNum(const string& alphabets);
};

#endif // _D_UTIL_H_
