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
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#define WRITE_LOG(LEVEL, MSG) \
va_list ap;\
va_start(ap, MSG);\
writeLog(Logger::LEVEL, MSG, ap);\
va_end(ap);

#define WRITE_LOG_EX(LEVEL, MSG, EX) \
va_list ap;\
va_start(ap, EX);\
writeLog(Logger::LEVEL, MSG, ap, EX);\
va_end(ap);

SimpleLogger::SimpleLogger(string filename) {
  file = fopen(filename.c_str(), "a");
}

SimpleLogger::SimpleLogger(FILE* logfile) {
  file = logfile;
}

SimpleLogger::~SimpleLogger() {
  if(file != NULL) {
    fclose(file);
  }
}

void SimpleLogger::writeLog(int level, const char* msg, va_list ap, Exception* e) const
{
  string levelStr;
  switch(level) {
  case Logger::DEBUG:
    levelStr = "DEBUG";
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
  vfprintf(file, string(string(datestr)+" - "+levelStr+" - "+Util::replace(msg, "\r", "")+"\n").c_str(), ap);
  if(e != NULL) {
    fprintf(file, string(string(datestr)+" - "+levelStr+" - exception: "+Util::replace(e->getMsg(), "\r", "")+"\n").c_str());
  }
  fflush(file);
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

  
