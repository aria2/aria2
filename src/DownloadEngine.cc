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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "Socket.h"
#include "util.h"
#include "a2functional.h"
#include "DlAbortEx.h"
#include "ServerStatMan.h"
#include "CookieStorage.h"
#include "A2STR.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "Request.h"
#include "EventPoll.h"
#include "Command.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "BtProgressInfoFile.h"
#include "DownloadContext.h"
#include "fmt.h"
#include "wallclock.h"
#ifdef ENABLE_BITTORRENT
# include "BtRegistry.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace global {

// 0 ... running
// 1 ... stop signal detected
// 2 ... stop signal processed by DownloadEngine
// 3 ... 2nd stop signal(force shutdown) detected
// 4 ... 2nd stop signal processed by DownloadEngine
volatile sig_atomic_t globalHaltRequested = 0;

} // namespace global

DownloadEngine::DownloadEngine(const SharedHandle<EventPoll>& eventPoll)
  : eventPoll_(eventPoll),
    haltRequested_(false),
    noWait_(false),
    refreshInterval_(DEFAULT_REFRESH_INTERVAL),
    cookieStorage_(new CookieStorage()),
#ifdef ENABLE_BITTORRENT
    btRegistry_(new BtRegistry()),
#endif // ENABLE_BITTORRENT
#ifdef HAVE_ARES_ADDR_NODE
    asyncDNSServers_(0),
#endif // HAVE_ARES_ADDR_NODE
    dnsCache_(new DNSCache())
{
  unsigned char sessionId[20];
  util::generateRandomKey(sessionId);
  sessionId_.assign(&sessionId[0], & sessionId[sizeof(sessionId)]);
}

DownloadEngine::~DownloadEngine() {
  cleanQueue();
#ifdef HAVE_ARES_ADDR_NODE
  setAsyncDNSServers(0);
#endif // HAVE_ARES_ADDR_NODE
}

void DownloadEngine::cleanQueue() {
  std::for_each(commands_.begin(), commands_.end(), Deleter());
  commands_.clear();
}

namespace {
void executeCommand(std::deque<Command*>& commands,
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
} // namespace

void DownloadEngine::run()
{
  Timer cp;
  cp.reset(0);
  while(!commands_.empty() || !routineCommands_.empty()) {
    global::wallclock().reset();
    calculateStatistics();
    if(cp.differenceInMillis(global::wallclock())+A2_DELTA_MILLIS >=
       refreshInterval_) {
      refreshInterval_ = DEFAULT_REFRESH_INTERVAL;
      cp = global::wallclock();
      executeCommand(commands_, Command::STATUS_ALL);
    } else {
      executeCommand(commands_, Command::STATUS_ACTIVE);
    }
    executeCommand(routineCommands_, Command::STATUS_ALL);
    afterEachIteration();
    if(!commands_.empty()) {
      waitData();
    }
    noWait_ = false;
  }
  onEndOfRun();
}

void DownloadEngine::waitData()
{
  struct timeval tv;
  if(noWait_) {
    tv.tv_sec = tv.tv_usec = 0;
  } else {
    lldiv_t qr = lldiv(refreshInterval_*1000, 1000000);
    tv.tv_sec = qr.quot;
    tv.tv_usec = qr.rem;
  }
  eventPoll_->poll(tv);
}

bool DownloadEngine::addSocketForReadCheck(const SocketHandle& socket,
                                           Command* command)
{
  return eventPoll_->addEvents(socket->getSockfd(), command,
                               EventPoll::EVENT_READ);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
                                              Command* command)
{
  return eventPoll_->deleteEvents(socket->getSockfd(), command,
                                  EventPoll::EVENT_READ);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
                                            Command* command)
{
  return eventPoll_->addEvents(socket->getSockfd(), command,
                               EventPoll::EVENT_WRITE);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
                                               Command* command)
{
  return eventPoll_->deleteEvents(socket->getSockfd(), command,
                                  EventPoll::EVENT_WRITE);
}

void DownloadEngine::calculateStatistics()
{
  if(statCalc_) {
    statCalc_->calculateStat(this);
  }
}

void DownloadEngine::onEndOfRun()
{
  requestGroupMan_->removeStoppedGroup(this);
  requestGroupMan_->closeFile();
  requestGroupMan_->save();
}

void DownloadEngine::afterEachIteration()
{
  requestGroupMan_->calculateStat();
  if(global::globalHaltRequested == 1) {
    A2_LOG_NOTICE(_("Shutdown sequence commencing..."
                    " Press Ctrl-C again for emergency shutdown."));
    requestHalt();
    global::globalHaltRequested = 2;
    setNoWait(true);
    setRefreshInterval(0);
  } else if(global::globalHaltRequested == 3) {
    A2_LOG_NOTICE(_("Emergency shutdown sequence commencing..."));
    requestForceHalt();
    global::globalHaltRequested = 4;
    setNoWait(true);
    setRefreshInterval(0);
  }
}

void DownloadEngine::requestHalt()
{
  haltRequested_ = true;
  requestGroupMan_->halt();
}

void DownloadEngine::requestForceHalt()
{
  haltRequested_ = true;
  requestGroupMan_->forceHalt();
}

void DownloadEngine::setStatCalc(const StatCalcHandle& statCalc)
{
  statCalc_ = statCalc;
}

#ifdef ENABLE_ASYNC_DNS
bool DownloadEngine::addNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  return eventPoll_->addNameResolver(resolver, command);
}

bool DownloadEngine::deleteNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  return eventPoll_->deleteNameResolver(resolver, command);
}
#endif // ENABLE_ASYNC_DNS

void DownloadEngine::setNoWait(bool b)
{
  noWait_ = b;
}

void DownloadEngine::addRoutineCommand(Command* command)
{
  routineCommands_.push_back(command);
}

void DownloadEngine::poolSocket(const std::string& key,
                                const SocketPoolEntry& entry)
{
  A2_LOG_INFO(fmt("Pool socket for %s", key.c_str()));
  std::multimap<std::string, SocketPoolEntry>::value_type p(key, entry);
  socketPool_.insert(p);

  if(lastSocketPoolScan_.difference(global::wallclock()) >= 60) {
    std::multimap<std::string, SocketPoolEntry> newPool;
    A2_LOG_DEBUG("Scaning SocketPool and erasing timed out entry.");
    lastSocketPoolScan_ = global::wallclock();
    for(std::multimap<std::string, SocketPoolEntry>::iterator i =
          socketPool_.begin(), eoi = socketPool_.end(); i != eoi; ++i) {
      if(!(*i).second.isTimeout()) {
        newPool.insert(*i);
      }
    }
    A2_LOG_DEBUG(fmt("%lu entries removed.",
                     static_cast<unsigned long>
                     (socketPool_.size()-newPool.size())));
    socketPool_ = newPool;
  }
}

namespace {
std::string createSockPoolKey
(const std::string& host, uint16_t port,
 const std::string& username,
 const std::string& proxyhost, uint16_t proxyport)
{
  std::string key;
  if(!username.empty()) {
    key += util::percentEncode(username);
    key += "@";
  }
  key += fmt("%s(%u)", host.c_str(), port);
  if(!proxyhost.empty()) {
    key += fmt("/%s(%u)", proxyhost.c_str(), proxyport);
  }
  return key;
}
} // namespace

void DownloadEngine::poolSocket
(const std::string& ipaddr,
 uint16_t port,
 const std::string& username,
 const std::string& proxyhost,
 uint16_t proxyport,
 const SharedHandle<SocketCore>& sock,
 const std::map<std::string, std::string>& options,
 time_t timeout)
{
  SocketPoolEntry e(sock, options, timeout);
  poolSocket(createSockPoolKey(ipaddr, port, username, proxyhost, proxyport),e);
}

void DownloadEngine::poolSocket
(const std::string& ipaddr,
 uint16_t port,
 const std::string& proxyhost,
 uint16_t proxyport,
 const SharedHandle<SocketCore>& sock,
 time_t timeout)
{
  SocketPoolEntry e(sock, timeout);
  poolSocket(createSockPoolKey(ipaddr, port, A2STR::NIL,proxyhost,proxyport),e);
}

namespace {
bool getPeerInfo(std::pair<std::string, uint16_t>& res,
                 const SharedHandle<SocketCore>& socket)
{
  try {
    socket->getPeerInfo(res);
    return true;
  } catch(RecoverableException& e) {
    // socket->getPeerInfo() can fail if the socket has been
    // disconnected.
    A2_LOG_INFO_EX("Getting peer info failed. Pooling socket canceled.", e);
    return false;
  }
}
} // namespace

void DownloadEngine::poolSocket(const SharedHandle<Request>& request,
                                const SharedHandle<Request>& proxyRequest,
                                const SharedHandle<SocketCore>& socket,
                                time_t timeout)
{
  if(!proxyRequest) {
    std::pair<std::string, uint16_t> peerInfo;
    if(getPeerInfo(peerInfo, socket)) {
      poolSocket(peerInfo.first, peerInfo.second,
                 A2STR::NIL, 0, socket, timeout);
    }
  } else {
    // If proxy is defined, then pool socket with its hostname.
    poolSocket(request->getHost(), request->getPort(),
               proxyRequest->getHost(), proxyRequest->getPort(),
               socket, timeout);
  }
}

void DownloadEngine::poolSocket
(const SharedHandle<Request>& request,
 const std::string& username,
 const SharedHandle<Request>& proxyRequest,
 const SharedHandle<SocketCore>& socket,
 const std::map<std::string, std::string>& options,
 time_t timeout)
{
  if(!proxyRequest) {
    std::pair<std::string, uint16_t> peerInfo;
    if(getPeerInfo(peerInfo, socket)) {
      poolSocket(peerInfo.first, peerInfo.second, username,
                 A2STR::NIL, 0, socket, options, timeout);
    }
  } else {
    // If proxy is defined, then pool socket with its hostname.
    poolSocket(request->getHost(), request->getPort(), username,
               proxyRequest->getHost(), proxyRequest->getPort(),
               socket, options, timeout);
  }
}

std::multimap<std::string, DownloadEngine::SocketPoolEntry>::iterator
DownloadEngine::findSocketPoolEntry(const std::string& key)
{
  std::pair<std::multimap<std::string, SocketPoolEntry>::iterator,
    std::multimap<std::string, SocketPoolEntry>::iterator> range =
    socketPool_.equal_range(key);
  for(std::multimap<std::string, SocketPoolEntry>::iterator i =
        range.first, eoi = range.second; i != eoi; ++i) {
    const SocketPoolEntry& e = (*i).second;
    // We assume that if socket is readable it means peer shutdowns
    // connection and the socket will receive EOF. So skip it.
    if(!e.isTimeout() && !e.getSocket()->isReadable(0)) {
      A2_LOG_INFO(fmt("Found socket for %s", key.c_str()));
      return i;
    }
  }
  return socketPool_.end();
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(const std::string& ipaddr, uint16_t port,
 const std::string& proxyhost, uint16_t proxyport)
{
  SharedHandle<SocketCore> s;
  std::multimap<std::string, SocketPoolEntry>::iterator i =
    findSocketPoolEntry
    (createSockPoolKey(ipaddr, port, A2STR::NIL, proxyhost, proxyport));
  if(i != socketPool_.end()) {
    s = (*i).second.getSocket();
    socketPool_.erase(i);
  }
  return s;
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(std::map<std::string, std::string>& options,
 const std::string& ipaddr, uint16_t port,
 const std::string& username,
 const std::string& proxyhost, uint16_t proxyport)
{
  SharedHandle<SocketCore> s;
  std::multimap<std::string, SocketPoolEntry>::iterator i =
    findSocketPoolEntry
    (createSockPoolKey(ipaddr, port, username, proxyhost, proxyport));
  if(i != socketPool_.end()) {
    s = (*i).second.getSocket();
    options = (*i).second.getOptions();
    socketPool_.erase(i);
  }
  return s;
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(const std::vector<std::string>& ipaddrs, uint16_t port)
{
  SharedHandle<SocketCore> s;
  for(std::vector<std::string>::const_iterator i = ipaddrs.begin(),
        eoi = ipaddrs.end(); i != eoi; ++i) {
    s = popPooledSocket(*i, port, A2STR::NIL, 0);
    if(s) {
      break;
    }
  }
  return s;
}

SharedHandle<SocketCore>
DownloadEngine::popPooledSocket
(std::map<std::string, std::string>& options,
 const std::vector<std::string>& ipaddrs, uint16_t port,
 const std::string& username)
{
  SharedHandle<SocketCore> s;
  for(std::vector<std::string>::const_iterator i = ipaddrs.begin(),
        eoi = ipaddrs.end(); i != eoi; ++i) {
    s = popPooledSocket(options, *i, port, username, A2STR::NIL, 0);
    if(s) {
      break;
    }
  }
  return s;
}

DownloadEngine::SocketPoolEntry::SocketPoolEntry
(const SharedHandle<SocketCore>& socket,
 const std::map<std::string, std::string>& options,
 time_t timeout):
  socket_(socket),
  options_(options),
  timeout_(timeout) {}

DownloadEngine::SocketPoolEntry::SocketPoolEntry
(const SharedHandle<SocketCore>& socket, time_t timeout):
  socket_(socket),
  timeout_(timeout) {}

DownloadEngine::SocketPoolEntry::~SocketPoolEntry() {}

bool DownloadEngine::SocketPoolEntry::isTimeout() const
{
  return registeredTime_.difference(global::wallclock()) >= timeout_;
}

cuid_t DownloadEngine::newCUID()
{
  return cuidCounter_.newID();
}

const std::string& DownloadEngine::findCachedIPAddress
(const std::string& hostname, uint16_t port) const
{
  return dnsCache_->find(hostname, port);
}

void DownloadEngine::cacheIPAddress
(const std::string& hostname, const std::string& ipaddr, uint16_t port)
{
  dnsCache_->put(hostname, ipaddr, port);
}

void DownloadEngine::markBadIPAddress
(const std::string& hostname, const std::string& ipaddr, uint16_t port)
{
  dnsCache_->markBad(hostname, ipaddr, port);
}

void DownloadEngine::removeCachedIPAddress
(const std::string& hostname, uint16_t port)
{
  dnsCache_->remove(hostname, port);
}

void DownloadEngine::setAuthConfigFactory
(const SharedHandle<AuthConfigFactory>& factory)
{
  authConfigFactory_ = factory;
}

void DownloadEngine::setRefreshInterval(int64_t interval)
{
  refreshInterval_ = std::min(static_cast<int64_t>(999), interval);
}

void DownloadEngine::addCommand(const std::vector<Command*>& commands)
{
  commands_.insert(commands_.end(), commands.begin(), commands.end());
}

void DownloadEngine::addCommand(Command* command)
{
  commands_.push_back(command);
}

void DownloadEngine::setRequestGroupMan
(const SharedHandle<RequestGroupMan>& rgman)
{
  requestGroupMan_ = rgman;
}

void DownloadEngine::setFileAllocationMan
(const SharedHandle<FileAllocationMan>& faman)
{
  fileAllocationMan_ = faman;
}

void DownloadEngine::setCheckIntegrityMan
(const SharedHandle<CheckIntegrityMan>& ciman)
{
  checkIntegrityMan_ = ciman;
}

#ifdef HAVE_ARES_ADDR_NODE
void DownloadEngine::setAsyncDNSServers(ares_addr_node* asyncDNSServers)
{
  ares_addr_node* node = asyncDNSServers_;
  while(node) {
    ares_addr_node* next = node->next;
    delete node;
    node = next;
  }
  asyncDNSServers_ = asyncDNSServers;
}
#endif // HAVE_ARES_ADDR_NODE

} // namespace aria2
