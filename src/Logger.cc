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
#include "Logger.h"

#include <cstring>

#include "DlAbortEx.h"
#include "StringFormat.h"
#include "message.h"
#include "LogFormatter.h"

namespace aria2 {

const std::string Logger::DEBUG_LABEL("DEBUG");

const std::string Logger::NOTICE_LABEL("NOTICE");

const std::string Logger::WARN_LABEL("WARN");

const std::string Logger::ERROR_LABEL("ERROR");

const std::string Logger::INFO_LABEL("INFO");

Logger::Logger():
  logFormatter_(0), logLevel_(Logger::A2_DEBUG), stdoutField_(0) {}

Logger::~Logger()
{
  closeFile();
  delete logFormatter_;
}

void Logger::openFile(const std::string& filename)
{
  file_.open(filename.c_str(), std::ios::app|std::ios::binary);
  if(!file_) {
    throw DL_ABORT_EX
      (StringFormat(EX_FILE_OPEN, filename.c_str(), "n/a").str());
  }
}

void Logger::closeFile()
{
  if(file_.is_open()) {
    file_.close();
  }
}

#define WRITE_LOG(LEVEL, LEVEL_LABEL, MSG)              \
  if(LEVEL >= logLevel_ && file_.is_open()) {           \
    va_list ap;                                         \
    va_start(ap, MSG);                                  \
    writeLog(file_, LEVEL, LEVEL_LABEL, MSG, ap);       \
    va_end(ap);                                         \
    file_ << std::flush;                                \
  }                                                     \
  if(stdoutField_&LEVEL) {                              \
    std::cout << "\n";                                  \
    va_list ap;                                         \
    va_start(ap, MSG);                                  \
    writeLog(std::cout, LEVEL, LEVEL_LABEL, MSG, ap);   \
    va_end(ap);                                         \
    std::cout << std::flush;                            \
  }                                                     \

#define WRITE_LOG_EX(LEVEL, LEVEL_LABEL, MSG, EX)       \
  if(LEVEL >= logLevel_ && file_.is_open()) {           \
    va_list ap;                                         \
    va_start(ap, EX);                                   \
    writeLog(file_, LEVEL, LEVEL_LABEL, MSG, ap);       \
    va_end(ap);                                         \
    writeStackTrace(file_, LEVEL, LEVEL_LABEL, EX);     \
    file_ << std::flush;                                \
  }                                                     \
  if(stdoutField_&LEVEL) {                              \
    std::cout << "\n";                                  \
    va_list ap;                                         \
    va_start(ap, EX);                                   \
    writeLog(std::cout, LEVEL, LEVEL_LABEL, MSG, ap);   \
    va_end(ap);                                         \
    writeStackTrace(std::cout, LEVEL, LEVEL_LABEL, EX); \
    std::cout << std::flush;                            \
  }                                                     \

void Logger::debug(const char* msg, ...)
{
  WRITE_LOG(A2_DEBUG, DEBUG_LABEL, msg);
}

void Logger::debug(const char* msg, const Exception& ex, ...)
{
  WRITE_LOG_EX(A2_DEBUG, DEBUG_LABEL, msg, ex);
}

void Logger::info(const char* msg, ...)
{
  WRITE_LOG(A2_INFO, INFO_LABEL, msg);
}

void Logger::info(const char* msg, const Exception& ex, ...)
{
  WRITE_LOG_EX(A2_INFO, INFO_LABEL, msg, ex);
}

void Logger::notice(const char* msg, ...)
{
  WRITE_LOG(A2_NOTICE, NOTICE_LABEL, msg);
}

void Logger::notice(const char* msg, const Exception& ex, ...)
{
  WRITE_LOG_EX(A2_NOTICE, NOTICE_LABEL, msg, ex);
}

void Logger::warn(const char* msg, ...)
{
  WRITE_LOG(A2_WARN, WARN_LABEL, msg);
}

void Logger::warn(const char* msg, const Exception& ex, ...)
{
  WRITE_LOG_EX(A2_WARN, WARN_LABEL, msg, ex);
}

void Logger::error(const char*  msg, ...)
{
  WRITE_LOG(A2_ERROR, ERROR_LABEL, msg);
}

void Logger::error(const char* msg, const Exception& ex, ...)
{
  WRITE_LOG_EX(A2_ERROR, ERROR_LABEL, msg, ex);
}

void Logger::setLogFormatter(LogFormatter* logFormatter)
{
  delete logFormatter_;
  logFormatter_ = logFormatter;
}

void Logger::setStdoutLogLevel(Logger::LEVEL level, bool enabled)
{
  if(enabled) {
    stdoutField_ |= level;
  } else {
    stdoutField_ &= ~level;
  }
}

void Logger::writeLog
(std::ostream& o, LEVEL logLevel, const std::string& logLevelLabel,
 const char* msg, va_list ap)
{
  if(logFormatter_) {
    logFormatter_->writeLog(o, logLevel, logLevelLabel, msg, ap);
  }
}

void Logger::writeStackTrace
(std::ostream& o, LEVEL logLevel, const std::string& logLevelLabel,
 const Exception& ex)
{
  if(logFormatter_) {
    logFormatter_->writeStackTrace(o, logLevel, logLevelLabel, ex);
  }
}

bool Logger::debug()
{
  return levelEnabled(A2_DEBUG);
}

bool Logger::info()
{
  return levelEnabled(A2_INFO);
}

} // namespace aria2
