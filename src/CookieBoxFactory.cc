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
#include "CookieBoxFactory.h"
#include "CookieParser.h"
#include "CookieBox.h"
#include "Util.h"
#include "RecoverableException.h"
#include "LogFactory.h"
#include "Logger.h"
#include "NsCookieParser.h"
#ifdef HAVE_SQLITE3
# include "Sqlite3MozCookieParser.h"
#endif // HAVE_SQLITE3
#include <fstream>
#include <iomanip>

namespace aria2 {

CookieBoxFactory::CookieBoxFactory():_logger(LogFactory::getInstance()) {}

CookieBoxFactory::~CookieBoxFactory() {}

CookieBoxHandle CookieBoxFactory::createNewInstance()
{
  CookieBoxHandle box(new CookieBox());
  box->add(defaultCookies);
  return box;
}

void CookieBoxFactory::loadDefaultCookie(const std::string& filename)
{
  char header[16]; // "SQLite format 3" plus \0
  {
    std::ifstream s(filename.c_str());
    s.get(header, sizeof(header));
    if(s.bad()) {
      _logger->error("Failed to read header of cookie file %s",
		     filename.c_str());
      return;
    }
  }
  try {
    if(std::string(header) == "SQLite format 3") {
#ifdef HAVE_SQLITE3
      defaultCookies = Sqlite3MozCookieParser().parse(filename);
#else // !HAVE_SQLITE3
    _logger->notice("Cannot read SQLite3 database because SQLite3 support is"
		    " disabled by configuration.");
#endif // !HAVE_SQLITE3
    } else {
      defaultCookies = NsCookieParser().parse(filename);
    }
  } catch(RecoverableException& e) {
    _logger->error("Failed to load cookies from %s, cause: %s",
		   filename.c_str(), e.what());
  }
}

} // namespace aria2
