/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "NameResolveCommand.h"
#include "DownloadEngine.h"
#include "NameResolver.h"
#include "prefs.h"
#include "message.h"
#include "util.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"
#include "UDPTrackerRequest.h"
#include "UDPTrackerClient.h"
#include "BtRegistry.h"
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

NameResolveCommand::NameResolveCommand
(cuid_t cuid, DownloadEngine* e,
 const SharedHandle<UDPTrackerRequest>& req)
  : Command(cuid),
    e_(e),
    req_(req)
{
  setStatus(Command::STATUS_ONESHOT_REALTIME);
}

NameResolveCommand::~NameResolveCommand()
{
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(resolver_);
#endif // ENABLE_ASYNC_DNS
}

bool NameResolveCommand::execute()
{
  // This is UDP tracker specific, but we need to keep this command
  // alive until force shutdown is
  // commencing. RequestGroupMan::downloadFinished() is useless here
  // at the moment.
  if(e_->isForceHaltRequested()) {
    onShutdown();
    return true;
  }
#ifdef ENABLE_ASYNC_DNS
  if(!resolver_) {
    int family = AF_INET;
    resolver_.reset(new AsyncNameResolver(family
#ifdef HAVE_ARES_ADDR_NODE
                                          , e_->getAsyncDNSServers()
#endif // HAVE_ARES_ADDR_NODE
                                          ));
  }
#endif // ENABLE_ASYNC_DNS
  std::string hostname = req_->remoteAddr;
  std::vector<std::string> res;
  if(util::isNumericHost(hostname)) {
    res.push_back(hostname);
  } else {
#ifdef ENABLE_ASYNC_DNS
    if(e_->getOption()->getAsBool(PREF_ASYNC_DNS)) {
      try {
        if(resolveHostname(hostname, resolver_)) {
          res = resolver_->getResolvedAddresses();
        } else {
          e_->addCommand(this);
          return false;
        }
      } catch(RecoverableException& e) {
        A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
      }
    } else
#endif // ENABLE_ASYNC_DNS
      {
        NameResolver resolver;
        resolver.setSocktype(SOCK_DGRAM);
        try {
          resolver.resolve(res, hostname);
        } catch(RecoverableException& e) {
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
      }
  }
  if(res.empty()) {
    onFailure();
  } else {
    onSuccess(res, e_);
  }
  return true;
}

void NameResolveCommand::onShutdown()
{
  req_->state = UDPT_STA_COMPLETE;
  req_->error = UDPT_ERR_SHUTDOWN;
}

void NameResolveCommand::onFailure()
{
  req_->state = UDPT_STA_COMPLETE;
  req_->error = UDPT_ERR_NETWORK;
}

void NameResolveCommand::onSuccess
(const std::vector<std::string>& addrs, DownloadEngine* e)
{
  req_->remoteAddr = addrs[0];
  e->getBtRegistry()->getUDPTrackerClient()->addRequest(req_);
}

#ifdef ENABLE_ASYNC_DNS

bool NameResolveCommand::resolveHostname
(const std::string& hostname,
 const SharedHandle<AsyncNameResolver>& resolver)
{
  switch(resolver->getStatus()) {
  case AsyncNameResolver::STATUS_READY:
      A2_LOG_INFO(fmt(MSG_RESOLVING_HOSTNAME,
                      getCuid(),
                      hostname.c_str()));
    resolver->resolve(hostname);
    setNameResolverCheck(resolver);
    return false;
  case AsyncNameResolver::STATUS_SUCCESS:
    A2_LOG_INFO(fmt(MSG_NAME_RESOLUTION_COMPLETE,
                    getCuid(),
                    resolver->getHostname().c_str(),
                    resolver->getResolvedAddresses().front().c_str()));
    return true;
    break;
  case AsyncNameResolver::STATUS_ERROR:
    throw DL_ABORT_EX
      (fmt(MSG_NAME_RESOLUTION_FAILED,
           getCuid(),
           hostname.c_str(),
           resolver->getError().c_str()));
  default:
    return false;
  }
}

void NameResolveCommand::setNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver)
{
  e_->addNameResolverCheck(resolver, this);
}

void NameResolveCommand::disableNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver)
{
  e_->deleteNameResolverCheck(resolver, this);
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
