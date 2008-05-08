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
#include <signal.h>
#include <cstring>
#include <algorithm>

namespace aria2 {

// 0 ... running
// 1 ... stop signal detected
// 2 ... stop signal processed by DownloadEngine
// 3 ... 2nd stop signal(force shutdown) detected
// 4 ... 2nd stop signal processed by DownloadEngine
volatile sig_atomic_t globalHaltRequested = 0;

SocketEntry::SocketEntry(const SocketHandle& socket,
			 Command* command,
			 TYPE type):
  socket(socket), command(command), type(type) {}

bool SocketEntry::operator==(const SocketEntry& entry)
{
  return socket == entry.socket &&
    command == entry.command &&
    type == entry.type;
}

#ifdef ENABLE_ASYNC_DNS
AsyncNameResolverEntry::AsyncNameResolverEntry
(const SharedHandle<AsyncNameResolver>& nameResolver,
 Command* command):
  nameResolver(nameResolver), command(command) {}

bool AsyncNameResolverEntry::operator==(const AsyncNameResolverEntry& entry)
{
  return nameResolver == entry.nameResolver &&
    command == entry.command;
}
#endif // ENABLE_ASYNC_DNS

DownloadEngine::DownloadEngine():logger(LogFactory::getInstance()),
				 _haltRequested(false),
				 _noWait(false)
{}

DownloadEngine::~DownloadEngine() {
  cleanQueue();
}

class Deleter {
public:
  template<class T>
  void operator()(T* ptr) {
    delete ptr;
  }
};

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
      if(com->execute()) {
	delete com;
      } else {
	com->transitStatus();
      }
    } else {
      commands.push_back(com);
    }
  }
}

void DownloadEngine::run() {
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

void DownloadEngine::waitData() {
  fd_set rfds;
  fd_set wfds;
  struct timeval tv;
  
  memcpy(&rfds, &rfdset, sizeof(fd_set));
  memcpy(&wfds, &wfdset, sizeof(fd_set));
  
#ifdef ENABLE_ASYNC_DNS
  for(AsyncNameResolverEntries::iterator itr = nameResolverEntries.begin();
      itr != nameResolverEntries.end(); ++itr) {
    AsyncNameResolverEntry& entry = *itr;
    int fd = entry.nameResolver->getFds(&rfds, &wfds);
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
    for(SocketEntries::iterator itr = socketEntries.begin();
	itr != socketEntries.end(); ++itr) {
      SocketEntry& entry = *itr;
      if(FD_ISSET(entry.socket->getSockfd(), &rfds) ||
	 FD_ISSET(entry.socket->getSockfd(), &wfds)) {
	entry.command->setStatusActive();
      }
    }
  }
#ifdef ENABLE_ASYNC_DNS
  for(AsyncNameResolverEntries::iterator itr = nameResolverEntries.begin();
      itr != nameResolverEntries.end(); ++itr) {
    AsyncNameResolverEntry& entry = *itr;
    entry.nameResolver->process(&rfds, &wfds);
    switch(entry.nameResolver->getStatus()) {
    case AsyncNameResolver::STATUS_SUCCESS:
    case AsyncNameResolver::STATUS_ERROR:
      entry.command->setStatusActive();
      break;
    default:
      break;
    }
  }
#endif // ENABLE_ASYNC_DNS
}

void DownloadEngine::updateFdSet() {
  fdmax = 0;
  FD_ZERO(&rfdset);
  FD_ZERO(&wfdset);
  for(SocketEntries::iterator itr = socketEntries.begin();
      itr != socketEntries.end(); ++itr) {
    SocketEntry& entry = *itr;
    int fd = entry.socket->getSockfd();
    switch(entry.type) {
    case SocketEntry::TYPE_RD:
      FD_SET(fd, &rfdset);
      break;
    case SocketEntry::TYPE_WR:
      FD_SET(fd, &wfdset);
      break;
    }
    if(fdmax < fd) {
      fdmax = fd;
    }
  }
}

bool DownloadEngine::addSocket(const SocketEntry& entry) {
  SocketEntries::iterator itr =
    std::find(socketEntries.begin(), socketEntries.end(), entry);
  if(itr == socketEntries.end()) {
    socketEntries.push_back(entry);
    updateFdSet();
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteSocket(const SocketEntry& entry) {
  SocketEntries::iterator itr =
    std::find(socketEntries.begin(), socketEntries.end(), entry);
  if(itr == socketEntries.end()) {
    return false;
  } else {
    socketEntries.erase(itr);
    updateFdSet();
    return true;
  }
}

bool DownloadEngine::addSocketForReadCheck(const SocketHandle& socket,
					   Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_RD);
  return addSocket(entry);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
					      Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_RD);
  return deleteSocket(entry);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
					    Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_WR);
  return addSocket(entry);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
					       Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_WR);
  return deleteSocket(entry);
}

void DownloadEngine::calculateStatistics()
{
  if(!_statCalc.isNull()) {
    _statCalc->calculateStat(_requestGroupMan, _fileAllocationMan, _checkIntegrityMan);
  }
}

void DownloadEngine::onEndOfRun()
{
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
  addCommand(_requestGroupMan->getInitialCommands(this));
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
(const SharedHandle<AsyncNameResolver>& resolver,
 Command* command)
{
  AsyncNameResolverEntry entry(resolver, command);
  AsyncNameResolverEntries::iterator itr =
    std::find(nameResolverEntries.begin(), nameResolverEntries.end(), entry);
  if(itr == nameResolverEntries.end()) {
    nameResolverEntries.push_back(entry);
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver,
 Command* command)
{
  AsyncNameResolverEntry entry(resolver, command);
  AsyncNameResolverEntries::iterator itr =
    std::find(nameResolverEntries.begin(), nameResolverEntries.end(), entry);
  if(itr == nameResolverEntries.end()) {
    return false;
  } else {
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

void DownloadEngine::poolSocket(const std::string& ipaddr, uint16_t port,
                               const SharedHandle<SocketCore>& sock)
{
  std::string addr = ipaddr+":"+Util::uitos(port);
  logger->info("Pool socket for %s", addr.c_str());
  std::multimap<std::string, SharedHandle<SocketCore> >::value_type newPair
    (addr, sock);
  _socketPool.insert(newPair);
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket(const std::string& ipaddr, uint16_t port)
{
  std::string addr = ipaddr+":"+Util::uitos(port);
  std::multimap<std::string, SharedHandle<SocketCore> >::iterator i =
    _socketPool.find(addr);
  if(i == _socketPool.end()) {
    return SharedHandle<SocketCore>();
  } else {
    logger->info("Reuse socket for %s", addr.c_str());
    SharedHandle<SocketCore> s = (*i).second;
    _socketPool.erase(i);
    return s;
  }
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

} // namespace aria2
