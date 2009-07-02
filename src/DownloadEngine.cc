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

#include <signal.h>

#include <cstring>
#include <cerrno>
#include <algorithm>
#include <numeric>

#include "StatCalc.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
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
#include "A2STR.h"
#include "DNSCache.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "Request.h"
#include "EventPoll.h"
#include "Command.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "BtProgressInfoFile.h"
#include "DownloadContext.h"
#ifdef ENABLE_BITTORRENT
# include "BtRegistry.h"
# include "PeerStorage.h"
# include "PieceStorage.h"
# include "BtAnnounce.h"
# include "BtRuntime.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

// 0 ... running
// 1 ... stop signal detected
// 2 ... stop signal processed by DownloadEngine
// 3 ... 2nd stop signal(force shutdown) detected
// 4 ... 2nd stop signal processed by DownloadEngine
volatile sig_atomic_t globalHaltRequested = 0;

DownloadEngine::DownloadEngine(const SharedHandle<EventPoll>& eventPoll):
  _eventPoll(eventPoll),
  logger(LogFactory::getInstance()),
  _haltRequested(false),
  _noWait(false),
  _refreshInterval(DEFAULT_REFRESH_INTERVAL),
  _cookieStorage(new CookieStorage()),
  _btRegistry(new BtRegistry()),
  _dnsCache(new DNSCache())
{}

DownloadEngine::~DownloadEngine() {
  cleanQueue();
}

void DownloadEngine::cleanQueue() {
  std::for_each(commands.begin(), commands.end(), Deleter());
  commands.clear();
}

static void executeCommand(std::deque<Command*>& commands,
			   Command::STATUS statusFilter)
{
  size_t max = commands.size();
  for(size_t i = 0; i < max; ++i) {
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

void DownloadEngine::run()
{
  Time cp;
  cp.setTimeInSec(0);
  while(!commands.empty() || !_routineCommands.empty()) {
    if(cp.elapsed(_refreshInterval)) {
      _refreshInterval = DEFAULT_REFRESH_INTERVAL;
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

void DownloadEngine::waitData()
{
  struct timeval tv;
  if(_noWait) {
    tv.tv_sec = tv.tv_usec = 0;
  } else {
    tv.tv_sec = 1;
    tv.tv_usec = 0;
  }
  _eventPoll->poll(tv);
}

bool DownloadEngine::addSocketForReadCheck(const SocketHandle& socket,
					   Command* command)
{
  return _eventPoll->addEvents(socket->getSockfd(), command,
			       EventPoll::EVENT_READ);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
					      Command* command)
{
  return _eventPoll->deleteEvents(socket->getSockfd(), command,
				  EventPoll::EVENT_READ);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
					    Command* command)
{
  return _eventPoll->addEvents(socket->getSockfd(), command,
			       EventPoll::EVENT_WRITE);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
					       Command* command)
{
  return _eventPoll->deleteEvents(socket->getSockfd(), command,
				  EventPoll::EVENT_WRITE);
}

void DownloadEngine::calculateStatistics()
{
  if(!_statCalc.isNull()) {
    _statCalc->calculateStat(this);
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
  _requestGroupMan->calculateStat();
  if(globalHaltRequested == 1) {
    logger->notice(_("Shutdown sequence commencing... Press Ctrl-C again for emergency shutdown."));
    requestHalt();
    globalHaltRequested = 2;
    setNoWait(true);
    setRefreshInterval(0);
  } else if(globalHaltRequested == 3) {
    logger->notice(_("Emergency shutdown sequence commencing..."));
    _requestGroupMan->forceHalt();
    globalHaltRequested = 4;
    setNoWait(true);
    setRefreshInterval(0);
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
  return _eventPoll->addNameResolver(resolver, command);
}

bool DownloadEngine::deleteNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  return _eventPoll->deleteNameResolver(resolver, command);
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

void DownloadEngine::poolSocket(const std::string& ipaddr,
				uint16_t port,
				const SocketPoolEntry& entry)
{
  std::string addr = strconcat(ipaddr, ":", Util::uitos(port));
  logger->info("Pool socket for %s", addr.c_str());
  std::multimap<std::string, SocketPoolEntry>::value_type p(addr, entry);
  _socketPool.insert(p);

  if(_lastSocketPoolScan.elapsed(60)) {
    std::multimap<std::string, SocketPoolEntry> newPool;
    logger->debug("Scaning SocketPool and erasing timed out entry.");
    _lastSocketPoolScan.reset();
    for(std::multimap<std::string, SocketPoolEntry>::iterator i =
	  _socketPool.begin(); i != _socketPool.end(); ++i) {
      if(!(*i).second.isTimeout()) {
	newPool.insert(*i);
      }
    }
    logger->debug
      ("%lu entries removed.",
       static_cast<unsigned long>(_socketPool.size()-newPool.size()));
    _socketPool = newPool;
  }
}

void DownloadEngine::poolSocket
(const std::string& ipaddr,
 uint16_t port,
 const SharedHandle<SocketCore>& sock,
 const std::map<std::string, std::string>& options,
 time_t timeout)
{
  SocketPoolEntry e(sock, options, timeout);
  poolSocket(ipaddr, port, e);
}

void DownloadEngine::poolSocket
(const std::string& ipaddr,
 uint16_t port,
 const SharedHandle<SocketCore>& sock,
 time_t timeout)
{
  SocketPoolEntry e(sock, std::map<std::string, std::string>(), timeout);
  poolSocket(ipaddr, port, e);
}

void DownloadEngine::poolSocket(const SharedHandle<Request>& request,
				bool proxyDefined,
				const SharedHandle<SocketCore>& socket,
				time_t timeout)
{
  if(proxyDefined) {
    // If proxy is defined, then pool socket with its hostname.
    poolSocket(request->getHost(), request->getPort(), socket);
  } else {
    std::pair<std::string, uint16_t> peerInfo;
    socket->getPeerInfo(peerInfo);
    poolSocket(peerInfo.first, peerInfo.second, socket);
  }
}

void DownloadEngine::poolSocket
(const SharedHandle<Request>& request,
 bool proxyDefined,
 const SharedHandle<SocketCore>& socket,
 const std::map<std::string, std::string>& options,				
 time_t timeout)
{
  if(proxyDefined) {
    // If proxy is defined, then pool socket with its hostname.
    poolSocket(request->getHost(), request->getPort(), socket, options);
  } else {
    std::pair<std::string, uint16_t> peerInfo;
    socket->getPeerInfo(peerInfo);
    poolSocket(peerInfo.first, peerInfo.second, socket, options);
  }
}

std::multimap<std::string, DownloadEngine::SocketPoolEntry>::iterator
DownloadEngine::findSocketPoolEntry(const std::string& ipaddr, uint16_t port)
{
  std::string addr = ipaddr;
  strappend(addr, ":", Util::uitos(port));
  std::pair<std::multimap<std::string, SocketPoolEntry>::iterator,
    std::multimap<std::string, SocketPoolEntry>::iterator> range =
    _socketPool.equal_range(addr);
  for(std::multimap<std::string, SocketPoolEntry>::iterator i = range.first;
      i != range.second; ++i) {
    const SocketPoolEntry& e = (*i).second;
    if(!e.isTimeout()) {
      logger->info("Found socket for %s", addr.c_str());
      return i;
    }
  }
  return _socketPool.end();
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket(const std::string& ipaddr, uint16_t port)
{
  SharedHandle<SocketCore> s;
  std::multimap<std::string, SocketPoolEntry>::iterator i =
    findSocketPoolEntry(ipaddr, port);
  if(i != _socketPool.end()) {
    s = (*i).second.getSocket();
    _socketPool.erase(i);
  }
  return s;
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket(std::map<std::string, std::string>& options,
				const std::string& ipaddr, uint16_t port)
{
  SharedHandle<SocketCore> s;
  std::multimap<std::string, SocketPoolEntry>::iterator i =
    findSocketPoolEntry(ipaddr, port);
  if(i != _socketPool.end()) {
    s = (*i).second.getSocket();
    options = (*i).second.getOptions();
    _socketPool.erase(i);
  }
  return s;  
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(const std::deque<std::string>& ipaddrs, uint16_t port)
{
  SharedHandle<SocketCore> s;
  for(std::deque<std::string>::const_iterator i = ipaddrs.begin();
      i != ipaddrs.end(); ++i) {
    s = popPooledSocket(*i, port);
    if(!s.isNull()) {
      break;
    }
  }
  return s;
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(std::map<std::string, std::string>& options,
 const std::deque<std::string>& ipaddrs, uint16_t port)
{
  SharedHandle<SocketCore> s;
  for(std::deque<std::string>::const_iterator i = ipaddrs.begin();
      i != ipaddrs.end(); ++i) {
    s = popPooledSocket(options, *i, port);
    if(!s.isNull()) {
      break;
    }
  }
  return s;
}

DownloadEngine::SocketPoolEntry::SocketPoolEntry
(const SharedHandle<SocketCore>& socket,
 const std::map<std::string, std::string>& options,
 time_t timeout):
  _socket(socket),
  _options(options),
  _timeout(timeout) {}

DownloadEngine::SocketPoolEntry::~SocketPoolEntry() {}

bool DownloadEngine::SocketPoolEntry::isTimeout() const
{
  return _registeredTime.elapsed(_timeout);
}

cuid_t DownloadEngine::newCUID()
{
  return _cuidCounter.newID();
}

const std::string& DownloadEngine::findCachedIPAddress
(const std::string& hostname, uint16_t port) const
{
  return _dnsCache->find(hostname, port);
}

void DownloadEngine::cacheIPAddress
(const std::string& hostname, const std::string& ipaddr, uint16_t port)
{
  _dnsCache->put(hostname, ipaddr, port);
}

void DownloadEngine::markBadIPAddress
(const std::string& hostname, const std::string& ipaddr, uint16_t port)
{
  _dnsCache->markBad(hostname, ipaddr, port);
}

void DownloadEngine::removeCachedIPAddress
(const std::string& hostname, uint16_t port)
{
  _dnsCache->remove(hostname, port);
}

void DownloadEngine::setAuthConfigFactory
(const SharedHandle<AuthConfigFactory>& factory)
{
  _authConfigFactory = factory;
}

void DownloadEngine::setRefreshInterval(time_t interval)
{
  _refreshInterval = interval;
}

} // namespace aria2
