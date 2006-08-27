/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "SimpleLogger.h"
#include "Util.h"
#include "DlAbortEx.h"
#include "message.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

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

SimpleLogger::SimpleLogger(FILE* logfile):file(logfile), stdoutField(0) {}

SimpleLogger::~SimpleLogger() {
  closeFile();
}

void SimpleLogger::openFile(const string& filename) {
  file = fopen(filename.c_str(), "a");
  if(file == NULL) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }
}

void SimpleLogger::closeFile() {
  if(file != NULL) {
    fclose(file);
  }
}

void SimpleLogger::setStdout(int level, bool enabled) {
  if(enabled) {
    stdoutField |= level;
  } else {
    stdoutField &= ~level;
  }
}

void SimpleLogger::writeHeader(FILE* file, string date, string level) const {
  fprintf(file, "%s - %s - ", date.c_str(), level.c_str());
}

void SimpleLogger::writeLog(FILE* file, int level, const char* msg, va_list ap, Exception* e) const
{
  string levelStr;
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
  char datestr[26];
  ctime_r(&now, datestr);
  datestr[strlen(datestr)-1] = '\0';
  writeHeader(file, datestr, levelStr);
  vfprintf(file, string(Util::replace(msg, "\r", "")+"\n").c_str(), ap);
  if(e != NULL) {
    writeHeader(file, datestr, levelStr);
    fprintf(file, "exception: %s\n", Util::replace(e->getMsg(), "\r", "").c_str());
  }
  fflush(file);
}

void SimpleLogger::writeFile(int level, const char* msg, va_list ap, Exception* e) const {
  writeLog(file, level, msg, ap, e);
  if(stdoutField&level) {
    fprintf(stdout, "\n");
    writeLog(stdout, level, msg, ap, e);
  }
}

void SimpleLogger::debug(const char* msg, ...) const {
  WRITE_LOG(DEBUG, msg);
}

void SimpleLogger::debug(const char* msg, Exception* e, ...) const {
  WRITE_LOG_EX(DEBUG, msg, e);
}

void SimpleLogger::info(const char* msg, ...) const {
  WRITE_LOG(INFO, msg);
}

void SimpleLogger::info(const char* msg, Exception* e, ...) const {
  WRITE_LOG_EX(INFO, msg, e);
}

void SimpleLogger::notice(const char* msg, ...) const {
  WRITE_LOG(NOTICE, msg);
}

void SimpleLogger::notice(const char* msg, Exception* e, ...) const {
  WRITE_LOG_EX(INFO, msg, e);
}

void SimpleLogger::warn(const char* msg, ...) const {
  WRITE_LOG(WARN, msg);
}

void SimpleLogger::warn(const char* msg, Exception* e, ...) const {
  WRITE_LOG_EX(WARN, msg, e);
}

void SimpleLogger::error(const char* msg, ...) const {
  WRITE_LOG(ERROR, msg);
}

void SimpleLogger::error(const char* msg, Exception* e, ...) const {
  WRITE_LOG_EX(ERROR, msg, e);
}

  
