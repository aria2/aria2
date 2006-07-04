/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_UTIL_H_
#define _D_UTIL_H_

#include "common.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <string>
#include <utility>
#include <deque>
#include <sys/time.h>

using namespace std;

#define STRTOLL(X) strtoll(X, (char**)NULL, 10);

class Util {
public:
  static void split(pair<string, string>& hp, const string& src, char delim);
  static string llitos(long long int value, bool comma = false);
  static string itos(int value, bool comma = false);
  /**
   * Computes difference in micro-seconds between tv1 and tv2,
   * assuming tv1 is newer than tv2.
   * If tv1 is older than tv2, then this method returns 0.
   */
  static long long int difftv(struct timeval tv1, struct timeval tv2);
  static int difftvsec(struct timeval tv1, struct timeval tv2);
  /**
   * Take a string src which is a deliminated list and add its elements
   * into result. result is not cleared before conversion begins.
   */
  static void slice(Strings& result, const string& src, char delim);
  
  static string trim(const string& src);

  static bool startsWith(const string& target, const string& part);

  static bool endsWith(const string& target, const string& part);

  static string replace(const string& target, const string& oldstr, const string& newstr);

  static string urlencode(const unsigned char* target, int len);

  static string toHex(const unsigned char* src, int len);

  static FILE* openFile(const string& filename, const string& mode);

  static void fileCopy(const string& destFile, const string& src);

  static void rangedFileCopy(const string& destFile, const string& src, long long int srcOffset, long long int length);

  static bool isPowerOf(int num, int base);

  static string secfmt(int sec);

  static int expandBuffer(char** pbuf, int curLength, int newLength);

  static void unfoldRange(const string& src, Integers& range);

  // this function temporarily put here
  static string getContentDispositionFilename(const string& header);

  // digest must be at least 20 bytes long.
#ifdef ENABLE_MESSAGE_DIGEST
  static void sha1Sum(unsigned char* digest, const void* data, int dataLength);
#endif // ENABLE_MESSAGE_DIGEST

  // Before call this method, allocate enough memory to the parameter "digest".
  // For sha1, you need 20 bytes. For md5, 16 bytes.
#ifdef ENABLE_MESSAGE_DIGEST
  static void fileChecksum(const string& filename, unsigned char* digest,
			   MessageDigestContext::HashAlgo algo);
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_BITTORRENT
  static Integers computeFastSet(string ipaddr, const unsigned char* infoHash,
				int pieces, int fastSetSize);
#endif // ENABLE_BITTORRENT

  static int countBit(unsigned int);

  static string randomAlpha(int length);

  static string toUpper(const string& src);

  static string toLower(const string& src);
};

#endif // _D_UTIL_H_
