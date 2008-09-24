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
#include "SharedHandle.h"
#include "Command.h"
#include "a2netcompat.h"
#include "TimeA2.h"
#include "a2io.h"
#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS
#include <string>
#include <deque>
#include <map>
#ifdef HAVE_EPOLL
# include <sys/epoll.h>
#endif // HAVE_EPOLL

namespace aria2 {

class Logger;
class Option;
class RequestGroupMan;
class FileAllocationMan;
class StatCalc;
class CheckIntegrityMan;
class SocketCore;
class CookieStorage;

class CommandEvent
{
private:
  Command* _command;
  int _events;
public:
  CommandEvent(Command* command, int events);

  Command* getCommand() const;

  int getEvents() const;

  void addEvents(int events);

  void removeEvents(int events);

  bool eventsEmpty() const;

  void processEvents(int events);

  bool operator==(const CommandEvent& event) const;
};

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

class ADNSEvent {
private:
  SharedHandle<AsyncNameResolver> _resolver;
  Command* _command;
  sock_t _socket;
  int _events;
public:
  ADNSEvent(const SharedHandle<AsyncNameResolver>& resolver, Command* command,
	    sock_t socket, int events);

  void processEvents(int events);

  bool operator==(const ADNSEvent& event) const;

  int getEvents() const;
};

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

class SocketEntry {
private:
  sock_t _socket;

  std::deque<CommandEvent> _commandEvents;

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

  std::deque<ADNSEvent> _adnsEvents;

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

#ifdef HAVE_EPOLL

  struct epoll_event _epEvent;

#endif // HAVE_EPOLL

public:

#ifdef HAVE_EPOLL

  enum EventType {
    EVENT_READ = EPOLLIN,
    EVENT_WRITE = EPOLLOUT,
    EVENT_ERROR = EPOLLERR,
    EVENT_HUP = EPOLLHUP,
  };

#else // !HAVE_EPOLL

  enum EventType {
    EVENT_READ = 1,
    EVENT_WRITE = 1 << 1,
    EVENT_ERROR = 1 << 2,
    EVENT_HUP = 1 << 3,
  };

#endif // !HAVE_EPOLL

  SocketEntry(sock_t socket);

  bool operator==(const SocketEntry& entry) const;

  bool operator<(const SocketEntry& entry) const;

  void addCommandEvent(Command* command, int events);

  void removeCommandEvent(Command* command, int events);

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

  void addADNSEvent(const SharedHandle<AsyncNameResolver>& resolver,
		    Command* command, int events);

  void removeADNSEvent(const SharedHandle<AsyncNameResolver>& resolver,
		       Command* command);

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

#ifdef HAVE_EPOLL

  struct epoll_event& getEpEvent();

#else // !HAVE_EPOLL

  int getEvents();

#endif // !HAVE_EPOLL

  sock_t getSocket() const;

  bool eventEmpty() const;

  void processEvents(int events);

};

#ifdef ENABLE_ASYNC_DNS

class DownloadEngine;

class AsyncNameResolverEntry {
private:
  SharedHandle<AsyncNameResolver> _nameResolver;

  Command* _command;

#ifdef HAVE_EPOLL
  // HAVE_EPOLL assumes c-ares

  size_t _socketsSize;
  
  sock_t _sockets[ARES_GETSOCK_MAXNUM];

#endif // HAVE_EPOLL

public:
  AsyncNameResolverEntry(const SharedHandle<AsyncNameResolver>& nameResolver,
			 Command* command);

  bool operator==(const AsyncNameResolverEntry& entry);

#ifdef HAVE_EPOLL

  void addSocketEvents(DownloadEngine* e);
  
  void removeSocketEvents(DownloadEngine* e);

#else // !HAVE_EPOLL

  int getFds(fd_set* rfdsPtr, fd_set* wfdsPtr);

  void process(fd_set* rfdsPtr, fd_set* wfdsPtr);

#endif // !HAVE_EPOLL

};

#endif // ENABLE_ASYNC_DNS

class DownloadEngine {
private:
  void waitData();
  std::deque<SharedHandle<SocketEntry> > socketEntries;
#ifdef ENABLE_ASYNC_DNS
  std::deque<SharedHandle<AsyncNameResolverEntry> > nameResolverEntries;
#endif // ENABLE_ASYNC_DNS

#ifdef HAVE_EPOLL

  int _epfd;

  struct epoll_event* _epEvents;

  static const size_t EPOLL_EVENTS_MAX = 1024;

#else // !HAVE_EPOLL
  // If epoll is not available, then use select system call.

  fd_set rfdset;
  fd_set wfdset;
  int fdmax;

#endif // !HAVE_EPOLL

  Logger* logger;
  
  SharedHandle<StatCalc> _statCalc;

  bool _haltRequested;

  class SocketPoolEntry {
  private:
    SharedHandle<SocketCore> _socket;

    std::map<std::string, std::string> _options;

    time_t _timeout;

    Time _registeredTime;
  public:
    SocketPoolEntry(const SharedHandle<SocketCore>& socket,
		    const std::map<std::string, std::string>& option,
		    time_t timeout);

    ~SocketPoolEntry();

    bool isTimeout() const;

    SharedHandle<SocketCore> getSocket() const;

    const std::map<std::string, std::string>& getOptions() const;
  };

  // key = IP address:port, value = SocketPoolEntry
  std::multimap<std::string, SocketPoolEntry> _socketPool;
 
  Time _lastSocketPoolScan;

  bool _noWait;

  std::deque<Command*> _routineCommands;

  SharedHandle<CookieStorage> _cookieStorage;

  void shortSleep() const;

  /**
   * Delegates to StatCalc
   */
  void calculateStatistics();

  void onEndOfRun();

  void afterEachIteration();
  
  void poolSocket(const std::string& ipaddr,
		  uint16_t port,
		  const SocketPoolEntry& entry);

  std::multimap<std::string, SocketPoolEntry>::iterator
  findSocketPoolEntry(const std::string& ipaddr, uint16_t port);
public:
  std::deque<Command*> commands;
  SharedHandle<RequestGroupMan> _requestGroupMan;
  SharedHandle<FileAllocationMan> _fileAllocationMan;
  SharedHandle<CheckIntegrityMan> _checkIntegrityMan;
  const Option* option;
  
  DownloadEngine();

  virtual ~DownloadEngine();

  void run();

  void cleanQueue();

#ifndef HAVE_EPOLL

  void updateFdSet();

#endif // !HAVE_EPOLL

  bool addSocketForReadCheck(const SharedHandle<SocketCore>& socket,
			     Command* command);
  bool deleteSocketForReadCheck(const SharedHandle<SocketCore>& socket,
				Command* command);
  bool addSocketForWriteCheck(const SharedHandle<SocketCore>& socket,
			      Command* command);
  bool deleteSocketForWriteCheck(const SharedHandle<SocketCore>& socket,
				 Command* command);

  bool addSocketEvents(sock_t socket, Command* command, int events
#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS
		       ,const SharedHandle<AsyncNameResolver>& rs =
		       SharedHandle<AsyncNameResolver>()
#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS
		       );

  bool deleteSocketEvents(sock_t socket, Command* command, int events
#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS
			  ,const SharedHandle<AsyncNameResolver>& rs =
			  SharedHandle<AsyncNameResolver>()
#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS
			  );

#ifdef ENABLE_ASYNC_DNS

  bool addNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver,
			    Command* command);
  bool deleteNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver,
			       Command* command);
#endif // ENABLE_ASYNC_DNS

  void addCommand(const Commands& commands);

  void fillCommand();

  void setStatCalc(const SharedHandle<StatCalc>& statCalc);

  bool isHaltRequested() const
  {
    return _haltRequested;
  }

  void requestHalt();

  void setNoWait(bool b);

  void addRoutineCommand(Command* command);

  void poolSocket(const std::string& ipaddr, uint16_t port,
		  const SharedHandle<SocketCore>& sock,
		  const std::map<std::string, std::string>& options,
		  time_t timeout = 15);

  void poolSocket(const std::string& ipaddr, uint16_t port,
		  const SharedHandle<SocketCore>& sock,
		  time_t timeout = 15);
  
  SharedHandle<SocketCore> popPooledSocket(const std::string& ipaddr,
					   uint16_t port);

  SharedHandle<SocketCore> popPooledSocket
  (std::map<std::string, std::string>& options,
   const std::string& ipaddr,
   uint16_t port);

  SharedHandle<SocketCore>
  popPooledSocket(const std::deque<std::string>& ipaddrs, uint16_t port);

  SharedHandle<SocketCore>
  popPooledSocket
  (std::map<std::string, std::string>& options,
   const std::deque<std::string>& ipaddrs,
   uint16_t port);

  SharedHandle<CookieStorage> getCookieStorage() const;
};

typedef SharedHandle<DownloadEngine> DownloadEngineHandle;

} // namespace aria2

#endif // _D_DOWNLOAD_ENGINE_H_

