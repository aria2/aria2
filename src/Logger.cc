/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include <cstdio>

#include "DlAbortEx.h"
#include "fmt.h"
#include "message.h"
#include "A2STR.h"
#include "a2time.h"
#include "BufferedFile.h"
#include "util.h"

namespace aria2 {

namespace {
static const std::string DEBUG_LABEL("DEBUG");

static const std::string INFO_LABEL("INFO");

static const std::string NOTICE_LABEL("NOTICE");

static const std::string WARN_LABEL("WARN");

static const std::string ERROR_LABEL("ERROR");
} // namespace

Logger::Logger()
  : logLevel_(Logger::A2_DEBUG),
    fpp_(0),
    stdoutfpp_(new BufferedFile(stdout)),
    stdoutField_(0)
{}

Logger::~Logger()
{
  delete fpp_;
  delete stdoutfpp_;
}

void Logger::openFile(const std::string& filename)
{
  closeFile();
  fpp_ = new BufferedFile(filename, BufferedFile::APPEND);
  if(!fpp_) {
    throw DL_ABORT_EX(fmt(EX_FILE_OPEN, utf8ToNative(filename).c_str(), "n/a"));
  }
}

void Logger::closeFile()
{
  if(fpp_) {
    fpp_->close();
    fpp_ = 0;
  }
}

void Logger::setStdoutLogLevel(Logger::LEVEL level, bool enabled)
{
  if(enabled) {
    stdoutField_ |= level;
  } else {
    stdoutField_ &= ~level;
  }
}

bool Logger::levelEnabled(LEVEL level)
{
  return (level >= logLevel_ && fpp_) || stdoutField_&level;
}

namespace {
const std::string& levelToString(Logger::LEVEL level)
{
  switch(level) {
  case Logger::A2_DEBUG:
    return DEBUG_LABEL;
  case Logger::A2_INFO:
    return INFO_LABEL;
  case Logger::A2_NOTICE:
    return NOTICE_LABEL;
  case Logger::A2_WARN:
    return WARN_LABEL;
  case Logger::A2_ERROR:
    return ERROR_LABEL;
  default:
    return A2STR::NIL;
  }
}
} // namespace

namespace {
void writeHeader
(BufferedFile& fp, Logger::LEVEL level, const char* sourceFile, int lineNum)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  char datestr[20]; // 'YYYY-MM-DD hh:mm:ss'+'\0' = 20 bytes
  struct tm tm;
  //tv.tv_sec may not be of type time_t.
  time_t timesec = tv.tv_sec;
  localtime_r(&timesec, &tm);
  size_t dateLength =
    strftime(datestr, sizeof(datestr), "%Y-%m-%d %H:%M:%S", &tm);
  assert(dateLength <= (size_t)20);
  fp.printf("%s.%06ld %s - ", datestr, tv.tv_usec,
            levelToString(level).c_str());
  if(sourceFile) {
    fp.printf("[%s:%d]", sourceFile, lineNum);
  }
}
} // namespace

namespace {
void writeStackTrace(BufferedFile& fp, const std::string& stackTrace)
{
  fp.write(stackTrace.data(), stackTrace.size());
}
} // namespace

void Logger::writeLog
(Logger::LEVEL level,
 const char* sourceFile,
 int lineNum,
 const char* msg,
 const std::string& trace,
 bool toStream,
 bool toConsole)
{
  if(toStream) {
    writeHeader(*fpp_, level, sourceFile, lineNum);
    fpp_->printf("%s\n", msg);
    writeStackTrace(*fpp_, trace);
    fpp_->flush();
  }
  if(toConsole) {
    stdoutfpp_->write("\n", 1);
    writeHeader(*stdoutfpp_, level, 0, 0);
    stdoutfpp_->printf("%s\n", msg);
    writeStackTrace(*stdoutfpp_, trace);
    stdoutfpp_->flush();
  }
}

void Logger::log
(LEVEL level,
 const char* sourceFile,
 int lineNum,
 const char* msg)
{
  writeLog(level, sourceFile, lineNum, msg, A2STR::NIL,
           level >= logLevel_ && fpp_,
           stdoutField_&level);
}

void Logger::log
(LEVEL level,
 const char* sourceFile,
 int lineNum,
 const std::string& msg)
{
  log(level, sourceFile, lineNum, msg.c_str());
}

void Logger::log
(LEVEL level,
 const char* sourceFile,
 int lineNum,
 const char* msg,
 const Exception& ex)
{
  writeLog(level, sourceFile, lineNum, msg, ex.stackTrace(),
           level >= logLevel_ && fpp_,
           stdoutField_&level);
}

void Logger::log
(LEVEL level,
 const char* sourceFile,
 int lineNum,
 const std::string& msg,
 const Exception& ex)
{
  log(level, sourceFile, lineNum, msg.c_str(), ex);
}

} // namespace aria2
