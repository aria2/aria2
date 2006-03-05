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
  case DEBUG:
    levelStr = "DEBUG";
    break;
  case ERROR:
    levelStr = "ERROR";
    break;
  case INFO:
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
  va_list ap;
  va_start(ap, msg);
  writeLog(DEBUG, msg, ap);
  va_end(ap);
}

void SimpleLogger::debug(const char* msg, Exception* e, ...) const {
  va_list ap;
  va_start(ap, e);
  writeLog(DEBUG, msg, ap, e);
  va_end(ap);
}

  
void SimpleLogger::info(const char* msg, ...) const {
  va_list ap;
  va_start(ap, msg);
  writeLog(INFO, msg, ap);
  va_end(ap);
}

void SimpleLogger::info(const char* msg, Exception* e, ...) const {
  va_list ap;
  va_start(ap, e);
  writeLog(INFO, msg, ap, e);
  va_end(ap);
}

  
void SimpleLogger::error(const char* msg, ...) const {
  va_list ap;
  va_start(ap, msg);
  writeLog(ERROR, msg, ap);
  va_end(ap);
}

void SimpleLogger::error(const char* msg, Exception* e, ...) const {
  va_list ap;
  va_start(ap, e);
  writeLog(ERROR, msg, ap, e);
  va_end(ap);
}

  
