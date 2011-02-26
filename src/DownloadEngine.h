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
#ifndef D_DOWNLOAD_ENGINE_H
#define D_DOWNLOAD_ENGINE_H

#include "common.h"

#include <string>
#include <deque>
#include <map>
#include <vector>

#include "SharedHandle.h"
#include "a2netcompat.h"
#include "TimerA2.h"
#include "a2io.h"
#include "CUIDCounter.h"
#include "FileAllocationMan.h"
#include "CheckIntegrityMan.h"
#include "DNSCache.h"
#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class Option;
class RequestGroupMan;
class StatCalc;
class SocketCore;
class CookieStorage;
class AuthConfigFactory;
class Request;
class EventPoll;
class Command;
#ifdef ENABLE_BITTORRENT
class BtRegistry;
#endif // ENABLE_BITTORRENT

class DownloadEngine {
private:
  void waitData();

  std::string sessionId_;

  SharedHandle<EventPoll> eventPoll_;

  SharedHandle<StatCalc> statCalc_;

  bool haltRequested_;

  class SocketPoolEntry {
  private:
    SharedHandle<SocketCore> socket_;

    std::map<std::string, std::string> options_;

    time_t timeout_;

    Timer registeredTime_;
  public:
    SocketPoolEntry(const SharedHandle<SocketCore>& socket,
                    const std::map<std::string, std::string>& option,
                    time_t timeout);

    SocketPoolEntry(const SharedHandle<SocketCore>& socket,
                    time_t timeout);

    ~SocketPoolEntry();

    bool isTimeout() const;

    const SharedHandle<SocketCore>& getSocket() const
    {
      return socket_;
    }

    const std::map<std::string, std::string>& getOptions() const
    {
      return options_;
    }
  };

  // key = IP address:port, value = SocketPoolEntry
  std::multimap<std::string, SocketPoolEntry> socketPool_;
 
  Timer lastSocketPoolScan_;

  bool noWait_;

  static const int64_t DEFAULT_REFRESH_INTERVAL = 1000;

  // Milliseconds
  int64_t refreshInterval_;

  std::deque<Command*> routineCommands_;

  SharedHandle<CookieStorage> cookieStorage_;

#ifdef ENABLE_BITTORRENT
  SharedHandle<BtRegistry> btRegistry_;
#endif // ENABLE_BITTORRENT

  CUIDCounter cuidCounter_;

#ifdef HAVE_ARES_ADDR_NODE
  ares_addr_node* asyncDNSServers_;
#endif // HAVE_ARES_ADDR_NODE

  SharedHandle<DNSCache> dnsCache_;

  SharedHandle<AuthConfigFactory> authConfigFactory_;

  /**
   * Delegates to StatCalc
   */
  void calculateStatistics();

  void onEndOfRun();

  void afterEachIteration();
  
  void poolSocket(const std::string& key, const SocketPoolEntry& entry);

  std::multimap<std::string, SocketPoolEntry>::iterator
  findSocketPoolEntry(const std::string& key);

  std::deque<Command*> commands_;
  SharedHandle<RequestGroupMan> requestGroupMan_;
  SharedHandle<FileAllocationMan> fileAllocationMan_;
  SharedHandle<CheckIntegrityMan> checkIntegrityMan_;
  Option* option_;
public:  
  DownloadEngine(const SharedHandle<EventPoll>& eventPoll);

  ~DownloadEngine();

  void run();

  void cleanQueue();

  bool addSocketForReadCheck(const SharedHandle<SocketCore>& socket,
                             Command* command);
  bool deleteSocketForReadCheck(const SharedHandle<SocketCore>& socket,
                                Command* command);
  bool addSocketForWriteCheck(const SharedHandle<SocketCore>& socket,
                              Command* command);
  bool deleteSocketForWriteCheck(const SharedHandle<SocketCore>& socket,
                                 Command* command);

#ifdef ENABLE_ASYNC_DNS

  bool addNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver,
                            Command* command);
  bool deleteNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver,
                               Command* command);
#endif // ENABLE_ASYNC_DNS

  void addCommand(const std::vector<Command*>& commands);

  void addCommand(Command* command);

  const SharedHandle<RequestGroupMan>& getRequestGroupMan() const
  {
    return requestGroupMan_;
  }

  void setRequestGroupMan(const SharedHandle<RequestGroupMan>& rgman);

  const SharedHandle<FileAllocationMan>& getFileAllocationMan() const
  {
    return fileAllocationMan_;
  }

  void setFileAllocationMan(const SharedHandle<FileAllocationMan>& faman);

  const SharedHandle<CheckIntegrityMan>& getCheckIntegrityMan() const
  {
    return checkIntegrityMan_;
  }

  void setCheckIntegrityMan(const SharedHandle<CheckIntegrityMan>& ciman);

  Option* getOption() const
  {
    return option_;
  }

  void setOption(Option* op)
  {
    option_ = op;
  }

  void setStatCalc(const SharedHandle<StatCalc>& statCalc);

  bool isHaltRequested() const
  {
    return haltRequested_;
  }

  void requestHalt();

  void requestForceHalt();

  void setNoWait(bool b);

  void addRoutineCommand(Command* command);

  void poolSocket(const std::string& ipaddr, uint16_t port,
                  const std::string& username,
                  const std::string& proxyhost, uint16_t proxyport,
                  const SharedHandle<SocketCore>& sock,
                  const std::map<std::string, std::string>& options,
                  time_t timeout = 15);

  void poolSocket(const SharedHandle<Request>& request,
                  const std::string& username,
                  const SharedHandle<Request>& proxyRequest,
                  const SharedHandle<SocketCore>& socket,
                  const std::map<std::string, std::string>& options,
                  time_t timeout = 15);
    
  void poolSocket(const std::string& ipaddr, uint16_t port,
                  const std::string& proxyhost, uint16_t proxyport,
                  const SharedHandle<SocketCore>& sock,
                  time_t timeout = 15);

  void poolSocket(const SharedHandle<Request>& request,
                  const SharedHandle<Request>& proxyRequest,
                  const SharedHandle<SocketCore>& socket,
                  time_t timeout = 15);

  SharedHandle<SocketCore> popPooledSocket
  (const std::string& ipaddr,
   uint16_t port,
   const std::string& proxyhost, uint16_t proxyport);

  SharedHandle<SocketCore> popPooledSocket
  (std::map<std::string, std::string>& options,
   const std::string& ipaddr,
   uint16_t port,
   const std::string& username,
   const std::string& proxyhost, uint16_t proxyport);

  SharedHandle<SocketCore>
  popPooledSocket
  (const std::vector<std::string>& ipaddrs, uint16_t port);

  SharedHandle<SocketCore>
  popPooledSocket
  (std::map<std::string, std::string>& options,
   const std::vector<std::string>& ipaddrs,
   uint16_t port,
   const std::string& username);

  const SharedHandle<CookieStorage>& getCookieStorage() const
  {
    return cookieStorage_;
  }

#ifdef ENABLE_BITTORRENT
  const SharedHandle<BtRegistry>& getBtRegistry() const
  {
    return btRegistry_;
  }
#endif // ENABLE_BITTORRENT

  cuid_t newCUID();

  const std::string& findCachedIPAddress
  (const std::string& hostname, uint16_t port) const;

  template<typename OutputIterator>
  void findAllCachedIPAddresses
  (OutputIterator out, const std::string& hostname, uint16_t port) const
  {
    dnsCache_->findAll(out, hostname, port);
  }

  void cacheIPAddress
  (const std::string& hostname, const std::string& ipaddr, uint16_t port);

  void markBadIPAddress
  (const std::string& hostname, const std::string& ipaddr, uint16_t port);

  void removeCachedIPAddress(const std::string& hostname, uint16_t port);

  void setAuthConfigFactory(const SharedHandle<AuthConfigFactory>& factory);

  const SharedHandle<AuthConfigFactory>& getAuthConfigFactory() const
  {
    return authConfigFactory_;
  }

  void setRefreshInterval(int64_t interval);

  const std::string getSessionId() const
  {
    return sessionId_;
  }

#ifdef HAVE_ARES_ADDR_NODE
  void setAsyncDNSServers(ares_addr_node* asyncDNSServers);

  ares_addr_node* getAsyncDNSServers() const
  {
    return asyncDNSServers_;
  }
#endif // HAVE_ARES_ADDR_NODE
};

typedef SharedHandle<DownloadEngine> DownloadEngineHandle;

} // namespace aria2

#endif // D_DOWNLOAD_ENGINE_H

