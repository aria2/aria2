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
#include "InitiateConnectionCommand.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "Logger.h"
#include "message.h"
#include "prefs.h"
#include "NameResolver.h"
#include "DNSCache.h"
#include "SocketCore.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Segment.h"
#include "a2functional.h"
#include "InitiateConnectionCommandFactory.h"

namespace aria2 {

InitiateConnectionCommand::InitiateConnectionCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 DownloadEngine* e):
  AbstractCommand(cuid, req, fileEntry, requestGroup, e)
{
  setTimeout(getOption()->getAsInt(PREF_DNS_TIMEOUT));
  // give a chance to be executed in the next loop in DownloadEngine
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

InitiateConnectionCommand::~InitiateConnectionCommand() {}

bool InitiateConnectionCommand::executeInternal() {
  std::string hostname;
  uint16_t port;
  SharedHandle<Request> proxyRequest = createProxyRequest();
  if(proxyRequest.isNull()) {
    hostname = req->getHost();
    port = req->getPort();
  } else {
    hostname = proxyRequest->getHost();
    port = proxyRequest->getPort();
  }
  std::vector<std::string> addrs;
  e->findAllCachedIPAddresses(std::back_inserter(addrs), hostname, port);
  std::string ipaddr;
  if(addrs.empty()) {
#ifdef ENABLE_ASYNC_DNS
    if(getOption()->getAsBool(PREF_ASYNC_DNS)) {
      if(!isAsyncNameResolverInitialized()) {
        initAsyncNameResolver(hostname);
      }
      if(asyncResolveHostname()) {
        addrs = getResolvedAddresses();
      } else {
        e->commands.push_back(this);
        return false;
      }
    } else
#endif // ENABLE_ASYNC_DNS
      {
        NameResolver res;
        res.setSocktype(SOCK_STREAM);
        if(e->option->getAsBool(PREF_DISABLE_IPV6)) {
          res.setFamily(AF_INET);
        }
        res.resolve(addrs, hostname);
      }
    logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
                 hostname.c_str(),
                 strjoin(addrs.begin(), addrs.end(), ", ").c_str());
    for(std::vector<std::string>::const_iterator i = addrs.begin(),
          eoi = addrs.end(); i != eoi; ++i) {
      e->cacheIPAddress(hostname, *i, port);
    }
    ipaddr = e->findCachedIPAddress(hostname, port);
  } else {
    ipaddr = addrs.front();
    logger->info(MSG_DNS_CACHE_HIT, cuid, hostname.c_str(),
                 strjoin(addrs.begin(), addrs.end(), ", ").c_str());
  }
  try {
    Command* command = createNextCommand(hostname, ipaddr, port,
                                         addrs, proxyRequest);
    e->commands.push_back(command);
    return true;
  } catch(RecoverableException& ex) {
    // Catch exception and retry another address.
    // See also AbstractCommand::checkIfConnectionEstablished

    // TODO ipaddr might not be used if pooled sockt was found.
    e->markBadIPAddress(hostname, ipaddr, port);
    if(!e->findCachedIPAddress(hostname, port).empty()) {
      logger->info(EX_EXCEPTION_CAUGHT, ex);
      logger->info(MSG_CONNECT_FAILED_AND_RETRY, cuid, ipaddr.c_str(), port);
      Command* command =
        InitiateConnectionCommandFactory::createInitiateConnectionCommand
        (cuid, req, _fileEntry, _requestGroup, e);
      e->setNoWait(true);
      e->commands.push_back(command);
      return true;
    }
    e->removeCachedIPAddress(hostname, port);
    throw;
  }
}

} // namespace aria2
