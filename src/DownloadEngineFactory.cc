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
#include "DownloadEngineFactory.h"

#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "Option.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "FileAllocationMan.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityMan.h"
# include "CheckIntegrityEntry.h"
# include "CheckIntegrityDispatcherCommand.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "prefs.h"
#include "FillRequestGroupCommand.h"
#include "FileAllocationDispatcherCommand.h"
#include "AutoSaveCommand.h"
#include "HaveEraseCommand.h"
#include "TimedHaltCommand.h"
#include "DownloadResult.h"
#include "ServerStatMan.h"
#include "a2io.h"
#include "DownloadContext.h"
#ifdef HAVE_EPOLL
# include "EpollEventPoll.h"
#endif // HAVE_EPOLL
#include "SelectEventPoll.h"
#include "DlAbortEx.h"
#include "FileAllocationEntry.h"
#ifdef ENABLE_XML_RPC
# include "HttpListenCommand.h"
#endif // ENABLE_XML_RPC

namespace aria2 {

DownloadEngineFactory::DownloadEngineFactory():
  _logger(LogFactory::getInstance()) {}

DownloadEngineHandle
DownloadEngineFactory::newDownloadEngine(Option* op,
					 const RequestGroups& requestGroups)
{
  RequestGroups workingSet;
  RequestGroups reservedSet;
  const size_t MAX_CONCURRENT_DOWNLOADS = op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS);
  if(MAX_CONCURRENT_DOWNLOADS < requestGroups.size()) {
    workingSet.insert(workingSet.end(),
		      requestGroups.begin(),
		      requestGroups.begin()+MAX_CONCURRENT_DOWNLOADS);

    reservedSet.insert(reservedSet.end(),
		       requestGroups.begin()+MAX_CONCURRENT_DOWNLOADS,
		       requestGroups.end());
  } else {
    workingSet = requestGroups;
  }

  SharedHandle<EventPoll> eventPoll;
#ifdef HAVE_EPOLL
  if(op->get(PREF_EVENT_POLL) == V_EPOLL) {
    SharedHandle<EpollEventPoll> ep(new EpollEventPoll());
    if(ep->good()) {
      eventPoll = ep;
    } else {
      throw DL_ABORT_EX("Initializing EpollEventPoll failed."
		      " Try --event-poll=select");
    }
  } else
#endif // HAVE_EPLL
    if(op->get(PREF_EVENT_POLL) == V_SELECT) {
      eventPoll.reset(new SelectEventPoll());
    } else {
      abort();
    }
  DownloadEngineHandle e(new DownloadEngine(eventPoll));
  e->option = op;

  RequestGroupManHandle
    requestGroupMan(new RequestGroupMan(workingSet, MAX_CONCURRENT_DOWNLOADS,
					op));
  requestGroupMan->addReservedGroup(reservedSet);
  e->_requestGroupMan = requestGroupMan;
  e->_fileAllocationMan.reset(new FileAllocationMan());
#ifdef ENABLE_MESSAGE_DIGEST
  e->_checkIntegrityMan.reset(new CheckIntegrityMan());
#endif // ENABLE_MESSAGE_DIGEST
  e->addRoutineCommand(new FillRequestGroupCommand(e->newCUID(), e.get(), 1));
  e->addRoutineCommand(new FileAllocationDispatcherCommand
		       (e->newCUID(), e->_fileAllocationMan, e.get()));
#ifdef ENABLE_MESSAGE_DIGEST
  e->addRoutineCommand(new CheckIntegrityDispatcherCommand
		       (e->newCUID(), e->_checkIntegrityMan, e.get()));
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
#ifdef ENABLE_XML_RPC
  if(op->getAsBool(PREF_ENABLE_XML_RPC)) {
    HttpListenCommand* httpListenCommand =
      new HttpListenCommand(e->newCUID(), e.get());
    if(httpListenCommand->bindPort(op->getAsInt(PREF_XML_RPC_LISTEN_PORT))){
      e->addRoutineCommand(httpListenCommand);
    } else {
      delete httpListenCommand;
    }
  }
#endif // ENABLE_XML_RPC
  return e;
}

} // namespace aria2
