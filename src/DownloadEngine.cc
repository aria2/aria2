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
#include <iterator>

#include "StatCalc.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "DownloadResult.h"
#include "StatCalc.h"
#include "LogFactory.h"
#include "Logger.h"
#include "SocketCore.h"
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
#  include "BtRegistry.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_WEBSOCKET
#  include "WebSocketSessionMan.h"
#endif // ENABLE_WEBSOCKET
#include "Option.h"
#include "util_security.h"

namespace aria2 {

namespace global {

// 0 ... running
// 1 ... stop signal detected
// 2 ... stop signal processed by DownloadEngine
// 3 ... 2nd stop signal(force shutdown) detected
// 4 ... 2nd stop signal processed by DownloadEngine
// 5 ... main loop exited
volatile sig_atomic_t globalHaltRequested = 0;

} // namespace global

namespace {
constexpr auto DEFAULT_REFRESH_INTERVAL = 1_s;
} // namespace

DownloadEngine::DownloadEngine(std::unique_ptr<EventPoll> eventPoll)
    : eventPoll_(std::move(eventPoll)),
      haltRequested_(0),
      noWait_(true),
      refreshInterval_(DEFAULT_REFRESH_INTERVAL),
      lastRefresh_(Timer::zero()),
      cookieStorage_(make_unique<CookieStorage>()),
#ifdef ENABLE_BITTORRENT
      btRegistry_(make_unique<BtRegistry>()),
#endif // ENABLE_BITTORRENT
#ifdef HAVE_ARES_ADDR_NODE
      asyncDNSServers_(nullptr),
#endif // HAVE_ARES_ADDR_NODE
      dnsCache_(make_unique<DNSCache>()),
      option_(nullptr)
{
  unsigned char sessionId[20];
  util::generateRandomKey(sessionId);
  sessionId_.assign(&sessionId[0], &sessionId[sizeof(sessionId)]);
}

DownloadEngine::~DownloadEngine()
{
#ifdef HAVE_ARES_ADDR_NODE
  setAsyncDNSServers(nullptr);
#endif // HAVE_ARES_ADDR_NODE
}

namespace {
void executeCommand(std::deque<std::unique_ptr<Command>>& commands,
                    Command::STATUS statusFilter)
{
  size_t max = commands.size();
  for (size_t i = 0; i < max; ++i) {
    auto com = std::move(commands.front());
    commands.pop_front();
    if (!com->statusMatch(statusFilter)) {
      com->clearIOEvents();
      commands.push_back(std::move(com));
      continue;
    }
    com->transitStatus();
    if (com->execute()) {
      com.reset();
    }
    else {
      com->clearIOEvents();
      com.release();
    }
  }
}
} // namespace

namespace {
class GlobalHaltRequestedFinalizer {
public:
  GlobalHaltRequestedFinalizer(bool oneshot) : oneshot_(oneshot) {}
  ~GlobalHaltRequestedFinalizer()
  {
    if (!oneshot_) {
      global::globalHaltRequested = 5;
    }
  }

private:
  bool oneshot_;
};
} // namespace

int DownloadEngine::run(bool oneshot)
{
  GlobalHaltRequestedFinalizer ghrf(oneshot);
  while (!commands_.empty() || !routineCommands_.empty()) {
    if (!commands_.empty()) {
      waitData();
    }
    noWait_ = false;
    global::wallclock().reset();
    calculateStatistics();
    if (lastRefresh_.difference(global::wallclock()) + A2_DELTA_MILLIS >=
        refreshInterval_) {
      refreshInterval_ = DEFAULT_REFRESH_INTERVAL;
      lastRefresh_ = global::wallclock();
      executeCommand(commands_, Command::STATUS_ALL);
    }
    else {
      executeCommand(commands_, Command::STATUS_ACTIVE);
    }
    executeCommand(routineCommands_, Command::STATUS_ALL);
    afterEachIteration();
    if (!noWait_ && oneshot) {
      return 1;
    }
  }
  onEndOfRun();
  return 0;
}

void DownloadEngine::waitData()
{
  struct timeval tv;
  if (noWait_) {
    tv.tv_sec = tv.tv_usec = 0;
  }
  else {
    auto t =
        std::chrono::duration_cast<std::chrono::microseconds>(refreshInterval_);
    tv.tv_sec = t.count() / 1000000;
    tv.tv_usec = t.count() % 1000000;
  }
  eventPoll_->poll(tv);
}

bool DownloadEngine::addSocketForReadCheck(
    const std::shared_ptr<SocketCore>& socket, Command* command)
{
  return eventPoll_->addEvents(socket->getSockfd(), command,
                               EventPoll::EVENT_READ);
}

bool DownloadEngine::deleteSocketForReadCheck(
    const std::shared_ptr<SocketCore>& socket, Command* command)
{
  return eventPoll_->deleteEvents(socket->getSockfd(), command,
                                  EventPoll::EVENT_READ);
}

bool DownloadEngine::addSocketForWriteCheck(
    const std::shared_ptr<SocketCore>& socket, Command* command)
{
  return eventPoll_->addEvents(socket->getSockfd(), command,
                               EventPoll::EVENT_WRITE);
}

bool DownloadEngine::deleteSocketForWriteCheck(
    const std::shared_ptr<SocketCore>& socket, Command* command)
{
  return eventPoll_->deleteEvents(socket->getSockfd(), command,
                                  EventPoll::EVENT_WRITE);
}

void DownloadEngine::calculateStatistics()
{
  if (statCalc_) {
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
  if (global::globalHaltRequested == 1) {
    A2_LOG_NOTICE(_("Shutdown sequence commencing..."
                    " Press Ctrl-C again for emergency shutdown."));
    requestHalt();
    global::globalHaltRequested = 2;
    setNoWait(true);
    setRefreshInterval(std::chrono::milliseconds(0));
    return;
  }

  if (global::globalHaltRequested == 3) {
    A2_LOG_NOTICE(_("Emergency shutdown sequence commencing..."));
    requestForceHalt();
    global::globalHaltRequested = 4;
    setNoWait(true);
    setRefreshInterval(std::chrono::milliseconds(0));
    return;
  }
}

void DownloadEngine::requestHalt()
{
  haltRequested_ = std::max(haltRequested_, 1);
  requestGroupMan_->halt();
}

void DownloadEngine::requestForceHalt()
{
  haltRequested_ = std::max(haltRequested_, 2);
  requestGroupMan_->forceHalt();
}

void DownloadEngine::setStatCalc(std::unique_ptr<StatCalc> statCalc)
{
  statCalc_ = std::move(statCalc);
}

#ifdef ENABLE_ASYNC_DNS
bool DownloadEngine::addNameResolverCheck(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  return eventPoll_->addNameResolver(resolver, command);
}

bool DownloadEngine::deleteNameResolverCheck(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  return eventPoll_->deleteNameResolver(resolver, command);
}
#endif // ENABLE_ASYNC_DNS

void DownloadEngine::setNoWait(bool b) { noWait_ = b; }

void DownloadEngine::addRoutineCommand(std::unique_ptr<Command> command)
{
  routineCommands_.push_back(std::move(command));
}

void DownloadEngine::poolSocket(const std::string& key,
                                const SocketPoolEntry& entry)
{
  A2_LOG_INFO(fmt("Pool socket for %s", key.c_str()));
  std::multimap<std::string, SocketPoolEntry>::value_type p(key, entry);
  socketPool_.insert(p);
}

void DownloadEngine::evictSocketPool()
{
  if (socketPool_.empty()) {
    return;
  }

  std::multimap<std::string, SocketPoolEntry> newPool;
  A2_LOG_DEBUG("Scanning SocketPool and erasing timed out entry.");
  for (auto& elem : socketPool_) {
    if (!elem.second.isTimeout()) {
      newPool.insert(elem);
    }
  }
  A2_LOG_DEBUG(
      fmt("%lu entries removed.",
          static_cast<unsigned long>(socketPool_.size() - newPool.size())));
  socketPool_ = std::move(newPool);
}

namespace {
std::string createSockPoolKey(const std::string& host, uint16_t port,
                              const std::string& username,
                              const std::string& proxyhost, uint16_t proxyport)
{
  std::string key;
  if (!username.empty()) {
    key += util::percentEncode(username);
    key += "@";
  }
  key += fmt("%s(%u)", host.c_str(), port);
  if (!proxyhost.empty()) {
    key += fmt("/%s(%u)", proxyhost.c_str(), proxyport);
  }
  return key;
}
} // namespace

void DownloadEngine::poolSocket(const std::string& ipaddr, uint16_t port,
                                const std::string& username,
                                const std::string& proxyhost,
                                uint16_t proxyport,
                                const std::shared_ptr<SocketCore>& sock,
                                const std::string& options,
                                std::chrono::seconds timeout)
{
  SocketPoolEntry e(sock, options, std::move(timeout));
  poolSocket(createSockPoolKey(ipaddr, port, username, proxyhost, proxyport),
             e);
}

void DownloadEngine::poolSocket(const std::string& ipaddr, uint16_t port,
                                const std::string& proxyhost,
                                uint16_t proxyport,
                                const std::shared_ptr<SocketCore>& sock,
                                std::chrono::seconds timeout)
{
  SocketPoolEntry e(sock, std::move(timeout));
  poolSocket(createSockPoolKey(ipaddr, port, A2STR::NIL, proxyhost, proxyport),
             e);
}

namespace {
bool getPeerInfo(Endpoint& res, const std::shared_ptr<SocketCore>& socket)
{
  try {
    res = socket->getPeerInfo();
    return true;
  }
  catch (RecoverableException& e) {
    // socket->getPeerInfo() can fail if the socket has been
    // disconnected.
    A2_LOG_INFO_EX("Getting peer info failed. Pooling socket canceled.", e);
    return false;
  }
}
} // namespace

void DownloadEngine::poolSocket(const std::shared_ptr<Request>& request,
                                const std::shared_ptr<Request>& proxyRequest,
                                const std::shared_ptr<SocketCore>& socket,
                                std::chrono::seconds timeout)
{
  if (proxyRequest) {
    // If proxy is defined, then pool socket with its hostname.
    poolSocket(request->getHost(), request->getPort(), proxyRequest->getHost(),
               proxyRequest->getPort(), socket, std::move(timeout));
    return;
  }

  Endpoint peerInfo;
  if (getPeerInfo(peerInfo, socket)) {
    poolSocket(peerInfo.addr, peerInfo.port, A2STR::NIL, 0, socket,
               std::move(timeout));
  }
}

void DownloadEngine::poolSocket(const std::shared_ptr<Request>& request,
                                const std::string& username,
                                const std::shared_ptr<Request>& proxyRequest,
                                const std::shared_ptr<SocketCore>& socket,
                                const std::string& options,
                                std::chrono::seconds timeout)
{
  if (proxyRequest) {
    // If proxy is defined, then pool socket with its hostname.
    poolSocket(request->getHost(), request->getPort(), username,
               proxyRequest->getHost(), proxyRequest->getPort(), socket,
               options, std::move(timeout));
    return;
  }

  Endpoint peerInfo;
  if (getPeerInfo(peerInfo, socket)) {
    poolSocket(peerInfo.addr, peerInfo.port, username, A2STR::NIL, 0, socket,
               options, std::move(timeout));
  }
}

std::multimap<std::string, DownloadEngine::SocketPoolEntry>::iterator
DownloadEngine::findSocketPoolEntry(const std::string& key)
{
  std::pair<std::multimap<std::string, SocketPoolEntry>::iterator,
            std::multimap<std::string, SocketPoolEntry>::iterator>
      range = socketPool_.equal_range(key);
  for (auto i = range.first, eoi = range.second; i != eoi; ++i) {
    const SocketPoolEntry& e = (*i).second;
    // We assume that if socket is readable it means peer shutdowns
    // connection and the socket will receive EOF. So skip it.
    if (!e.isTimeout() && !e.getSocket()->isReadable(0)) {
      A2_LOG_INFO(fmt("Found socket for %s", key.c_str()));
      return i;
    }
  }
  return socketPool_.end();
}

std::shared_ptr<SocketCore>
DownloadEngine::popPooledSocket(const std::string& ipaddr, uint16_t port,
                                const std::string& proxyhost,
                                uint16_t proxyport)
{
  std::shared_ptr<SocketCore> s;
  auto i = findSocketPoolEntry(
      createSockPoolKey(ipaddr, port, A2STR::NIL, proxyhost, proxyport));
  if (i != socketPool_.end()) {
    s = (*i).second.getSocket();
    socketPool_.erase(i);
  }
  return s;
}

std::shared_ptr<SocketCore>
DownloadEngine::popPooledSocket(std::string& options, const std::string& ipaddr,
                                uint16_t port, const std::string& username,
                                const std::string& proxyhost,
                                uint16_t proxyport)
{
  std::shared_ptr<SocketCore> s;
  auto i = findSocketPoolEntry(
      createSockPoolKey(ipaddr, port, username, proxyhost, proxyport));
  if (i != socketPool_.end()) {
    s = (*i).second.getSocket();
    options = (*i).second.getOptions();
    socketPool_.erase(i);
  }
  return s;
}

std::shared_ptr<SocketCore>
DownloadEngine::popPooledSocket(const std::vector<std::string>& ipaddrs,
                                uint16_t port)
{
  std::shared_ptr<SocketCore> s;
  for (const auto& ipaddr : ipaddrs) {
    s = popPooledSocket(ipaddr, port, A2STR::NIL, 0);
    if (s) {
      break;
    }
  }
  return s;
}

std::shared_ptr<SocketCore>
DownloadEngine::popPooledSocket(std::string& options,
                                const std::vector<std::string>& ipaddrs,
                                uint16_t port, const std::string& username)
{
  std::shared_ptr<SocketCore> s;
  for (const auto& ipaddr : ipaddrs) {
    s = popPooledSocket(options, ipaddr, port, username, A2STR::NIL, 0);
    if (s) {
      break;
    }
  }
  return s;
}

DownloadEngine::SocketPoolEntry::SocketPoolEntry(
    const std::shared_ptr<SocketCore>& socket, const std::string& options,
    std::chrono::seconds timeout)
    : socket_(socket), options_(options), timeout_(std::move(timeout))
{
}

DownloadEngine::SocketPoolEntry::SocketPoolEntry(
    const std::shared_ptr<SocketCore>& socket, std::chrono::seconds timeout)
    : socket_(socket), timeout_(std::move(timeout))
{
}

DownloadEngine::SocketPoolEntry::~SocketPoolEntry() = default;

bool DownloadEngine::SocketPoolEntry::isTimeout() const
{
  return registeredTime_.difference(global::wallclock()) >= timeout_;
}

cuid_t DownloadEngine::newCUID() { return cuidCounter_.newID(); }

const std::string&
DownloadEngine::findCachedIPAddress(const std::string& hostname,
                                    uint16_t port) const
{
  return dnsCache_->find(hostname, port);
}

void DownloadEngine::cacheIPAddress(const std::string& hostname,
                                    const std::string& ipaddr, uint16_t port)
{
  dnsCache_->put(hostname, ipaddr, port);
}

void DownloadEngine::markBadIPAddress(const std::string& hostname,
                                      const std::string& ipaddr, uint16_t port)
{
  dnsCache_->markBad(hostname, ipaddr, port);
}

void DownloadEngine::removeCachedIPAddress(const std::string& hostname,
                                           uint16_t port)
{
  dnsCache_->remove(hostname, port);
}

void DownloadEngine::setAuthConfigFactory(
    std::unique_ptr<AuthConfigFactory> factory)
{
  authConfigFactory_ = std::move(factory);
}

const std::unique_ptr<AuthConfigFactory>&
DownloadEngine::getAuthConfigFactory() const
{
  return authConfigFactory_;
}

const std::unique_ptr<CookieStorage>& DownloadEngine::getCookieStorage() const
{
  return cookieStorage_;
}

void DownloadEngine::setRefreshInterval(std::chrono::milliseconds interval)
{
  refreshInterval_ = std::move(interval);
}

void DownloadEngine::addCommand(std::vector<std::unique_ptr<Command>> commands)
{
  commands_.insert(commands_.end(),
                   std::make_move_iterator(std::begin(commands)),
                   std::make_move_iterator(std::end(commands)));
}

void DownloadEngine::addCommand(std::unique_ptr<Command> command)
{
  commands_.push_back(std::move(command));
}

void DownloadEngine::setRequestGroupMan(std::unique_ptr<RequestGroupMan> rgman)
{
  requestGroupMan_ = std::move(rgman);
}

void DownloadEngine::setFileAllocationMan(
    std::unique_ptr<FileAllocationMan> faman)
{
  fileAllocationMan_ = std::move(faman);
}

void DownloadEngine::setCheckIntegrityMan(
    std::unique_ptr<CheckIntegrityMan> ciman)
{
  checkIntegrityMan_ = std::move(ciman);
}

#ifdef HAVE_ARES_ADDR_NODE
void DownloadEngine::setAsyncDNSServers(ares_addr_node* asyncDNSServers)
{
  ares_addr_node* node = asyncDNSServers_;
  while (node) {
    ares_addr_node* next = node->next;
    delete node;
    node = next;
  }
  asyncDNSServers_ = asyncDNSServers;
}
#endif // HAVE_ARES_ADDR_NODE

#ifdef ENABLE_WEBSOCKET
void DownloadEngine::setWebSocketSessionMan(
    std::unique_ptr<rpc::WebSocketSessionMan> wsman)
{
  webSocketSessionMan_ = std::move(wsman);
}
#endif // ENABLE_WEBSOCKET

bool DownloadEngine::validateToken(const std::string& token)
{
  using namespace util::security;

  if (!option_->defined(PREF_RPC_SECRET)) {
    return true;
  }

  if (!tokenHMAC_) {
    tokenHMAC_ = HMAC::createRandom();
    if (!tokenHMAC_) {
      A2_LOG_ERROR("Failed to create HMAC");
      return false;
    }
    tokenExpected_ = make_unique<HMACResult>(
        tokenHMAC_->getResult(option_->get(PREF_RPC_SECRET)));
  }

  return *tokenExpected_ == tokenHMAC_->getResult(token);
}

} // namespace aria2
