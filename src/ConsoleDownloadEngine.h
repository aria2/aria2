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
#ifndef _D_CONSOLE_DOWNLOAD_ENGINE_H_
#define _D_CONSOLE_DOWNLOAD_ENGINE_H_

#include "DownloadEngine.h"

class ConsoleDownloadEngine : public DownloadEngine {
private:
  struct timeval cp;
  long long int psize;
  int speed;
protected:
  void sendStatistics(long long int currentSize, long long int totalSize);
  void initStatistics();
  void calculateStatistics();
  void onEndOfRun();
public:
  ConsoleDownloadEngine();
  ~ConsoleDownloadEngine();
};

#endif // _D_CONSOLE_DOWNLOAD_ENGINE_H_
