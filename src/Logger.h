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
#ifndef _D_LOGGER_H_
#define _D_LOGGER_H_

#include "common.h"

#include <cstdarg>
#include <string>
#include <fstream>
#include <iostream>

namespace aria2 {

class Exception;

class Logger {
public:
  enum LEVEL {
    DEBUG  = 1 << 0,
    INFO   = 1 << 1,
    NOTICE = 1 << 2,
    WARN   = 1 << 3,
    ERROR  = 1 << 4,
  };

  static const std::string DEBUG_LABEL;

  static const std::string NOTICE_LABEL;

  static const std::string WARN_LABEL;

  static const std::string ERROR_LABEL;

  static const std::string INFO_LABEL;
private:
  LEVEL _logLevel;

  std::ofstream _file;

  int _stdoutField;
protected:
  virtual void writeLog
  (std::ostream& o, LEVEL logLevel, const std::string& logLevelLabel,
   const char* msg, va_list ap) = 0;

  virtual void writeStackTrace
  (std::ostream& o, LEVEL logLevel, const std::string& logLevelLabel,
   const Exception& ex) = 0;
public:
  Logger();

  virtual ~Logger();

#define WRITE_LOG(LEVEL, LEVEL_LABEL, MSG)              \
  if(LEVEL >= _logLevel && _file.is_open()) {           \
    va_list ap;                                         \
    va_start(ap, MSG);                                  \
    writeLog(_file, LEVEL, LEVEL_LABEL, MSG, ap);       \
    va_end(ap);                                         \
    _file << std::flush;                                \
  }                                                     \
  if(_stdoutField&LEVEL) {                              \
    std::cout << "\n";                                  \
    va_list ap;                                         \
    va_start(ap, MSG);                                  \
    writeLog(std::cout, LEVEL, LEVEL_LABEL, MSG, ap);   \
    va_end(ap);                                         \
    std::cout << std::flush;                            \
  }                                                     \

#define WRITE_LOG_EX(LEVEL, LEVEL_LABEL, MSG, EX)       \
  if(LEVEL >= _logLevel && _file.is_open()) {           \
    va_list ap;                                         \
    va_start(ap, EX);                                   \
    writeLog(_file, LEVEL, LEVEL_LABEL, MSG, ap);       \
    va_end(ap);                                         \
    writeStackTrace(_file, LEVEL, LEVEL_LABEL, EX);     \
    _file << std::flush;                                \
  }                                                     \
  if(_stdoutField&LEVEL) {                              \
    std::cout << "\n";                                  \
    va_list ap;                                         \
    va_start(ap, EX);                                   \
    writeLog(std::cout, LEVEL, LEVEL_LABEL, MSG, ap);   \
    va_end(ap);                                         \
    writeStackTrace(std::cout, LEVEL, LEVEL_LABEL, EX); \
    std::cout << std::flush;                            \
  }                                                     \

  void debug(const char* msg, ...)
  {
    WRITE_LOG(DEBUG, DEBUG_LABEL, msg);
  }

  void debug(const char* msg, const Exception& ex, ...)
  {
    WRITE_LOG_EX(DEBUG, DEBUG_LABEL, msg, ex);
  }

  void info(const char* msg, ...)
  {
    WRITE_LOG(INFO, INFO_LABEL, msg);
  }

  void info(const char* msg, const Exception& ex, ...)
  {
    WRITE_LOG_EX(INFO, INFO_LABEL, msg, ex);
  }

  void notice(const char* msg, ...)
  {
    WRITE_LOG(NOTICE, NOTICE_LABEL, msg);
  }

  void notice(const char* msg, const Exception& ex, ...)
  {
    WRITE_LOG_EX(NOTICE, NOTICE_LABEL, msg, ex);
  }

  void warn(const char* msg, ...)
  {
    WRITE_LOG(WARN, WARN_LABEL, msg);
  }

  void warn(const char* msg, const Exception& ex, ...)
  {
    WRITE_LOG_EX(WARN, WARN_LABEL, msg, ex);
  }

  void error(const char*  msg, ...)
  {
    WRITE_LOG(ERROR, ERROR_LABEL, msg);
  }

  void error(const char* msg, const Exception& ex, ...)
  {
    WRITE_LOG_EX(ERROR, ERROR_LABEL, msg, ex);
  }

  void openFile(const std::string& filename);

  void closeFile();

  void setLogLevel(LEVEL level)
  {
    _logLevel = level;
  }

  void setStdoutLogLevel(Logger::LEVEL level, bool enabled)
  {
    if(enabled) {
      _stdoutField |= level;
    } else {
      _stdoutField &= ~level;
    }
  }
};

} // namespace aria2

#endif // _D_LOGGER_H_
