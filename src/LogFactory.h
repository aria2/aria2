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
#ifndef D_LOG_FACTORY_H
#define D_LOG_FACTORY_H

#include "common.h"

#include <string>

#include "Logger.h"
#include "SharedHandle.h"

namespace aria2 {

class LogFactory {
private:
  static std::string filename_;
  static SharedHandle<Logger> logger_;
  static bool consoleOutput_;
  static Logger::LEVEL logLevel_;

  static void openLogger(const SharedHandle<Logger>& logger);

  LogFactory();
public:
  /**
   * Get logger instance. Returned logger is singleton.
   * This function is not thread-safe.
   */
  static const SharedHandle<Logger>& getInstance();

  /**
   * Set a filename to write log. If name is "-", log is written to
   * stdout. If name is "", log is not written to file.
   */
  static void setLogFile(const std::string& name);

  /**
   * Set flag whether the log is printed in console.
   * If f is false, log is not printed in console.
   */
  static void setConsoleOutput(bool f)
  {
    consoleOutput_ = f;
  }

  /**
   * Set log level to output.
   */
  static void setLogLevel(Logger::LEVEL level);

  /**
   * Set log level to output by string represention of log level.
   * Possible values are: debug, info, notice, warn, error
   */
  static void setLogLevel(const std::string& level);

  /**
   * Releases used resources
   */
  static void release();

  static void reconfigure();
};

#define A2_LOG_DEBUG_ENABLED                                            \
  aria2::LogFactory::getInstance()->levelEnabled(Logger::A2_DEBUG)

#define A2_LOG(level, msg)                                              \
  {                                                                     \
    const aria2::SharedHandle<aria2::Logger>& logger =                  \
      aria2::LogFactory::getInstance();                                 \
    if(logger->levelEnabled(level))                                     \
      logger->log(level, __FILE__, __LINE__, msg);                      \
  }
#define A2_LOG_EX(level, msg, ex)                                       \
  {                                                                     \
    const aria2::SharedHandle<aria2::Logger>& logger =                  \
      aria2::LogFactory::getInstance();                                 \
    if(logger->levelEnabled(level))                                     \
      logger->log(level, __FILE__, __LINE__, msg, ex);                  \
  }

#define A2_LOG_DEBUG(msg) A2_LOG(Logger::A2_DEBUG, msg)
#define A2_LOG_DEBUG_EX(msg, ex) A2_LOG_EX(Logger::A2_DEBUG, msg, ex)

#define A2_LOG_INFO(msg) A2_LOG(Logger::A2_INFO, msg)
#define A2_LOG_INFO_EX(msg, ex) A2_LOG_EX(Logger::A2_INFO, msg, ex)

#define A2_LOG_NOTICE(msg) A2_LOG(Logger::A2_NOTICE, msg)
#define A2_LOG_NOTICE_EX(msg, ex) A2_LOG_EX(Logger::A2_NOTICE, msg, ex)

#define A2_LOG_WARN(msg) A2_LOG(Logger::A2_WARN, msg)
#define A2_LOG_WARN_EX(msg, ex) A2_LOG_EX(Logger::A2_WARN, msg, ex)

#define A2_LOG_ERROR(msg) A2_LOG(Logger::A2_ERROR, msg)
#define A2_LOG_ERROR_EX(msg, ex) A2_LOG_EX(Logger::A2_ERROR, msg, ex)

} // namespace aria2

#endif // D_LOG_FACTORY_H
