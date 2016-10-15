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
std::map<std::string, std::unique_ptr<RpcMethod>> cache;
} // namespace

namespace {
std::unique_ptr<RpcMethod> noSuchRpcMethod;
} // namespace

namespace {
std::vector<std::string> rpcMethodNames = {
    "aria2.addUri",
#ifdef ENABLE_BITTORRENT
    "aria2.addTorrent",
    "aria2.getPeers",
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    "aria2.addMetalink",
#endif // ENABLE_METALINK
    "aria2.remove",
    "aria2.pause",
    "aria2.forcePause",
    "aria2.pauseAll",
    "aria2.forcePauseAll",
    "aria2.unpause",
    "aria2.unpauseAll",
    "aria2.forceRemove",
    "aria2.changePosition",
    "aria2.tellStatus",
    "aria2.getUris",
    "aria2.getFiles",
    "aria2.getServers",
    "aria2.tellActive",
    "aria2.tellWaiting",
    "aria2.tellStopped",
    "aria2.getOption",
    "aria2.changeUri",
    "aria2.changeOption",
    "aria2.getGlobalOption",
    "aria2.changeGlobalOption",
    "aria2.purgeDownloadResult",
    "aria2.removeDownloadResult",
    "aria2.getVersion",
    "aria2.getSessionInfo",
    "aria2.shutdown",
    "aria2.forceShutdown",
    "aria2.getGlobalStat",
    "aria2.saveSession",
    "system.multicall",
    "system.listMethods",
    "system.listNotifications",
};
} // namespace

const std::vector<std::string>& allMethodNames() { return rpcMethodNames; }

namespace {
std::vector<std::string> rpcNotificationsNames = {
    "aria2.onDownloadStart",      "aria2.onDownloadPause",
    "aria2.onDownloadStop",       "aria2.onDownloadComplete",
    "aria2.onDownloadError",
#ifdef ENABLE_BITTORRENT
    "aria2.onBtDownloadComplete",
#endif // ENABLE_BITTORRENT
};
} // namespace

const std::vector<std::string>& allNotificationsNames()
{
  return rpcNotificationsNames;
}

namespace {
std::unique_ptr<RpcMethod> createMethod(const std::string& methodName)
{
  if (methodName == AddUriRpcMethod::getMethodName()) {
    return make_unique<AddUriRpcMethod>();
  }

#ifdef ENABLE_BITTORRENT
  if (methodName == AddTorrentRpcMethod::getMethodName()) {
    return make_unique<AddTorrentRpcMethod>();
  }

  if (methodName == GetPeersRpcMethod::getMethodName()) {
    return make_unique<GetPeersRpcMethod>();
  }
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
  if (methodName == AddMetalinkRpcMethod::getMethodName()) {
    return make_unique<AddMetalinkRpcMethod>();
  }
#endif // ENABLE_METALINK

  if (methodName == RemoveRpcMethod::getMethodName()) {
    return make_unique<RemoveRpcMethod>();
  }

  if (methodName == PauseRpcMethod::getMethodName()) {
    return make_unique<PauseRpcMethod>();
  }

  if (methodName == ForcePauseRpcMethod::getMethodName()) {
    return make_unique<ForcePauseRpcMethod>();
  }

  if (methodName == PauseAllRpcMethod::getMethodName()) {
    return make_unique<PauseAllRpcMethod>();
  }

  if (methodName == ForcePauseAllRpcMethod::getMethodName()) {
    return make_unique<ForcePauseAllRpcMethod>();
  }

  if (methodName == UnpauseRpcMethod::getMethodName()) {
    return make_unique<UnpauseRpcMethod>();
  }

  if (methodName == UnpauseAllRpcMethod::getMethodName()) {
    return make_unique<UnpauseAllRpcMethod>();
  }

  if (methodName == ForceRemoveRpcMethod::getMethodName()) {
    return make_unique<ForceRemoveRpcMethod>();
  }

  if (methodName == ChangePositionRpcMethod::getMethodName()) {
    return make_unique<ChangePositionRpcMethod>();
  }

  if (methodName == TellStatusRpcMethod::getMethodName()) {
    return make_unique<TellStatusRpcMethod>();
  }

  if (methodName == GetUrisRpcMethod::getMethodName()) {
    return make_unique<GetUrisRpcMethod>();
  }

  if (methodName == GetFilesRpcMethod::getMethodName()) {
    return make_unique<GetFilesRpcMethod>();
  }

  if (methodName == GetServersRpcMethod::getMethodName()) {
    return make_unique<GetServersRpcMethod>();
  }

  if (methodName == TellActiveRpcMethod::getMethodName()) {
    return make_unique<TellActiveRpcMethod>();
  }

  if (methodName == TellWaitingRpcMethod::getMethodName()) {
    return make_unique<TellWaitingRpcMethod>();
  }

  if (methodName == TellStoppedRpcMethod::getMethodName()) {
    return make_unique<TellStoppedRpcMethod>();
  }

  if (methodName == GetOptionRpcMethod::getMethodName()) {
    return make_unique<GetOptionRpcMethod>();
  }

  if (methodName == ChangeUriRpcMethod::getMethodName()) {
    return make_unique<ChangeUriRpcMethod>();
  }

  if (methodName == ChangeOptionRpcMethod::getMethodName()) {
    return make_unique<ChangeOptionRpcMethod>();
  }

  if (methodName == GetGlobalOptionRpcMethod::getMethodName()) {
    return make_unique<GetGlobalOptionRpcMethod>();
  }

  if (methodName == ChangeGlobalOptionRpcMethod::getMethodName()) {
    return make_unique<ChangeGlobalOptionRpcMethod>();
  }

  if (methodName == PurgeDownloadResultRpcMethod::getMethodName()) {
    return make_unique<PurgeDownloadResultRpcMethod>();
  }

  if (methodName == RemoveDownloadResultRpcMethod::getMethodName()) {
    return make_unique<RemoveDownloadResultRpcMethod>();
  }

  if (methodName == GetVersionRpcMethod::getMethodName()) {
    return make_unique<GetVersionRpcMethod>();
  }

  if (methodName == GetSessionInfoRpcMethod::getMethodName()) {
    return make_unique<GetSessionInfoRpcMethod>();
  }

  if (methodName == ShutdownRpcMethod::getMethodName()) {
    return make_unique<ShutdownRpcMethod>();
  }

  if (methodName == ForceShutdownRpcMethod::getMethodName()) {
    return make_unique<ForceShutdownRpcMethod>();
  }

  if (methodName == GetGlobalStatRpcMethod::getMethodName()) {
    return make_unique<GetGlobalStatRpcMethod>();
  }

  if (methodName == SaveSessionRpcMethod::getMethodName()) {
    return make_unique<SaveSessionRpcMethod>();
  }

  if (methodName == SystemMulticallRpcMethod::getMethodName()) {
    return make_unique<SystemMulticallRpcMethod>();
  }

  if (methodName == SystemListMethodsRpcMethod::getMethodName()) {
    return make_unique<SystemListMethodsRpcMethod>();
  }

  if (methodName == SystemListNotificationsRpcMethod::getMethodName()) {
    return make_unique<SystemListNotificationsRpcMethod>();
  }

  return nullptr;
}
} // namespace

RpcMethod* getMethod(const std::string& methodName)
{
  auto itr = cache.find(methodName);
  if (itr == std::end(cache)) {
    auto m = createMethod(methodName);
    if (m) {
      auto rv = cache.insert(std::make_pair(methodName, std::move(m)));
      return (*rv.first).second.get();
    }

    if (!noSuchRpcMethod) {
      noSuchRpcMethod = make_unique<NoSuchMethodRpcMethod>();
    }

    return noSuchRpcMethod.get();
  }

  return (*itr).second.get();
}

} // namespace rpc

} // namespace aria2
