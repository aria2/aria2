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
#ifndef _D_META_FILE_UTIL_H_
#define _D_META_FILE_UTIL_H_

#include "MetaEntry.h"
#include "Dictionary.h"
#include "List.h"
#include "Data.h"
#include "common.h"
#include <string>

using namespace std;

class MetaFileUtil {
private:
  MetaFileUtil() {}

  static MetaEntry* bdecodingR(const char** pp, const char* end);
  static Dictionary* parseDictionaryTree(const char** pp, const char* end);
  static List* parseListTree(const char** pp, const char* end);
  static Data* decodeWord(const char** pp, const char* end);
  static Data* decodeInt(const char** pp, const char* end);
  static string decodeWordAsString(const char** pp, const char* end);

public:
  static MetaEntry* parseMetaFile(const string& file);
  static MetaEntry* bdecoding(const char* buf, int len);
};

#endif // _D_META_FILE_UTIL_H_
