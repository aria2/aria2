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
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityDispatcherCommand.h"
#endif // ENABLE_MESSAGE_DIGEST
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
#ifdef HAVE_LIBUV
# include "LibuvEventPoll.h"
#endif // HAVE_LIBUV
#ifdef HAVE_EPOLL
# include "EpollEventPoll.h"
#endif // HAVE_EPOLL
#ifdef HAVE_PORT_ASSOCIATE
# include "PortEventPoll.h"
#endif // HAVE_PORT_ASSOCIATE
#ifdef HAVE_KQUEUE
# include "KqueueEventPoll.h"
#endif // HAVE_KQUEUE
#ifdef HAVE_POLL
# include "PollEventPoll.h"
#endif // HAVE_POLL
#include "SelectEventPoll.h"
#include "DlAbortEx.h"
#include "FileAllocationEntry.h"
#include "HttpListenCommand.h"
#include "LogFactory.h"

namespace aria2 {

DownloadEngineFactory::DownloadEngineFactory() {}

std::shared_ptr<DownloadEngine>
DownloadEngineFactory::newDownloadEngine
(Option* op, std::vector<std::shared_ptr<RequestGroup> > requestGroups)
{
  const size_t MAX_CONCURRENT_DOWNLOADS =
    op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS);
  std::shared_ptr<EventPoll> eventPoll;
  const std::string& pollMethod = op->get(PREF_EVENT_POLL);
#ifdef HAVE_LIBUV
  if (pollMethod == V_LIBUV) {
    std::shared_ptr<LibuvEventPoll> ep(new LibuvEventPoll());
    if (!ep->good()) {
      throw DL_ABORT_EX("Initializing LibuvEventPoll failed."
                        " Try --event-poll=select");
    }
    eventPoll = ep;
  }
  else
#endif // HAVE_LIBUV
#ifdef HAVE_EPOLL
  if(pollMethod == V_EPOLL) {
    std::shared_ptr<EpollEventPoll> ep(new EpollEventPoll());
    if(ep->good()) {
      eventPoll = ep;
    } else {
      throw DL_ABORT_EX("Initializing EpollEventPoll failed."
                        " Try --event-poll=select");
    }
  } else
#endif // HAVE_EPLL
#ifdef HAVE_KQUEUE
    if(pollMethod == V_KQUEUE) {
      std::shared_ptr<KqueueEventPoll> kp(new KqueueEventPoll());
      if(kp->good()) {
        eventPoll = kp;
      } else {
        throw DL_ABORT_EX("Initializing KqueueEventPoll failed."
                          " Try --event-poll=select");
      }
    } else
#endif // HAVE_KQUEUE
#ifdef HAVE_PORT_ASSOCIATE
      if(pollMethod == V_PORT) {
        std::shared_ptr<PortEventPoll> pp(new PortEventPoll());
        if(pp->good()) {
          eventPoll = pp;
        } else {
          throw DL_ABORT_EX("Initializing PortEventPoll failed."
                            " Try --event-poll=select");
        }
      } else
#endif // HAVE_PORT_ASSOCIATE
#ifdef HAVE_POLL
        if(pollMethod == V_POLL) {
          eventPoll.reset(new PollEventPoll());
        } else
#endif // HAVE_POLL
          if(pollMethod == V_SELECT) {
            eventPoll.reset(new SelectEventPoll());
          } else {
            abort();
          }
  std::shared_ptr<DownloadEngine> e(new DownloadEngine(eventPoll));
  e->setOption(op);

  auto requestGroupMan = std::make_shared<RequestGroupMan>
    (std::move(requestGroups), MAX_CONCURRENT_DOWNLOADS, op);
  requestGroupMan->initWrDiskCache();
  e->setRequestGroupMan(requestGroupMan);
  e->setFileAllocationMan
    (std::shared_ptr<FileAllocationMan>(new FileAllocationMan()));
#ifdef ENABLE_MESSAGE_DIGEST
  e->setCheckIntegrityMan
    (std::shared_ptr<CheckIntegrityMan>(new CheckIntegrityMan()));
#endif // ENABLE_MESSAGE_DIGEST
  e->addRoutineCommand(make_unique<FillRequestGroupCommand>
                       (e->newCUID(), e.get()));
  e->addRoutineCommand(make_unique<FileAllocationDispatcherCommand>
                       (e->newCUID(), e->getFileAllocationMan(), e.get()));
#ifdef ENABLE_MESSAGE_DIGEST
  e->addRoutineCommand(make_unique<CheckIntegrityDispatcherCommand>
                       (e->newCUID(), e->getCheckIntegrityMan(), e.get()));
#endif // ENABLE_MESSAGE_DIGEST

  if(op->getAsInt(PREF_AUTO_SAVE_INTERVAL) > 0) {
    e->addRoutineCommand
      (make_unique<AutoSaveCommand>(e->newCUID(), e.get(),
                                    op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
  }
  if(op->getAsInt(PREF_SAVE_SESSION_INTERVAL) > 0) {
    e->addRoutineCommand(make_unique<SaveSessionCommand>
                         (e->newCUID(), e.get(),
                          op->getAsInt(PREF_SAVE_SESSION_INTERVAL)));
  }
  e->addRoutineCommand(make_unique<HaveEraseCommand>
                       (e->newCUID(), e.get(), 10));
  {
    time_t stopSec = op->getAsInt(PREF_STOP);
    if(stopSec > 0) {
      e->addRoutineCommand(make_unique<TimedHaltCommand>(e->newCUID(), e.get(),
                                                         stopSec));
    }
  }
  if(op->defined(PREF_STOP_WITH_PROCESS)) {
    unsigned int pid = op->getAsInt(PREF_STOP_WITH_PROCESS);
    e->addRoutineCommand(make_unique<WatchProcessCommand>(e->newCUID(),
                                                          e.get(), pid));
  }
  if(op->getAsBool(PREF_ENABLE_RPC)) {
    bool ok = false;
    bool secure = op->getAsBool(PREF_RPC_SECURE);
    if(secure) {
      A2_LOG_NOTICE("RPC transport will be encrypted.");
    }
    static int families[] = { AF_INET, AF_INET6 };
    size_t familiesLength = op->getAsBool(PREF_DISABLE_IPV6)?1:2;
    for(size_t i = 0; i < familiesLength; ++i) {
      auto httpListenCommand = make_unique<HttpListenCommand>
        (e->newCUID(), e.get(), families[i], secure);
      if(httpListenCommand->bindPort(op->getAsInt(PREF_RPC_LISTEN_PORT))){
        e->addCommand(std::move(httpListenCommand));
        ok = true;
      }
    }
    if(!ok) {
      throw DL_ABORT_EX("Failed to setup RPC server.");
    }
  }
  return e;
}

} // namespace aria2
