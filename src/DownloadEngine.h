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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_DOWNLOAD_ENGINE_H_
#define _D_DOWNLOAD_ENGINE_H_

#include "common.h"
#include "Command.h"

class SocketCore;
typedef SharedHandle<SocketCore> SocketHandle;
typedef deque<SocketHandle> Sockets;
class Logger;
class Option;
class NameResolver;
typedef SharedHandle<NameResolver> NameResolverHandle;
class RequestGroupMan;
typedef SharedHandle<RequestGroupMan> RequestGroupManHandle;
class FileAllocationMan;
typedef SharedHandle<FileAllocationMan> FileAllocationManHandle;
class StatCalc;
typedef SharedHandle<StatCalc> StatCalcHandle;
class CheckIntegrityMan;
typedef SharedHandle<CheckIntegrityMan> CheckIntegrityManHandle;

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
	      TYPE type);

  bool operator==(const SocketEntry& entry);
};

typedef deque<SocketEntry> SocketEntries;

#ifdef ENABLE_ASYNC_DNS
class NameResolverEntry {
public:
  NameResolverHandle nameResolver;
  Command* command;
public:
  NameResolverEntry(const NameResolverHandle& nameResolver,
		    Command* command);

  bool operator==(const NameResolverEntry& entry);
};

typedef deque<NameResolverEntry> NameResolverEntries;
#endif // ENABLE_ASYNC_DNS


class DownloadEngine {
private:
  void waitData();
  SocketEntries socketEntries;
#ifdef ENABLE_ASYNC_DNS
  NameResolverEntries nameResolverEntries;
#endif // ENABLE_ASYNC_DNS
  fd_set rfdset;
  fd_set wfdset;
  int32_t fdmax;

  const Logger* logger;
  
  StatCalcHandle _statCalc;

  bool _haltRequested;

  void shortSleep() const;
  bool addSocket(const SocketEntry& socketEntry);
  bool deleteSocket(const SocketEntry& socketEntry);
  void executeCommand(Command::STATUS statusFilter);

  /**
   * Delegates to StatCalc
   */
  void calculateStatistics();

  void onEndOfRun();

  void afterEachIteration();

public:
  bool noWait;
  Commands commands;
  RequestGroupManHandle _requestGroupMan;
  FileAllocationManHandle _fileAllocationMan;
  CheckIntegrityManHandle _checkIntegrityMan;
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
#ifdef ENABLE_ASYNC_DNS
  bool addNameResolverCheck(const NameResolverHandle& resolver,
			    Command* command);
  bool deleteNameResolverCheck(const NameResolverHandle& resolver,
			       Command* command);
#endif // ENABLE_ASYNC_DNS

  void addCommand(const Commands& commands);

  void fillCommand();

  void setStatCalc(const StatCalcHandle& statCalc);

  bool isHaltRequested() const
  {
    return _haltRequested;
  }
};

typedef SharedHandle<DownloadEngine> DownloadEngineHandle;

#endif // _D_DOWNLOAD_ENGINE_H_

