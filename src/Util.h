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
#include <string>
#include <utility>
#include <deque>
#include <sys/time.h>

using namespace std;

#define STRTOLL(X) strtoll(X, (char**)NULL, 10);

class Util {
public:
  static void split(pair<string, string>& hp, string src, char delim);
  static string llitos(long long int value, bool comma = false);
  static string itos(int value, bool comma = false);
  /**
   * Computes difference in micro-seconds between tv1 and tv2,
   * assuming tv1 is newer than tv2.
   * If tv1 is older than tv2, then this method returns 0.
   */
  static long long int difftv(struct timeval tv1, struct timeval tv2);
  /**
   * Take a string src which is a deliminated list and add its elements
   * into result. result is not cleared before conversion begins.
   */
  static void slice(Strings& result, string src, char delim);
  
  static string trim(string src);

  static bool startsWith(string target, string part);

  static bool endsWith(string target, string part);

  static string replace(string target, string oldstr, string newstr);

  static string urlencode(const unsigned char* target, int len);

  static string toHex(const unsigned char* src, int len);

  static FILE* openFile(string filename, string mode);

  static void rangedFileCopy(string destFile, string src, long long int srcOffset, long long int length);

};

#endif // _D_UTIL_H_
