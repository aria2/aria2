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
#include "DownloadEngine.h"
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS
#include "StatCalc.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "FileAllocationMan.h"
#include "CheckIntegrityMan.h"
#include "DownloadResult.h"
#include "StatCalc.h"
#include "LogFactory.h"
#include "Logger.h"
#include "TimeA2.h"
#include "a2time.h"
#include "Socket.h"
#include "Util.h"
#include "a2functional.h"
#include "DlAbortEx.h"
#include "ServerStatMan.h"
#include "CookieStorage.h"
#include <signal.h>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <cerrno>

namespace aria2 {

// 0 ... running
// 1 ... stop signal detected
// 2 ... stop signal processed by DownloadEngine
// 3 ... 2nd stop signal(force shutdown) detected
// 4 ... 2nd stop signal processed by DownloadEngine
volatile sig_atomic_t globalHaltRequested = 0;


CommandEvent::CommandEvent(Command* command, int events):
  _command(command), _events(events) {}

bool CommandEvent::operator==(const CommandEvent& commandEvent) const
{
  return _command == commandEvent._command;
}

int CommandEvent::getEvents() const
{
  return _events;
}

void CommandEvent::addEvents(int events)
{
  _events |= events;
}

void CommandEvent::removeEvents(int events)
{
  _events &= (~events); 
}

bool CommandEvent::eventsEmpty() const
{
  return _events == 0;
}

void CommandEvent::processEvents(int events)
{
  if((_events&events) ||
     ((SocketEntry::EVENT_ERROR|SocketEntry::EVENT_HUP)&events)) {
    _command->setStatusActive();
  }
  if(SocketEntry::EVENT_READ&events) {
    _command->readEventReceived();
  }
  if(SocketEntry::EVENT_WRITE&events) {
    _command->writeEventReceived();
  }
  if(SocketEntry::EVENT_ERROR&events) {
    _command->errorEventReceived();
  }
  if(SocketEntry::EVENT_HUP&events) {
    _command->hupEventReceived();
  }
}

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

ADNSEvent::ADNSEvent(const SharedHandle<AsyncNameResolver>& resolver,
		     Command* command,
		     sock_t socket, int events):
  _resolver(resolver), _command(command), _socket(socket), _events(events) {}

bool ADNSEvent::operator==(const ADNSEvent& event) const
{
  return _resolver == event._resolver;
}

int ADNSEvent::getEvents() const
{
  return _events;
}

void ADNSEvent::processEvents(int events)
{
  ares_socket_t readfd;
  ares_socket_t writefd;
  if(events&(SocketEntry::EVENT_READ|SocketEntry::EVENT_ERROR|SocketEntry::EVENT_HUP)) {
    readfd = _socket;
  } else {
    readfd = ARES_SOCKET_BAD;
  }
  if(events&(SocketEntry::EVENT_WRITE|SocketEntry::EVENT_ERROR|SocketEntry::EVENT_HUP)) {
    writefd = _socket;
  } else {
    writefd = ARES_SOCKET_BAD;
  }
  _resolver->process(readfd, writefd);
  _command->setStatusActive();
}

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

SocketEntry::SocketEntry(sock_t socket):_socket(socket)
{
#ifdef HAVE_EPOLL
  memset(&_epEvent, 0, sizeof(struct epoll_event));
#endif // HAVE_EPOLL
}

bool SocketEntry::operator==(const SocketEntry& entry) const
{
  return _socket == entry._socket;
}

bool SocketEntry::operator<(const SocketEntry& entry) const
{
  return _socket < entry._socket;
}

void SocketEntry::addCommandEvent(Command* command, int events)
{
  CommandEvent cev(command, events);
  std::deque<CommandEvent>::iterator i = std::find(_commandEvents.begin(),
						   _commandEvents.end(),
						   cev);
  if(i == _commandEvents.end()) {
    _commandEvents.push_back(cev);
  } else {
    (*i).addEvents(events);
  }
}

void SocketEntry::removeCommandEvent(Command* command, int events)
{
  CommandEvent cev(command, events);
  std::deque<CommandEvent>::iterator i = std::find(_commandEvents.begin(),
						   _commandEvents.end(),
						   cev);
  if(i == _commandEvents.end()) {
    // not found
  } else {
    (*i).removeEvents(events);
    if((*i).eventsEmpty()) {
      _commandEvents.erase(i);
    }
  }
}

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

void SocketEntry::addADNSEvent(const SharedHandle<AsyncNameResolver>& resolver,
			       Command* command, int events)
{
  ADNSEvent aev(resolver, command, _socket, events);
  std::deque<ADNSEvent>::iterator i = std::find(_adnsEvents.begin(),
						_adnsEvents.end(),
						aev);
  if(i == _adnsEvents.end()) {
    _adnsEvents.push_back(aev);
  }
}

void SocketEntry::removeADNSEvent(const SharedHandle<AsyncNameResolver>& resolver,
				  Command* command)
{
  ADNSEvent aev(resolver, command, _socket, 0);
  std::deque<ADNSEvent>::iterator i = std::find(_adnsEvents.begin(),
						_adnsEvents.end(),
						aev);
  if(i == _adnsEvents.end()) {
    // not found
  } else {
    _adnsEvents.erase(i);
  }
}

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

void SocketEntry::processEvents(int events)
{
  std::for_each(_commandEvents.begin(), _commandEvents.end(),
		std::bind2nd(std::mem_fun_ref(&CommandEvent::processEvents),
			     events));

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

  std::for_each(_adnsEvents.begin(), _adnsEvents.end(),
		std::bind2nd(std::mem_fun_ref(&ADNSEvent::processEvents),
			     events));

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

}

sock_t SocketEntry::getSocket() const
{
  return _socket;
}

bool SocketEntry::eventEmpty() const
{

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

  return _commandEvents.empty() && _adnsEvents.empty();

#else // !(HAVE_EPOLL && ENABLE_ASYNC_DNS)

  return _commandEvents.empty();

#endif // !(HAVE_EPOLL && ENABLE_ASYNC_DNS)

}

class AccEvent {
public:
  int operator()(int events, const CommandEvent& commandEvent) const
  {
    return events|commandEvent.getEvents();
  }

#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS

  int operator()(int events, const ADNSEvent& adnsEvent) const
  {
    return events|adnsEvent.getEvents();
  }

#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS

};

#ifdef  HAVE_EPOLL

struct epoll_event& SocketEntry::getEpEvent()
{
  _epEvent.data.ptr = this;

#ifdef ENABLE_ASYNC_DNS

  _epEvent.events =
    std::accumulate(_adnsEvents.begin(),
		    _adnsEvents.end(),
		    std::accumulate(_commandEvents.begin(),
				    _commandEvents.end(), 0, AccEvent()),
		    AccEvent());

#else // !ENABLE_ASYNC_DNS

  _epEvent.events =
    std::accumulate(_commandEvents.begin(), _commandEvents.end(), 0, AccEvent());

#endif // !ENABLE_ASYNC_DNS

  return _epEvent;
}

#else // !HAVE_EPOLL

int SocketEntry::getEvents()
{
  return
    std::accumulate(_commandEvents.begin(), _commandEvents.end(), 0, AccEvent());
}

#endif // !HAVE_EPOLL


#ifdef ENABLE_ASYNC_DNS

AsyncNameResolverEntry::AsyncNameResolverEntry
(const SharedHandle<AsyncNameResolver>& nameResolver, Command* command):
  _nameResolver(nameResolver), _command(command)
#ifdef HAVE_EPOLL
  , _socketsSize(0)
#endif // HAVE_EPOLL
{}

bool AsyncNameResolverEntry::operator==(const AsyncNameResolverEntry& entry)
{
  return _nameResolver == entry._nameResolver &&
    _command == entry._command;
}

#ifdef HAVE_EPOLL

void AsyncNameResolverEntry::addSocketEvents(DownloadEngine* e)
{
  _socketsSize = 0;
  int mask = _nameResolver->getsock(_sockets);
  if(mask == 0) {
    return;
  }
  size_t i;
  for(i = 0; i < ARES_GETSOCK_MAXNUM; ++i) {
    //epoll_event_t* epEventPtr = &_epEvents[_socketsSize];
    int events = 0;
    if(ARES_GETSOCK_READABLE(mask, i)) {
      events |= EPOLLIN;
    }
    if(ARES_GETSOCK_WRITABLE(mask, i)) {
      events |= EPOLLOUT;
    }
    if(events == 0) {
      // assume no further sockets are returned.
      break;
    }
    e->addSocketEvents(_sockets[i], _command, events, _nameResolver);
  }
  _socketsSize = i;
}
  
void AsyncNameResolverEntry::removeSocketEvents(DownloadEngine* e)
{
  for(size_t i = 0; i < _socketsSize; ++i) {
    e->deleteSocketEvents(_sockets[i], _command, 0, _nameResolver);
  }
}

#else // !HAVE_EPOLL

int AsyncNameResolverEntry::getFds(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  return _nameResolver->getFds(rfdsPtr, wfdsPtr);
}

void AsyncNameResolverEntry::process(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  _nameResolver->process(rfdsPtr, wfdsPtr);
  switch(_nameResolver->getStatus()) {
  case AsyncNameResolver::STATUS_SUCCESS:
  case AsyncNameResolver::STATUS_ERROR:
    _command->setStatusActive();
    break;
  default:
    break;
  }
}

#endif // !HAVE_EPOLL

#endif // ENABLE_ASYNC_DNS

DownloadEngine::DownloadEngine():logger(LogFactory::getInstance()),
				 _haltRequested(false),
				 _noWait(false),
				 _cookieStorage(new CookieStorage())
{
#ifdef HAVE_EPOLL

  _epfd = epoll_create(EPOLL_EVENTS_MAX);

  _epEvents = new struct epoll_event[EPOLL_EVENTS_MAX];

#else // !HAVE_EPOLL

  updateFdSet();

#endif // !HAVE_EPOLL
}

DownloadEngine::~DownloadEngine() {
  cleanQueue();

#ifdef HAVE_EPOLL

  if(_epfd != -1) {
    int r;
    while((r = close(_epfd)) == -1 && errno == EINTR);
    if(r == -1) {
      logger->error("Error occurred while closing epoll file descriptor %d: %s",
		    _epfd, strerror(errno));
    }
  }

  delete [] _epEvents;

#endif // HAVE_EPOLL
}

void DownloadEngine::cleanQueue() {
  std::for_each(commands.begin(), commands.end(), Deleter());
  commands.clear();
}

static void executeCommand(std::deque<Command*>& commands,
			   Command::STATUS statusFilter)
{
  size_t max = commands.size();
  for(size_t i = 0; i < max; i++) {
    Command* com = commands.front();
    commands.pop_front();
    if(com->statusMatch(statusFilter)) {
      com->transitStatus();
      if(com->execute()) {
	delete com;
	com = 0;
      }
    } else {
      commands.push_back(com);
    }
    if(com) {
      com->clearIOEvents();
    }
  }
}

void DownloadEngine::run() {

#ifdef HAVE_EPOLL

  if(_epfd == -1) {
    throw DlAbortEx("epoll_init() failed.");
  }

#endif // HAVE_EPOLL

  Time cp;
  cp.setTimeInSec(0);
  while(!commands.empty() || !_routineCommands.empty()) {
    if(cp.elapsed(1)) {
      cp.reset();
      executeCommand(commands, Command::STATUS_ALL);
    } else {
      executeCommand(commands, Command::STATUS_ACTIVE);
    }
    executeCommand(_routineCommands, Command::STATUS_ALL);
    afterEachIteration();
    if(!commands.empty()) {
      waitData();
    }
    _noWait = false;
    calculateStatistics();
  }
  onEndOfRun();
}

void DownloadEngine::shortSleep() const {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;
  fd_set rfds;
  FD_ZERO(&rfds);
  select(0, &rfds, NULL, NULL, &tv);
}

void DownloadEngine::waitData()
{
#ifdef HAVE_EPOLL

  // timeout is millisec
  int timeout = _noWait ? 0 : 1000;

  int res;
  while((res = epoll_wait(_epfd, _epEvents, EPOLL_EVENTS_MAX, timeout)) == -1 &&
	errno == EINTR);

  if(res > 0) {
    for(int i = 0; i < res; ++i) {
      SocketEntry* p = (SocketEntry*)_epEvents[i].data.ptr;
      p->processEvents(_epEvents[i].events);
    }
  }

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)

#else // !HAVE_EPOLL

  fd_set rfds;
  fd_set wfds;
  struct timeval tv;
  
  memcpy(&rfds, &rfdset, sizeof(fd_set));
  memcpy(&wfds, &wfdset, sizeof(fd_set));
  
#ifdef ENABLE_ASYNC_DNS

  for(std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
	nameResolverEntries.begin(); itr != nameResolverEntries.end(); ++itr) {
    SharedHandle<AsyncNameResolverEntry>& entry = *itr;
    int fd = entry->getFds(&rfds, &wfds);
    // TODO force error if fd == 0
    if(fdmax < fd) {
      fdmax = fd;
    }
  }

#endif // ENABLE_ASYNC_DNS

  tv.tv_sec = _noWait ? 0 : 1;
  tv.tv_usec = 0;
  int retval = select(fdmax+1, &rfds, &wfds, NULL, &tv);
  if(retval > 0) {
    for(std::deque<SharedHandle<SocketEntry> >::iterator i =
	  socketEntries.begin(); i != socketEntries.end(); ++i) {
      int events = 0;
      if(FD_ISSET((*i)->getSocket(), &rfds)) {
	events |= SocketEntry::EVENT_READ;
      }
      if(FD_ISSET((*i)->getSocket(), &wfds)) {
	events |= SocketEntry::EVENT_WRITE;
      }
      (*i)->processEvents(events);
    }
  }

#ifdef ENABLE_ASYNC_DNS

  for(std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator i =
	nameResolverEntries.begin(); i != nameResolverEntries.end(); ++i) {
    (*i)->process(&rfds, &wfds);
  }

#endif // ENABLE_ASYNC_DNS

#endif // !HAVE_EPOLL
}

#ifndef HAVE_EPOLL

void DownloadEngine::updateFdSet() {
  fdmax = 0;
  FD_ZERO(&rfdset);
  FD_ZERO(&wfdset);
  for(std::deque<SharedHandle<SocketEntry> >::iterator i =
	socketEntries.begin(); i != socketEntries.end(); ++i) {
    sock_t fd = (*i)->getSocket();
    int events = (*i)->getEvents();
    if(events&SocketEntry::EVENT_READ) {
      FD_SET(fd, &rfdset);
    }
    if(events&SocketEntry::EVENT_WRITE) {
      FD_SET(fd, &wfdset);
    }
    if(fdmax < fd) {
      fdmax = fd;
    }
  }
}

#endif // !HAVE_EPOLL

bool DownloadEngine::addSocketEvents(sock_t socket, Command* command, int events
#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS
				     ,const SharedHandle<AsyncNameResolver>& rs
#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS
				     )
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(socketEntries.begin(), socketEntries.end(), socketEntry);
  int r = 0;
  if(i != socketEntries.end() && (*i) == socketEntry) {

#ifdef HAVE_EPOLL

#ifdef ENABLE_ASYNC_DNS

    if(rs.isNull()) {
      (*i)->addCommandEvent(command, events);
    } else {
      (*i)->addADNSEvent(rs, command, events);
    }

#else // !ENABLE_ASYNC_DNS

    (*i)->addCommandEvent(command, events);

#endif // !ENABLE_ASYNC_DNS

    r = epoll_ctl(_epfd, EPOLL_CTL_MOD, (*i)->getSocket(), &(*i)->getEpEvent());
    if(r == -1) {
      // try EPOLL_CTL_ADD: There is a chance that previously socket X is
      // added to epoll, but it is closed and is not yet removed from
      // SocketEntries. In this case, EPOLL_CTL_MOD is failed with ENOENT.

      r = epoll_ctl(_epfd, EPOLL_CTL_ADD, (*i)->getSocket(), &(*i)->getEpEvent());
    }

#else // !HAVE_EPOLL

    (*i)->addCommandEvent(command, events);

#endif // !HAVE_EPOLL

  } else {
    socketEntries.insert(i, socketEntry);

#ifdef HAVE_EPOLL

#ifdef ENABLE_ASYNC_DNS

    if(rs.isNull()) {
      socketEntry->addCommandEvent(command, events);
    } else {
      socketEntry->addADNSEvent(rs, command, events);
    }

#else // !ENABLE_ASYNC_DNS

    socketEntry->addCommandEvent(command, events);    

#endif // !ENABLE_ASYNC_DNS

    r = epoll_ctl(_epfd, EPOLL_CTL_ADD, socketEntry->getSocket(), &socketEntry->getEpEvent());

#else // !HAVE_EPOLL

    socketEntry->addCommandEvent(command, events);

#endif // !HAVE_EPOLL

  }

#ifndef HAVE_EPOLL

  updateFdSet();

#endif // !HAVE_EPOLL

  if(r == -1) {
    logger->debug("Failed to add socket event %d:%s", socket, strerror(errno));
    return false;
  } else {
    return true;
  }
}

bool DownloadEngine::deleteSocketEvents(sock_t socket, Command* command, int events
#if defined HAVE_EPOLL && defined ENABLE_ASYNC_DNS
					,const SharedHandle<AsyncNameResolver>& rs
#endif // HAVE_EPOLL && ENABLE_ASYNC_DNS
					)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(socketEntries.begin(), socketEntries.end(), socketEntry);
  if(i != socketEntries.end() && (*i) == socketEntry) {

#ifdef HAVE_EPOLL

#ifdef ENABLE_ASYNC_DNS

    if(rs.isNull()) {
      (*i)->removeCommandEvent(command, events);
    } else {
      (*i)->removeADNSEvent(rs, command);
    }

#else // !ENABLE_ASYNC_DNS

    (*i)->removeCommandEvent(command, events);    

#endif // !ENABLE_ASYNC_DNS

#else // !HAVE_EPOLL

    (*i)->removeCommandEvent(command, events);

#endif // !HAVE_EPOLL

    int r = 0;
    if((*i)->eventEmpty()) {

#ifdef HAVE_EPOLL

      r = epoll_ctl(_epfd, EPOLL_CTL_DEL, (*i)->getSocket(), 0);

#endif // HAVE_EPOLL

      socketEntries.erase(i);
    } else {

#ifdef HAVE_EPOLL

      // If socket is closed, then it seems it is automatically removed from
      // epoll, so following EPOLL_CTL_MOD may fail.
      r = epoll_ctl(_epfd, EPOLL_CTL_MOD, (*i)->getSocket(), &(*i)->getEpEvent());
      if(r == -1) {
	logger->debug("Failed to delete socket event, but may be ignored:%s", strerror(errno));
      }
#endif // HAVE_EPOLL

    }

#ifndef HAVE_EPOLL

    updateFdSet();

#endif // !HAVE_EPOLL

    if(r == -1) {
      logger->debug("Failed to delete socket event:%s", strerror(errno));
      return false;
    } else {
      return true;
    }
  } else {
    logger->debug("Socket %d is not found in SocketEntries.", socket);
    return false;
  }
}

bool DownloadEngine::addSocketForReadCheck(const SocketHandle& socket,
					   Command* command)
{
  return addSocketEvents(socket->getSockfd(), command, SocketEntry::EVENT_READ);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
					      Command* command)
{
  return deleteSocketEvents(socket->getSockfd(), command, SocketEntry::EVENT_READ);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
					    Command* command)
{
  return addSocketEvents(socket->getSockfd(), command, SocketEntry::EVENT_WRITE);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
					       Command* command)
{
  return deleteSocketEvents(socket->getSockfd(), command, SocketEntry::EVENT_WRITE);
}

void DownloadEngine::calculateStatistics()
{
  if(!_statCalc.isNull()) {
    _statCalc->calculateStat(_requestGroupMan, _fileAllocationMan, _checkIntegrityMan);
  }
}

void DownloadEngine::onEndOfRun()
{
  _requestGroupMan->updateServerStat();
  _requestGroupMan->closeFile();
  _requestGroupMan->save();
}

void DownloadEngine::afterEachIteration()
{
  if(globalHaltRequested == 1) {
    logger->notice(_("Shutdown sequence commencing... Press Ctrl-C again for emergency shutdown."));
    requestHalt();
    globalHaltRequested = 2;
  } else if(globalHaltRequested == 3) {
    logger->notice(_("Emergency shutdown sequence commencing..."));
    _requestGroupMan->forceHalt();
    globalHaltRequested = 4;
  }
}

void DownloadEngine::requestHalt()
{
  _haltRequested = true;
  _requestGroupMan->halt();
}

void DownloadEngine::fillCommand()
{
  std::deque<Command*> commands;
  _requestGroupMan->getInitialCommands(commands, this);
  addCommand(commands);
}

void DownloadEngine::setStatCalc(const StatCalcHandle& statCalc)
{
  _statCalc = statCalc;
}

void DownloadEngine::addCommand(const Commands& commands)
{
  this->commands.insert(this->commands.end(), commands.begin(), commands.end());
}

#ifdef ENABLE_ASYNC_DNS
bool DownloadEngine::addNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
    std::find(nameResolverEntries.begin(), nameResolverEntries.end(), entry);
  if(itr == nameResolverEntries.end()) {
    nameResolverEntries.push_back(entry);

#ifdef HAVE_EPOLL

    entry->addSocketEvents(this);

#endif // HAVE_EPOLL

    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
    std::find(nameResolverEntries.begin(), nameResolverEntries.end(), entry);
  if(itr == nameResolverEntries.end()) {
    return false;
  } else {

#ifdef HAVE_EPOLL

    (*itr)->removeSocketEvents(this);

#endif // HAVE_EPOLL

    nameResolverEntries.erase(itr);
    return true;
  }
}
#endif // ENABLE_ASYNC_DNS

void DownloadEngine::setNoWait(bool b)
{
  _noWait = b;
}

void DownloadEngine::addRoutineCommand(Command* command)
{
  _routineCommands.push_back(command);
}

SharedHandle<CookieStorage> DownloadEngine::getCookieStorage() const
{
  return _cookieStorage;
}

void DownloadEngine::poolSocket(const std::string& ipaddr, uint16_t port,
				const SharedHandle<SocketCore>& sock,
				time_t timeout)
{
  std::string addr = ipaddr+":"+Util::uitos(port);
  logger->info("Pool socket for %s", addr.c_str());

  SocketPoolEntry e(sock, timeout);
  std::multimap<std::string, SocketPoolEntry>::value_type p(addr, e);
  _socketPool.insert(p);
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket(const std::string& ipaddr, uint16_t port)
{
  SharedHandle<SocketCore> s;
  std::string addr = ipaddr+":"+Util::uitos(port);

  std::multimap<std::string, SocketPoolEntry>::iterator first = _socketPool.find(addr);
  
  for(std::multimap<std::string, SocketPoolEntry>::iterator i = first;
      i != _socketPool.end() && (*i).first == addr; ++i) {
    const SocketPoolEntry& e = (*i).second;
    if(!e.isTimeout()) {
      logger->info("Reuse socket for %s", addr.c_str());
      s = e.getSocket();
      _socketPool.erase(first, ++i);
      break;
    }
  }
  return s;
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(const std::deque<std::string>& ipaddrs, uint16_t port)
{
  for(std::deque<std::string>::const_iterator i = ipaddrs.begin();
      i != ipaddrs.end(); ++i) {
    SharedHandle<SocketCore> s = popPooledSocket(*i, port);
    if(!s.isNull()) {
      return s;
    }
  }
  return SharedHandle<SocketCore>();
}

DownloadEngine::SocketPoolEntry::SocketPoolEntry
(const SharedHandle<SocketCore>& socket,
 time_t timeout):
  _socket(socket),
  _timeout(timeout) {}

DownloadEngine::SocketPoolEntry::~SocketPoolEntry() {}

bool DownloadEngine::SocketPoolEntry::isTimeout() const
{
  return _registeredTime.elapsed(_timeout);
}

SharedHandle<SocketCore> DownloadEngine::SocketPoolEntry::getSocket() const
{
  return _socket;
}

} // namespace aria2
