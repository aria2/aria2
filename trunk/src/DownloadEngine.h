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
#ifndef _D_DOWNLOAD_ENGINE_H_
#define _D_DOWNLOAD_ENGINE_H_

#include "Command.h"
#include "Socket.h"
#include "SegmentMan.h"
#include "common.h"
#include "Logger.h"
#include "Option.h"
#include <sys/time.h>
#include <deque>

typedef deque<Socket*> Sockets;
typedef deque<Command*> Commands;
typedef multimap<Socket*, Command*> SockCmdMap;

class DownloadEngine {
private:
  void waitData(Sockets& activeSockets);
  Sockets rsockets;
  Sockets wsockets;
  SockCmdMap sockCmdMap;

  void shortSleep() const;
  bool addSocket(Sockets& sockets, Socket* socket, Command* command);
  bool deleteSocket(Sockets& sockets, Socket* socket);
protected:
  const Logger* logger;
  virtual void initStatistics() = 0;
  virtual void calculateStatistics() = 0;
  virtual void onEndOfRun() = 0;
  virtual void afterEachIteration() {}
public:
  bool noWait;
  Commands commands;
  SegmentMan* segmentMan;
  const Option* option;

  DownloadEngine();
  virtual ~DownloadEngine();

  void run();

  void cleanQueue();

  bool addSocketForReadCheck(Socket* socket, Command* command);
  bool deleteSocketForReadCheck(Socket* socket);
  bool addSocketForWriteCheck(Socket* socket, Command* command);
  bool deleteSocketForWriteCheck(Socket* socket);
  
};

#endif // _D_DOWNLOAD_ENGINE_H_

