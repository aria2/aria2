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
#include "Sqlite3MozCookieParser.h"
#include "RecoverableException.h"
#include "Util.h"
#include "StringFormat.h"
#include "A2STR.h"
#include <cstring>
#include <sqlite3.h>

namespace aria2 {

Sqlite3MozCookieParser::Sqlite3MozCookieParser() {}

Sqlite3MozCookieParser::~Sqlite3MozCookieParser() {}

static std::string toString(const char* str)
{
  if(str) {
    return str;
  } else {
    return A2STR::NIL;
  }
}

static int cookieRowMapper(void* data, int rowIndex,
			   char** values, char** names)
{
  try {
    int64_t expireDate = Util::parseLLInt(toString(values[3]));
    // TODO assuming time_t is int32_t...
    if(expireDate > INT32_MAX) {
      expireDate = INT32_MAX;
    }

    Cookie c(toString(values[4]), // name
	     toString(values[5]), // value
	     expireDate, // expires
	     toString(values[1]), // path
	     toString(values[0]), // domain
	     strcmp(toString(values[2]).c_str(), "1") == 0 ? true:false //secure
	     );
		      
    if(c.good()) {
      ((std::deque<Cookie>*)data)->push_back(c);
    }
  } catch(RecoverableException& e) {
    //failed to parse expiry.
  }
  return 0;
}

std::deque<Cookie> Sqlite3MozCookieParser::parse(const std::string& filename)
{
  sqlite3* db = 0;
  
  int ret;
  ret = sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READONLY, 0);
  if(SQLITE_OK != ret) {
    std::string errMsg = sqlite3_errmsg(db);
    sqlite3_close(db);
    throw RecoverableException
      (StringFormat("Failed to open SQLite3 database: %s",
		    errMsg.c_str()).str());
  }
  std::deque<Cookie> cookies;
  char* sqlite3ErrMsg = 0;
  static const char* QUERY =
    "SELECT host, path, isSecure, expiry, name, value FROM moz_cookies";
  ret = sqlite3_exec(db, QUERY, cookieRowMapper, &cookies, &sqlite3ErrMsg);
  std::string errMsg;
  if(sqlite3ErrMsg) {
    errMsg = sqlite3ErrMsg;
    sqlite3_free(sqlite3ErrMsg);
  }
  if(SQLITE_OK != ret) {
    sqlite3_close(db);
    throw RecoverableException
      (StringFormat("Failed to read SQLite3 database: %s",
		    errMsg.c_str()).str());
  }
  sqlite3_close(db);
  return cookies;
}

} // namespace aria2
