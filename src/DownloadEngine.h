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

typedef deque<SocketHandle> Sockets;
typedef deque<Command*> Commands;
typedef deque<CommandUuid> CommandUuids;
typedef multimap<SocketHandle, int> SockCmdMap;

class DownloadEngine {
private:
  void waitData(CommandUuids& activeCommandUuids);
  SockCmdMap rsockmap;
  SockCmdMap wsockmap;
  
  void shortSleep() const;
  bool addSocket(SockCmdMap& sockmap, const SocketHandle& socket,
		 const CommandUuid& commandUuid);
  bool deleteSocket(SockCmdMap& sockmap, const SocketHandle& socket,
		    const CommandUuid& commandUuid);
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

  bool addSocketForReadCheck(const SocketHandle& socket,
			     const CommandUuid& commandUuid);
  bool deleteSocketForReadCheck(const SocketHandle& socket,
				const CommandUuid& commandUuid);
  bool addSocketForWriteCheck(const SocketHandle& socket,
			      const CommandUuid& commandUuid);
  bool deleteSocketForWriteCheck(const SocketHandle& socket,
				 const CommandUuid& command);
  
};

template<class T1, class T2>
class PairFind {
private:
  T1 first;
  T2 second;
public:
  PairFind(T1 t1, T2 t2):first(t1), second(t2) {}

  bool operator()(const pair<T1, T2>& pa) {
    if(pa.first == first && pa.second == second) {
      return true;
    } else {
      return false;
    }
  }
};

#endif // _D_DOWNLOAD_ENGINE_H_

