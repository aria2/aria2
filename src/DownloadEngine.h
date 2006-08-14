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
#ifdef HAVE_LIBARES
# include "NameResolver.h"
#endif // HAVE_LIBARES

typedef deque<SocketHandle> Sockets;
typedef deque<Command*> Commands;

class SocketEntry {
public:
  enum TYPE {
    TYPE_RD,
    TYPE_WR,
  };

  SocketHandle socket;
  Command* command;
  TYPE type;
public:
  SocketEntry(const SocketHandle& socket,
	      Command* command,
	      TYPE type):
    socket(socket), command(command), type(type) {}
  ~SocketEntry() {}

  bool operator==(const SocketEntry& entry) {
    return socket == entry.socket &&
      command == entry.command &&
      type == entry.type;
  }
};

typedef deque<SocketEntry> SocketEntries;

#ifdef HAVE_LIBARES
class NameResolverEntry {
public:
  NameResolverHandle nameResolver;
  Command* command;
public:
  NameResolverEntry(const NameResolverHandle& nameResolver,
		    Command* command):
    nameResolver(nameResolver), command(command) {}
  ~NameResolverEntry() {}

  bool operator==(const NameResolverEntry& entry) {
    return nameResolver == entry.nameResolver &&
      command == entry.command;
  }
};

typedef deque<NameResolverEntry> NameResolverEntries;
#endif // HAVE_LIBARES


class DownloadEngine {
private:
  void waitData(Commands& activeCommands);
  SocketEntries socketEntries;
#ifdef HAVE_LIBARES
  NameResolverEntries nameResolverEntries;
#endif // HAVE_LIBARES
  fd_set rfdset;
  fd_set wfdset;
  int fdmax;

  void shortSleep() const;
  bool addSocket(const SocketEntry& socketEntry);
  bool deleteSocket(const SocketEntry& socketEntry);
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

  void updateFdSet();

  bool addSocketForReadCheck(const SocketHandle& socket,
			     Command* command);
  bool deleteSocketForReadCheck(const SocketHandle& socket,
				Command* command);
  bool addSocketForWriteCheck(const SocketHandle& socket,
			      Command* command);
  bool deleteSocketForWriteCheck(const SocketHandle& socket,
				 Command* command);
#ifdef HAVE_LIBARES
  bool addNameResolverCheck(const NameResolverHandle& resolver,
			    Command* command);
  bool deleteNameResolverCheck(const NameResolverHandle& resolver,
			       Command* command);
#endif // HAVE_LIBARES
};

#endif // _D_DOWNLOAD_ENGINE_H_

