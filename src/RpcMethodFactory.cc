/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "RpcMethodFactory.h"
#include "RpcMethodImpl.h"
#include "OptionParser.h"
#include "OptionHandler.h"

namespace aria2 {

namespace rpc {

namespace {
std::shared_ptr<RpcMethod> getNoSuchMethod()
{
  static std::shared_ptr<RpcMethod> m(new NoSuchMethodRpcMethod());
  return m;
}
} // namespace

namespace {
std::shared_ptr<RpcMethod>
createMethod(const std::string& methodName)
{
  if(methodName == AddUriRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new AddUriRpcMethod());
#ifdef ENABLE_BITTORRENT
  } else if(methodName == AddTorrentRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new AddTorrentRpcMethod());
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  }
  else if(methodName == AddMetalinkRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new AddMetalinkRpcMethod());
#endif // ENABLE_METALINK
  }
  else if(methodName == RemoveRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new RemoveRpcMethod());
  } else if(methodName == PauseRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new PauseRpcMethod());
  } else if(methodName == ForcePauseRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ForcePauseRpcMethod());
  } else if(methodName == PauseAllRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new PauseAllRpcMethod());
  } else if(methodName == ForcePauseAllRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ForcePauseAllRpcMethod());
  } else if(methodName == UnpauseRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new UnpauseRpcMethod());
  } else if(methodName == UnpauseAllRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new UnpauseAllRpcMethod());
  } else if(methodName == ForceRemoveRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ForceRemoveRpcMethod());
  } else if(methodName == ChangePositionRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ChangePositionRpcMethod());
  } else if(methodName == TellStatusRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new TellStatusRpcMethod());
  } else if(methodName == GetUrisRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetUrisRpcMethod());
  } else if(methodName == GetFilesRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetFilesRpcMethod());
#ifdef ENABLE_BITTORRENT
  }
  else if(methodName == GetPeersRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetPeersRpcMethod());
#endif // ENABLE_BITTORRENT
  } else if(methodName == GetServersRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetServersRpcMethod());
  } else if(methodName == TellActiveRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new TellActiveRpcMethod());
  } else if(methodName == TellWaitingRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new TellWaitingRpcMethod());
  } else if(methodName == TellStoppedRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new TellStoppedRpcMethod());
  } else if(methodName == GetOptionRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetOptionRpcMethod());
  } else if(methodName == ChangeUriRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ChangeUriRpcMethod());
  } else if(methodName == ChangeOptionRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ChangeOptionRpcMethod());
  } else if(methodName == GetGlobalOptionRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetGlobalOptionRpcMethod());
  } else if(methodName == ChangeGlobalOptionRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ChangeGlobalOptionRpcMethod());
  } else if(methodName == PurgeDownloadResultRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new PurgeDownloadResultRpcMethod());
  } else if(methodName == RemoveDownloadResultRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new RemoveDownloadResultRpcMethod());
  } else if(methodName == GetVersionRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetVersionRpcMethod());
  } else if(methodName == GetSessionInfoRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetSessionInfoRpcMethod());
  } else if(methodName == ShutdownRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ShutdownRpcMethod());
  } else if(methodName == ForceShutdownRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new ForceShutdownRpcMethod());
  } else if(methodName == GetGlobalStatRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new GetGlobalStatRpcMethod());
  } else if(methodName == SystemMulticallRpcMethod::getMethodName()) {
    return std::shared_ptr<RpcMethod>(new SystemMulticallRpcMethod());
  } else {
    return nullptr;
  }
}
} // namespace

std::map<std::string, std::shared_ptr<RpcMethod> > RpcMethodFactory::cache_;

std::shared_ptr<RpcMethod>
RpcMethodFactory::create(const std::string& methodName)
{
  std::map<std::string, std::shared_ptr<RpcMethod> >::const_iterator itr =
    cache_.find(methodName);
  if(itr == cache_.end()) {
    std::shared_ptr<RpcMethod> m = createMethod(methodName);
    if(m) {
      cache_.insert(std::make_pair(methodName, m));
      return m;
    } else {
      return getNoSuchMethod();
    }
  } else {
    return (*itr).second;
  }
}

} // namespace rpc

} // namespace aria2
