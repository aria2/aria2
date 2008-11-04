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

namespace aria2 {

InitiateConnectionCommand::InitiateConnectionCommand
(int cuid,
 const RequestHandle& req,
 RequestGroup* requestGroup,
 DownloadEngine* e):
  AbstractCommand(cuid, req, requestGroup, e)
{
  setTimeout(e->option->getAsInt(PREF_DNS_TIMEOUT));
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

InitiateConnectionCommand::~InitiateConnectionCommand() {}

bool InitiateConnectionCommand::executeInternal() {
  std::string hostname;
  SharedHandle<Request> proxyRequest = createProxyRequest();
  if(proxyRequest.isNull()) {
    hostname = req->getHost();
  } else {
    hostname = proxyRequest->getHost();
  }
  std::deque<std::string> addrs;
  std::string ipaddr = e->findCachedIPAddress(hostname);
  if(ipaddr.empty()) {
#ifdef ENABLE_ASYNC_DNS
    if(e->option->getAsBool(PREF_ASYNC_DNS)) {
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
	res.resolve(addrs, hostname);
      }
    logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
		 hostname.c_str(),
		 addrs.front().c_str());
    e->cacheIPAddress(hostname, addrs.front());
  } else {
    logger->info(MSG_DNS_CACHE_HIT, cuid, hostname.c_str(), ipaddr.c_str());
    addrs.push_back(ipaddr);
  }

  Command* command = createNextCommand(addrs, proxyRequest);
  e->commands.push_back(command);
  return true;
}

} // namespace aria2
