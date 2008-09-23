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
#include "A2STR.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cassert>

namespace aria2 {

#if !defined(va_copy)
# if defined(__va_copy)
#  define va_copy(dest, src) __va_copy(dest, src)
# else
#  define va_copy(dest, src) (dest = src)
# endif
#endif

#define WRITE_LOG(LEVEL, MSG)						\
  if(LEVEL >= _logLevel && (stdoutField&LEVEL || file.is_open())) {	\
    va_list ap;								\
    va_start(ap, MSG);							\
    writeFile(LEVEL, MSG, ap);						\
    flush();								\
    va_end(ap);								\
  }

#define WRITE_LOG_EX(LEVEL, MSG, EX)					\
  if(LEVEL >= _logLevel && (stdoutField&LEVEL || file.is_open())) {	\
    va_list ap;								\
    va_start(ap, EX);							\
    writeFile(LEVEL, MSG, ap);						\
    writeStackTrace(LEVEL, EX);						\
    flush();								\
    va_end(ap);								\
}

const std::string SimpleLogger::DEBUG("DEBUG");

const std::string SimpleLogger::NOTICE("NOTICE");

const std::string SimpleLogger::WARN("WARN");

const std::string SimpleLogger::ERROR("ERROR");

const std::string SimpleLogger::INFO("INFO");

SimpleLogger::SimpleLogger():stdoutField(0), _logLevel(Logger::DEBUG) {}

SimpleLogger::~SimpleLogger() {
  closeFile();
}

void SimpleLogger::openFile(const std::string& filename) {
  file.open(filename.c_str(), std::ios::app|std::ios::binary);
  if(!file) {
    throw DlAbortEx
      (StringFormat(EX_FILE_OPEN, filename.c_str(), strerror(errno)).str());
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
			    const char* msg, va_list ap,
			    bool printHeader)
{
  va_list apCopy;
  va_copy(apCopy, ap);
  std::string levelStr;
  switch(level) {
  case Logger::DEBUG:
    levelStr = DEBUG;
    break;
  case Logger::NOTICE:
    levelStr = NOTICE;
    break;
  case Logger::WARN:
    levelStr = WARN;
    break;
  case Logger::ERROR:
    levelStr = ERROR;
    break;
  case Logger::INFO:
  default:
    levelStr = INFO;
  }
  struct timeval tv;
  gettimeofday(&tv, 0);
  char datestr[27]; // 'YYYY-MM-DD hh:mm:ss.uuuuuu'+'\0' = 27 bytes
  struct tm tm;
  localtime_r(&tv.tv_sec, &tm);
  size_t dateLength =
    strftime(datestr, sizeof(datestr), "%Y-%m-%d %H:%M:%S", &tm);
  assert(dateLength <= (size_t)20);
  snprintf(datestr+dateLength, sizeof(datestr)-dateLength,
	   ".%06ld", tv.tv_usec);

  // TODO a quick hack not to print header in console
  if(printHeader) {
    writeHeader(o, datestr, levelStr);
  }
  {
    char buf[1024];
    if(vsnprintf(buf, sizeof(buf), std::string(Util::replace(msg, A2STR::CR_C, A2STR::NIL)+A2STR::LF_C).c_str(), apCopy) < 0) {
      o << "SimpleLogger error, failed to format message.\n";
    } else {
      o << buf;
    }
  }
  va_end(apCopy);
}

void SimpleLogger::writeFile(Logger::LEVEL level, const char* msg, va_list ap)
{
  writeLog(file, level, msg, ap);
  if(stdoutField&level) {
    std::cout << "\n";
    writeLog(std::cout, level, msg, ap);
  }
}

void SimpleLogger::writeStackTrace(Logger::LEVEL level, const Exception& e)
{
  file << e.stackTrace();
  if(stdoutField&level) {
    std::cout << e.stackTrace();
  }
}

void SimpleLogger::flush()
{
  file << std::flush;
  std::cout << std::flush;
}

void SimpleLogger::debug(const char* msg, ...) {
  WRITE_LOG(Logger::DEBUG, msg);
}

void SimpleLogger::debug(const char* msg, const Exception& e, ...) {
  WRITE_LOG_EX(Logger::DEBUG, msg, e);
}

void SimpleLogger::info(const char* msg, ...) {
  WRITE_LOG(Logger::INFO, msg);
}

void SimpleLogger::info(const char* msg, const Exception& e, ...) {
  WRITE_LOG_EX(Logger::INFO, msg, e);
}

void SimpleLogger::notice(const char* msg, ...) {
  WRITE_LOG(Logger::NOTICE, msg);
}

void SimpleLogger::notice(const char* msg, const Exception& e, ...) {
  WRITE_LOG_EX(Logger::INFO, msg, e);
}

void SimpleLogger::warn(const char* msg, ...) {
  WRITE_LOG(Logger::WARN, msg);
}

void SimpleLogger::warn(const char* msg, const Exception& e, ...) {
  WRITE_LOG_EX(Logger::WARN, msg, e);
}

void SimpleLogger::error(const char* msg, ...) {
  WRITE_LOG(Logger::ERROR, msg);
}

void SimpleLogger::error(const char* msg, const Exception& e, ...) {
  WRITE_LOG_EX(Logger::ERROR, msg, e);
}

void SimpleLogger::setLogLevel(Logger::LEVEL level)
{
  _logLevel = level;
}

} // namespace aria2
