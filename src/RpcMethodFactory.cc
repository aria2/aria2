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
SharedHandle<RpcMethod> getNoSuchMethod()
{
  static SharedHandle<RpcMethod> m(new NoSuchMethodRpcMethod());
  return m;
}
} // namespace

namespace {
SharedHandle<RpcMethod>
createMethod(const std::string& methodName)
{
  if(methodName == AddUriRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new AddUriRpcMethod());
#ifdef ENABLE_BITTORRENT
  } else if(methodName == AddTorrentRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new AddTorrentRpcMethod());
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  }
  else if(methodName == AddMetalinkRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new AddMetalinkRpcMethod());
#endif // ENABLE_METALINK
  }
  else if(methodName == RemoveRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new RemoveRpcMethod());
  } else if(methodName == PauseRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new PauseRpcMethod());
  } else if(methodName == ForcePauseRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ForcePauseRpcMethod());
  } else if(methodName == PauseAllRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new PauseAllRpcMethod());
  } else if(methodName == ForcePauseAllRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ForcePauseAllRpcMethod());
  } else if(methodName == UnpauseRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new UnpauseRpcMethod());
  } else if(methodName == UnpauseAllRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new UnpauseAllRpcMethod());
  } else if(methodName == ForceRemoveRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ForceRemoveRpcMethod());
  } else if(methodName == ChangePositionRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ChangePositionRpcMethod());
  } else if(methodName == TellStatusRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new TellStatusRpcMethod());
  } else if(methodName == GetUrisRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetUrisRpcMethod());
  } else if(methodName == GetFilesRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetFilesRpcMethod());
#ifdef ENABLE_BITTORRENT
  }
  else if(methodName == GetPeersRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetPeersRpcMethod());
#endif // ENABLE_BITTORRENT
  } else if(methodName == GetServersRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetServersRpcMethod());
  } else if(methodName == TellActiveRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new TellActiveRpcMethod());
  } else if(methodName == TellWaitingRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new TellWaitingRpcMethod());
  } else if(methodName == TellStoppedRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new TellStoppedRpcMethod());
  } else if(methodName == GetOptionRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetOptionRpcMethod());
  } else if(methodName == ChangeUriRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ChangeUriRpcMethod());
  } else if(methodName == ChangeOptionRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ChangeOptionRpcMethod());
  } else if(methodName == GetGlobalOptionRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetGlobalOptionRpcMethod());
  } else if(methodName == ChangeGlobalOptionRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ChangeGlobalOptionRpcMethod());
  } else if(methodName == PurgeDownloadResultRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new PurgeDownloadResultRpcMethod());
  } else if(methodName == RemoveDownloadResultRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new RemoveDownloadResultRpcMethod());
  } else if(methodName == GetVersionRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetVersionRpcMethod());
  } else if(methodName == GetSessionInfoRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetSessionInfoRpcMethod());
  } else if(methodName == ShutdownRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ShutdownRpcMethod());
  } else if(methodName == ForceShutdownRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new ForceShutdownRpcMethod());
  } else if(methodName == GetGlobalStatRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new GetGlobalStatRpcMethod());
  } else if(methodName == SystemMulticallRpcMethod::getMethodName()) {
    return SharedHandle<RpcMethod>(new SystemMulticallRpcMethod());
  } else {
    return SharedHandle<RpcMethod>();
  }
}
} // namespace

std::map<std::string, SharedHandle<RpcMethod> > RpcMethodFactory::cache_;

SharedHandle<RpcMethod>
RpcMethodFactory::create(const std::string& methodName)
{
  std::map<std::string, SharedHandle<RpcMethod> >::const_iterator itr =
    cache_.find(methodName);
  if(itr == cache_.end()) {
    SharedHandle<RpcMethod> m = createMethod(methodName);
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
