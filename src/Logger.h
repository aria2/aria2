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
#ifndef D_LOGGER_H
#define D_LOGGER_H

#include "common.h"

#include <cstdarg>
#include <string>
#include <fstream>
#include <iostream>

namespace aria2 {

class Exception;
class LogFormatter;

class Logger {
public:
  enum LEVEL {
    A2_DEBUG  = 1 << 0,
    A2_INFO   = 1 << 1,
    A2_NOTICE = 1 << 2,
    A2_WARN   = 1 << 3,
    A2_ERROR  = 1 << 4,
  };

  static const std::string DEBUG_LABEL;

  static const std::string NOTICE_LABEL;

  static const std::string WARN_LABEL;

  static const std::string ERROR_LABEL;

  static const std::string INFO_LABEL;
private:
  LogFormatter* logFormatter_;

  LEVEL logLevel_;

  std::ofstream file_;

  int stdoutField_;
  
  bool levelEnabled(LEVEL level)
  {
    return (level >= logLevel_ && file_.is_open()) || stdoutField_&level;
  }

  void writeLog
  (std::ostream& o, LEVEL logLevel, const std::string& logLevelLabel,
   const char* msg, va_list ap);

  void writeStackTrace
  (std::ostream& o, LEVEL logLevel, const std::string& logLevelLabel,
   const Exception& ex);
public:
  Logger();

  virtual ~Logger();

  void debug(const char* msg, ...)
    __attribute__ ((format (printf, 2, 3)));

  void debug(const char* msg, const Exception& ex, ...)
    __attribute__ ((format (printf, 2, 4)));

  void info(const char* msg, ...)
    __attribute__ ((format (printf, 2, 3)));

  void info(const char* msg, const Exception& ex, ...)
    __attribute__ ((format (printf, 2, 4)));

  void notice(const char* msg, ...)
    __attribute__ ((format (printf, 2, 3)));

  void notice(const char* msg, const Exception& ex, ...)
    __attribute__ ((format (printf, 2, 4)));

  void warn(const char* msg, ...)
    __attribute__ ((format (printf, 2, 3)));

  void warn(const char* msg, const Exception& ex, ...)
    __attribute__ ((format (printf, 2, 4)));

  void error(const char*  msg, ...)
    __attribute__ ((format (printf, 2, 3)));

  void error(const char* msg, const Exception& ex, ...)
    __attribute__ ((format (printf, 2, 4)));

  void openFile(const std::string& filename);

  void closeFile();

  void setLogFormatter(LogFormatter* logFormatter);

  void setLogLevel(LEVEL level)
  {
    logLevel_ = level;
  }

  void setStdoutLogLevel(Logger::LEVEL level, bool enabled);

  // Returns true if this logger actually writes debug log message to
  // either file or stdout.
  bool debug()
  {
    return levelEnabled(A2_DEBUG);
  }

  bool info()
  {
    return levelEnabled(A2_INFO);
  }
};

} // namespace aria2

#endif // D_LOGGER_H
