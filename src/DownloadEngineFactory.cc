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
#include "DownloadEngineFactory.h"

#include <algorithm>

#include "Option.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "FileAllocationMan.h"
#include "CheckIntegrityMan.h"
#include "CheckIntegrityEntry.h"
#include "CheckIntegrityDispatcherCommand.h"
#include "prefs.h"
#include "FillRequestGroupCommand.h"
#include "FileAllocationDispatcherCommand.h"
#include "AutoSaveCommand.h"
#include "SaveSessionCommand.h"
#include "HaveEraseCommand.h"
#include "TimedHaltCommand.h"
#include "WatchProcessCommand.h"
#include "DownloadResult.h"
#include "ServerStatMan.h"
#include "a2io.h"
#include "DownloadContext.h"
#include "array_fun.h"
#include "EvictSocketPoolCommand.h"
#ifdef HAVE_LIBUV
#  include "LibuvEventPoll.h"
#endif // HAVE_LIBUV
#ifdef HAVE_EPOLL
#  include "EpollEventPoll.h"
#endif // HAVE_EPOLL
#ifdef HAVE_PORT_ASSOCIATE
#  include "PortEventPoll.h"
#endif // HAVE_PORT_ASSOCIATE
#ifdef HAVE_KQUEUE
#  include "KqueueEventPoll.h"
#endif // HAVE_KQUEUE
#ifdef HAVE_POLL
#  include "PollEventPoll.h"
#endif // HAVE_POLL
#include "SelectEventPoll.h"
#include "DlAbortEx.h"
#include "FileAllocationEntry.h"
#include "HttpListenCommand.h"
#include "LogFactory.h"

namespace aria2 {

DownloadEngineFactory::DownloadEngineFactory() = default;

namespace {
std::unique_ptr<EventPoll> createEventPoll(Option* op)
{
  const std::string& pollMethod = op->get(PREF_EVENT_POLL);
#ifdef HAVE_LIBUV
  if (pollMethod == V_LIBUV) {
    auto ep = make_unique<LibuvEventPoll>();
    if (!ep->good()) {
      throw DL_ABORT_EX("Initializing LibuvEventPoll failed."
                        " Try --event-poll=select");
    }
    return std::move(ep);
  }
  else
#endif // HAVE_LIBUV
#ifdef HAVE_EPOLL
      if (pollMethod == V_EPOLL) {
    auto ep = make_unique<EpollEventPoll>();
    if (!ep->good()) {
      throw DL_ABORT_EX("Initializing EpollEventPoll failed."
                        " Try --event-poll=select");
    }
    return std::move(ep);
  }
  else
#endif // HAVE_EPLL
#ifdef HAVE_KQUEUE
      if (pollMethod == V_KQUEUE) {
    auto kp = make_unique<KqueueEventPoll>();
    if (!kp->good()) {
      throw DL_ABORT_EX("Initializing KqueueEventPoll failed."
                        " Try --event-poll=select");
    }
    return std::move(kp);
  }
  else
#endif // HAVE_KQUEUE
#ifdef HAVE_PORT_ASSOCIATE
      if (pollMethod == V_PORT) {
    auto pp = make_unique<PortEventPoll>();
    if (!pp->good()) {
      throw DL_ABORT_EX("Initializing PortEventPoll failed."
                        " Try --event-poll=select");
    }
    return std::move(pp);
  }
  else
#endif // HAVE_PORT_ASSOCIATE
#ifdef HAVE_POLL
      if (pollMethod == V_POLL) {
    return make_unique<PollEventPoll>();
  }
  else
#endif // HAVE_POLL
    if (pollMethod == V_SELECT) {
      return make_unique<SelectEventPoll>();
    }
  assert(0);
  return nullptr;
}
} // namespace

std::unique_ptr<DownloadEngine> DownloadEngineFactory::newDownloadEngine(
    Option* op, std::vector<std::shared_ptr<RequestGroup>> requestGroups)
{
  const size_t MAX_CONCURRENT_DOWNLOADS =
      op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS);
  auto e = make_unique<DownloadEngine>(createEventPoll(op));
  e->setOption(op);
  {
    auto requestGroupMan = make_unique<RequestGroupMan>(
        std::move(requestGroups), MAX_CONCURRENT_DOWNLOADS, op);
    requestGroupMan->initWrDiskCache();
    e->setRequestGroupMan(std::move(requestGroupMan));
  }
  e->setFileAllocationMan(make_unique<FileAllocationMan>());
  e->setCheckIntegrityMan(make_unique<CheckIntegrityMan>());
  e->addRoutineCommand(
      make_unique<FillRequestGroupCommand>(e->newCUID(), e.get()));
  e->addRoutineCommand(make_unique<FileAllocationDispatcherCommand>(
      e->newCUID(), e->getFileAllocationMan().get(), e.get()));
  e->addRoutineCommand(make_unique<CheckIntegrityDispatcherCommand>(
      e->newCUID(), e->getCheckIntegrityMan().get(), e.get()));
  e->addRoutineCommand(
      make_unique<EvictSocketPoolCommand>(e->newCUID(), e.get(), 30_s));

  if (op->getAsInt(PREF_AUTO_SAVE_INTERVAL) > 0) {
    e->addRoutineCommand(make_unique<AutoSaveCommand>(
        e->newCUID(), e.get(),
        std::chrono::seconds(op->getAsInt(PREF_AUTO_SAVE_INTERVAL))));
  }
  if (op->getAsInt(PREF_SAVE_SESSION_INTERVAL) > 0) {
    e->addRoutineCommand(make_unique<SaveSessionCommand>(
        e->newCUID(), e.get(),
        std::chrono::seconds(op->getAsInt(PREF_SAVE_SESSION_INTERVAL))));
  }
  e->addRoutineCommand(
      make_unique<HaveEraseCommand>(e->newCUID(), e.get(), 10_s));
  {
    auto stopSec = op->getAsInt(PREF_STOP);
    if (stopSec > 0) {
      e->addRoutineCommand(make_unique<TimedHaltCommand>(
          e->newCUID(), e.get(), std::chrono::seconds(stopSec)));
    }
  }
  if (op->defined(PREF_STOP_WITH_PROCESS)) {
    unsigned int pid = op->getAsInt(PREF_STOP_WITH_PROCESS);
    e->addRoutineCommand(
        make_unique<WatchProcessCommand>(e->newCUID(), e.get(), pid));
  }
  if (op->getAsBool(PREF_ENABLE_RPC)) {
    if (op->get(PREF_RPC_SECRET).empty() && op->get(PREF_RPC_USER).empty()) {
      A2_LOG_WARN("Neither --rpc-secret nor a combination of --rpc-user and "
                  "--rpc-passwd is set. This is insecure. It is extremely "
                  "recommended to specify --rpc-secret with the adequate "
                  "secrecy or now deprecated --rpc-user and --rpc-passwd.");
    }
    bool ok = false;
    bool secure = op->getAsBool(PREF_RPC_SECURE);
    if (secure) {
      A2_LOG_NOTICE("RPC transport will be encrypted.");
    }
    static int families[] = {AF_INET, AF_INET6};
    size_t familiesLength = op->getAsBool(PREF_DISABLE_IPV6) ? 1 : 2;
    for (size_t i = 0; i < familiesLength; ++i) {
      auto httpListenCommand = make_unique<HttpListenCommand>(
          e->newCUID(), e.get(), families[i], secure);
      if (httpListenCommand->bindPort(op->getAsInt(PREF_RPC_LISTEN_PORT))) {
        e->addCommand(std::move(httpListenCommand));
        ok = true;
      }
    }
    if (!ok) {
      throw DL_ABORT_EX("Failed to setup RPC server.");
    }
  }
  return e;
}

} // namespace aria2
