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
  LEVEL _logLevel;

  std::ofstream _file;

  int _stdoutField;
  
  bool levelEnabled(LEVEL level)
  {
    return (level >= _logLevel && _file.is_open()) || _stdoutField&level;
  }
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

  void debug(const char* msg, ...);

  void debug(const char* msg, const Exception& ex, ...);

  void info(const char* msg, ...);

  void info(const char* msg, const Exception& ex, ...);

  void notice(const char* msg, ...);

  void notice(const char* msg, const Exception& ex, ...);

  void warn(const char* msg, ...);

  void warn(const char* msg, const Exception& ex, ...);

  void error(const char*  msg, ...);

  void error(const char* msg, const Exception& ex, ...);

  void openFile(const std::string& filename);

  void closeFile();

  void setLogLevel(LEVEL level)
  {
    _logLevel = level;
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

#endif // _D_LOGGER_H_
