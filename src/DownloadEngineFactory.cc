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
#include "LogFactory.h"
#include "Option.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "FileAllocationMan.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityMan.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "prefs.h"
#include "Util.h"
#include "CUIDCounter.h"
#include "FillRequestGroupCommand.h"
#include "FileAllocationDispatcherCommand.h"
#include "AutoSaveCommand.h"
#include "HaveEraseCommand.h"
#include "PeerListenCommand.h"

DownloadEngineFactory::DownloadEngineFactory():
  _logger(LogFactory::getInstance()) {}

DownloadEngineHandle
DownloadEngineFactory::newDownloadEngine(Option* op,
					 const RequestGroups& requestGroups)
{
  RequestGroups workingSet;
  RequestGroups reservedSet;
  if(op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS) < (int32_t)requestGroups.size()) {
    copy(requestGroups.begin(), requestGroups.begin()+op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS), back_inserter(workingSet));
    copy(requestGroups.begin()+op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS),
	 requestGroups.end(), back_inserter(reservedSet));
  } else {
    workingSet = requestGroups;
  }

  DownloadEngineHandle e = new DownloadEngine();
  e->option = op;
  RequestGroupManHandle requestGroupMan =
    new RequestGroupMan(workingSet,
			op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
  requestGroupMan->addReservedGroup(reservedSet);
  e->_requestGroupMan = requestGroupMan;
  e->_fileAllocationMan = new FileAllocationMan();
#ifdef ENABLE_MESSAGE_DIGEST
  e->_checkIntegrityMan = new CheckIntegrityMan();
#endif // ENABLE_MESSAGE_DIGEST
  e->commands.push_back(new FillRequestGroupCommand(CUIDCounterSingletonHolder::instance()->newID(), e.get(), 1));
  e->commands.push_back(new FileAllocationDispatcherCommand(CUIDCounterSingletonHolder::instance()->newID(), e.get()));
  e->commands.push_back(new AutoSaveCommand(CUIDCounterSingletonHolder::instance()->newID(), e.get(), op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
  e->commands.push_back(new HaveEraseCommand(CUIDCounterSingletonHolder::instance()->newID(), e.get(), 10));

  PeerListenCommand* listenCommand =
    new PeerListenCommand(CUIDCounterSingletonHolder::instance()->newID(), e.get());
  int32_t port;
  int32_t listenPort = op->getAsInt(PREF_LISTEN_PORT);
  if(listenPort == -1) {
    port = listenCommand->bindPort(6881, 6999);
  } else {
    port = listenCommand->bindPort(listenPort, listenPort);
  }
  if(port == -1) {
    _logger->error(_("Errors occurred while binding port.\n"));
    delete listenCommand;
  } else {
    op->put(PREF_LISTEN_PORT, Util::itos(port).c_str());
    e->commands.push_back(listenCommand);
  }
  //btRuntime->setListenPort(port);

  return e;
}
