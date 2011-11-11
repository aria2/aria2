/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "Sqlite3CookieParser.h"

#include <cstring>
#include <limits>

#include "DlAbortEx.h"
#include "util.h"
#include "fmt.h"
#include "A2STR.h"
#include "cookie_helper.h"
#ifndef HAVE_SQLITE3_OPEN_V2
# include "File.h"
#endif // !HAVE_SQLITE3_OPEN_V2

namespace aria2 {

Sqlite3CookieParser::Sqlite3CookieParser(const std::string& filename):db_(0)
{
  int ret;
#ifdef HAVE_SQLITE3_OPEN_V2
  ret = sqlite3_open_v2(filename.c_str(), &db_, SQLITE_OPEN_READONLY, 0);
#else // !HAVE_SQLITE3_OPEN_V2
  if(!File(filename).isFile()) {
    return;
  }
  ret = sqlite3_open(filename.c_str(), &db_);
#endif // !HAVE_SQLITE3_OPEN_V2
  if(SQLITE_OK != ret) {
    sqlite3_close(db_);
    db_ = 0;
  }
}

Sqlite3CookieParser::~Sqlite3CookieParser()
{
  sqlite3_close(db_);
}

namespace {
std::string toString(const char* str)
{
  if(str) {
    return str;
  } else {
    return A2STR::NIL;
  }
}
} // namespace

namespace {
bool parseTime(int64_t& time, const std::string& s)
{
  if(!util::parseLLIntNoThrow(time, s)) {
    return false;
  }
  if(std::numeric_limits<time_t>::max() < time) {
    time = std::numeric_limits<time_t>::max();
  } else if(std::numeric_limits<time_t>::min() > time) {
    time = std::numeric_limits<time_t>::min();
  }
  return true;
}
} // namespace

namespace {
int cookieRowMapper(void* data, int columns, char** values, char** names)
{
  if(columns != 7 || !values[0] || !values[1] || !values[4]) {
    return 0;
  }
  std::vector<Cookie>& cookies =
    *reinterpret_cast<std::vector<Cookie>*>(data);
  size_t val0len = strlen(values[0]);
  std::string cookieDomain
    (util::lstripIter(&values[0][0], &values[0][val0len], '.'),
     &values[0][val0len]);
  std::string cookieName(&values[4][0], &values[4][strlen(values[4])]);
  std::string cookiePath(&values[1][0], &values[1][strlen(values[1])]);
  if(cookieName.empty() || cookieDomain.empty() ||
     !cookie::goodPath(cookiePath.begin(), cookiePath.end())) {
    return 0;
  }
  int64_t expiryTime;
  if(!values[3] || !parseTime(expiryTime, values[3])) {
    return 0;
  }
  int64_t lastAccessTime;
  if(!values[6] || !parseTime(lastAccessTime, values[6])) {
    return 0;
  }
  Cookie c(cookieName,
           toString(values[5]), // value
           expiryTime,
           true, // persistent
           cookieDomain,
           util::isNumericHost(cookieDomain) ||
           (values[0] && values[0][0] != '.'), // hostOnly
           cookiePath,
           values[2] && strcmp(values[2], "1") == 0, //secure
           false,
           lastAccessTime // creation time. Set this later.
           );
  cookies.push_back(c);
  return 0;
}
} // namespace

void Sqlite3CookieParser::parse(std::vector<Cookie>& cookies)
{
  if(!db_) {
    throw DL_ABORT_EX(fmt("SQLite3 database is not opened."));
  }
  std::vector<Cookie> tcookies;
  char* sqlite3ErrMsg = 0;
  int ret = sqlite3_exec(db_, getQuery().c_str(), cookieRowMapper,
                         &tcookies, &sqlite3ErrMsg);
  std::string errMsg;
  if(sqlite3ErrMsg) {
    errMsg = sqlite3ErrMsg;
    sqlite3_free(sqlite3ErrMsg);
  }
  if(SQLITE_OK != ret) {
    throw DL_ABORT_EX
      (fmt("Failed to read SQLite3 database: %s", errMsg.c_str()));
  }
  cookies.swap(tcookies);
}

} // namespace aria2
