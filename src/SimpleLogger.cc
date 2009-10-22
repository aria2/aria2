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
#include "SimpleLogger.h"

#include <cassert>

#include "Util.h"
#include "a2time.h"
#include "A2STR.h"
#include "StringFormat.h"
#include "Exception.h"

namespace aria2 {

static void writeHeader
(std::ostream& o, const std::string& date, const std::string& logLevelLabel)
{
  o << StringFormat("%s %s - ", date.c_str(), logLevelLabel.c_str());
}

void SimpleLogger::writeLog
(std::ostream& o, Logger::LEVEL level, const std::string& logLevelLabel,
 const char* msg, va_list ap)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  char datestr[27]; // 'YYYY-MM-DD hh:mm:ss.uuuuuu'+'\0' = 27 bytes
  struct tm tm;
  //tv.tv_sec may not be of type time_t.
  time_t timesec = tv.tv_sec;
  localtime_r(&timesec, &tm);
  size_t dateLength =
    strftime(datestr, sizeof(datestr), "%Y-%m-%d %H:%M:%S", &tm);
  assert(dateLength <= (size_t)20);
  snprintf(datestr+dateLength, sizeof(datestr)-dateLength,
	   ".%06ld", tv.tv_usec);
  writeHeader(o, datestr, logLevelLabel);
  {
    char buf[1024];
    std::string body = util::replace(msg, A2STR::CR_C, A2STR::NIL);
    body += A2STR::LF_C;
    if(vsnprintf(buf, sizeof(buf), body.c_str(), ap) < 0) {
      o << "SimpleLogger error, failed to format message.\n";
    } else {
      o << buf;
    }
  }
}

void SimpleLogger::writeStackTrace
(std::ostream& o, Logger::LEVEL level, const std::string& logLevelLabel,
 const Exception& e)
{
  o << e.stackTrace();
}

} // namespace aria2
