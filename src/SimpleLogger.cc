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
#include "Util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "a2io.h"
#include "a2time.h"
#include "StringFormat.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <cstdlib>

namespace aria2 {

#if !defined(va_copy)
# if defined(__va_copy)
#  define va_copy(dest, src) __va_copy(dest, src)
# else
#  define va_copy(dest, src) (dest = src)
# endif
#endif

#define WRITE_LOG(LEVEL, MSG) \
va_list ap;\
va_start(ap, MSG);\
writeFile(Logger::LEVEL, MSG, ap);\
va_end(ap);

#define WRITE_LOG_EX(LEVEL, MSG, EX) \
va_list ap;\
va_start(ap, EX);\
writeFile(Logger::LEVEL, MSG, ap, EX);\
va_end(ap);

SimpleLogger::SimpleLogger():stdoutField(0) {}

SimpleLogger::~SimpleLogger() {
  closeFile();
}

void SimpleLogger::openFile(const std::string& filename) {
  file.open(filename.c_str(), std::ios::app|std::ios::binary);
  if(!file) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }
}

void SimpleLogger::closeFile() {
  if(file.is_open()) {
    file.close();
  }
}

void SimpleLogger::setStdout(Logger::LEVEL level, bool enabled) {
  if(enabled) {
    stdoutField |= level;
  } else {
    stdoutField &= ~level;
  }
}

void SimpleLogger::writeHeader(std::ostream& o, const std::string& date,
			       const std::string& level)
{
  o << StringFormat("%s %s - ", date.c_str(), level.c_str());
}

void SimpleLogger::writeLog(std::ostream& o, Logger::LEVEL level,
			    const char* msg, va_list ap, Exception* e,
			    bool printHeader)
{
  va_list apCopy;
  va_copy(apCopy, ap);
  std::string levelStr;
  switch(level) {
  case Logger::DEBUG:
    levelStr = "DEBUG";
    break;
  case Logger::NOTICE:
    levelStr = "NOTICE";
    break;
  case Logger::WARN:
    levelStr = "WARN";
    break;
  case Logger::ERROR:
    levelStr = "ERROR";
    break;
  case Logger::INFO:
  default:
    levelStr = "INFO";
  }
  time_t now = time(NULL);
  char datestr[20];
  struct tm tm;
  localtime_r(&now, &tm);
  strftime(datestr, sizeof(datestr), "%Y-%m-%d %H:%M:%S", &tm);

  // TODO a quick hack not to print header in console
  if(printHeader) {
    writeHeader(o, datestr, levelStr);
  }
  {
    char* res;
    if(vasprintf(&res, std::string(Util::replace(msg, "\r", "")+"\n").c_str(), apCopy) == -1) {
      o << "SimpleLogger error, cannot allocate memory.\n";
    } else {
      o << res;
      free(res);
    }
  }
  for(Exception* nestedEx = e; nestedEx; nestedEx = nestedEx->getCause()) {
    // TODO a quick hack not to print header in console
    if(printHeader) {
      writeHeader(o, datestr, levelStr);
    }
    o << StringFormat("exception: %s\n", Util::replace(nestedEx->getMsg(), "\r", "").c_str());
  }
  o << std::flush;
  va_end(apCopy);
}

void SimpleLogger::writeFile(Logger::LEVEL level, const char* msg, va_list ap, Exception* e)
{
  writeLog(file, level, msg, ap, e);
  if(stdoutField&level) {
    std::cout << "\n";
    writeLog(std::cout, level, msg, ap, e);
  }
}

void SimpleLogger::debug(const char* msg, ...) {
  WRITE_LOG(DEBUG, msg);
}

void SimpleLogger::debug(const char* msg, Exception* e, ...) {
  WRITE_LOG_EX(DEBUG, msg, e);
}

void SimpleLogger::info(const char* msg, ...) {
  WRITE_LOG(INFO, msg);
}

void SimpleLogger::info(const char* msg, Exception* e, ...) {
  WRITE_LOG_EX(INFO, msg, e);
}

void SimpleLogger::notice(const char* msg, ...) {
  WRITE_LOG(NOTICE, msg);
}

void SimpleLogger::notice(const char* msg, Exception* e, ...) {
  WRITE_LOG_EX(INFO, msg, e);
}

void SimpleLogger::warn(const char* msg, ...) {
  WRITE_LOG(WARN, msg);
}

void SimpleLogger::warn(const char* msg, Exception* e, ...) {
  WRITE_LOG_EX(WARN, msg, e);
}

void SimpleLogger::error(const char* msg, ...) {
  WRITE_LOG(ERROR, msg);
}

void SimpleLogger::error(const char* msg, Exception* e, ...) {
  WRITE_LOG_EX(ERROR, msg, e);
}

} // namespace aria2
