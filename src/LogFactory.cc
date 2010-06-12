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
#include "LogFactory.h"
#include "SimpleLogger.h"
#include "a2io.h"
#include "prefs.h"

namespace aria2 {

std::string LogFactory::_filename = DEV_NULL;
Logger* LogFactory::_logger = 0;
bool LogFactory::_consoleOutput = true;
Logger::LEVEL LogFactory::_logLevel = Logger::A2_DEBUG;

Logger* LogFactory::getInstance() {
  if(!_logger) {
    SimpleLogger* slogger = new SimpleLogger();
    if(_filename != DEV_NULL) {
      // don't open file DEV_NULL for performance sake.
      // This avoids costly unecessary message formatting and write.
      slogger->openFile(_filename);
    }
    slogger->setLogLevel(_logLevel);
    if(_consoleOutput) {
      slogger->setStdoutLogLevel(Logger::A2_NOTICE, true);
      slogger->setStdoutLogLevel(Logger::A2_WARN, true);
      slogger->setStdoutLogLevel(Logger::A2_ERROR, true);
    }
    _logger = slogger;
  }
  return _logger;
}

void LogFactory::setLogLevel(Logger::LEVEL level)
{
  _logLevel = level;
}

void LogFactory::setLogLevel(const std::string& level)
{
  if(level == V_DEBUG) {
    _logLevel = Logger::A2_DEBUG;
  } else if(level == V_INFO) {
    _logLevel = Logger::A2_INFO;
  } else if(level == V_NOTICE) {
    _logLevel = Logger::A2_NOTICE;
  } else if(level == V_WARN) {
    _logLevel = Logger::A2_WARN;
  } else if(level == V_ERROR) {
    _logLevel = Logger::A2_ERROR;
  }
}

void LogFactory::release() {
  delete _logger;
  _logger = 0;
}

} // namespace aria2
