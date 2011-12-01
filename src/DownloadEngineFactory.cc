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
#include "HaveEraseCommand.h"
#include "TimedHaltCommand.h"
#include "WatchProcessCommand.h"
#include "DownloadResult.h"
#include "ServerStatMan.h"
#include "a2io.h"
#include "DownloadContext.h"
#include "array_fun.h"
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

namespace aria2 {

DownloadEngineFactory::DownloadEngineFactory() {}

DownloadEngineHandle
DownloadEngineFactory::newDownloadEngine
(Option* op, const std::vector<SharedHandle<RequestGroup> >& requestGroups)
{
  const size_t MAX_CONCURRENT_DOWNLOADS =
    op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS);
  SharedHandle<EventPoll> eventPoll;
  const std::string& pollMethod = op->get(PREF_EVENT_POLL);
#ifdef HAVE_EPOLL
  if(pollMethod == V_EPOLL) {
    SharedHandle<EpollEventPoll> ep(new EpollEventPoll());
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
      SharedHandle<KqueueEventPoll> kp(new KqueueEventPoll());
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
        SharedHandle<PortEventPoll> pp(new PortEventPoll());
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
  DownloadEngineHandle e(new DownloadEngine(eventPoll));
  e->setOption(op);

  RequestGroupManHandle
    requestGroupMan(new RequestGroupMan(requestGroups, MAX_CONCURRENT_DOWNLOADS,
                                        op));
  e->setRequestGroupMan(requestGroupMan);
  e->setFileAllocationMan
    (SharedHandle<FileAllocationMan>(new FileAllocationMan()));
#ifdef ENABLE_MESSAGE_DIGEST
  e->setCheckIntegrityMan
    (SharedHandle<CheckIntegrityMan>(new CheckIntegrityMan()));
#endif // ENABLE_MESSAGE_DIGEST
  e->addRoutineCommand(new FillRequestGroupCommand(e->newCUID(), e.get()));
  e->addRoutineCommand(new FileAllocationDispatcherCommand
                       (e->newCUID(), e->getFileAllocationMan(), e.get()));
#ifdef ENABLE_MESSAGE_DIGEST
  e->addRoutineCommand(new CheckIntegrityDispatcherCommand
                       (e->newCUID(), e->getCheckIntegrityMan(), e.get()));
#endif // ENABLE_MESSAGE_DIGEST

  if(op->getAsInt(PREF_AUTO_SAVE_INTERVAL) > 0) {
    e->addRoutineCommand
      (new AutoSaveCommand(e->newCUID(), e.get(),
                           op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
  }
  e->addRoutineCommand(new HaveEraseCommand(e->newCUID(), e.get(), 10));
  {
    time_t stopSec = op->getAsInt(PREF_STOP);
    if(stopSec > 0) {
      e->addRoutineCommand(new TimedHaltCommand(e->newCUID(), e.get(),
                                                stopSec));
    }
  }
  if(op->defined(PREF_STOP_WITH_PROCESS)) {
    unsigned int pid = op->getAsInt(PREF_STOP_WITH_PROCESS);
    e->addRoutineCommand(new WatchProcessCommand(e->newCUID(), e.get(), pid));
  }
  if(op->getAsBool(PREF_ENABLE_RPC)) {
    bool ok = false;
    static int families[] = { AF_INET, AF_INET6 };
    size_t familiesLength = op->getAsBool(PREF_DISABLE_IPV6)?1:2;
    for(size_t i = 0; i < familiesLength; ++i) {
      HttpListenCommand* httpListenCommand =
        new HttpListenCommand(e->newCUID(), e.get(), families[i]);
      if(httpListenCommand->bindPort(op->getAsInt(PREF_RPC_LISTEN_PORT))){
        e->addCommand(httpListenCommand);
        ok = true;
      } else {
        delete httpListenCommand;
      }
    }
    if(!ok) {
      throw DL_ABORT_EX("Failed to setup RPC server.");
    }
  }
  return e;
}

} // namespace aria2
