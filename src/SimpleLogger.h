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
#ifndef _D_SIMPLE_LOGGER_H_
#define _D_SIMPLE_LOGGER_H_

#include "Logger.h"
#include <cstdarg>
#include <string>
#include <fstream>

namespace aria2 {

class SimpleLogger:public Logger {
private:
  void writeFile(Logger::LEVEL level, const char* msg, va_list ap);

  void writeStackTrace(Logger::LEVEL level, const Exception& e);

  void flush();

  void writeHeader(std::ostream& out,
		   const std::string& date, const std::string& level);

  void writeLog(std::ostream& out, Logger::LEVEL level,
		const char* msg, va_list ap,
		bool printHeader = true);

  std::ofstream file;
  int stdoutField;

  Logger::LEVEL _logLevel;

  static const std::string DEBUG;

  static const std::string NOTICE;

  static const std::string WARN;

  static const std::string ERROR;

  static const std::string INFO;
public:
  SimpleLogger();
  ~SimpleLogger();

  void openFile(const std::string& filename);
  void closeFile();
  virtual void debug(const char* msg, ...);
  virtual void debug(const char* msg, const Exception& ex, ...);
  virtual void info(const char* msg, ...);
  virtual void info(const char* msg, const Exception& ex, ...);
  virtual void notice(const char* msg, ...);
  virtual void notice(const char* msg, const Exception& ex, ...);
  virtual void warn(const char* msg, ...);
  virtual void warn(const char* msg, const Exception& ex, ...);
  virtual void error(const char* msg, ...);
  virtual void error(const char* msg, const Exception& ex, ...);

  virtual void setLogLevel(Logger::LEVEL level);

  void setStdout(Logger::LEVEL level, bool enabled);
};

} // namespace aria2

#endif // _D_SIMPLE_LOGGER_H_

