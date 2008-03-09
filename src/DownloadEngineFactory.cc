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
#include "Logger.h"
#include "Option.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "FileAllocationMan.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityMan.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "prefs.h"
#include "CUIDCounter.h"
#include "FillRequestGroupCommand.h"
#include "FileAllocationDispatcherCommand.h"
#include "AutoSaveCommand.h"
#include "HaveEraseCommand.h"
#include "TimedHaltCommand.h"
#include "DownloadResult.h"
#include <algorithm>

namespace aria2 {

DownloadEngineFactory::DownloadEngineFactory():
  _logger(LogFactory::getInstance()) {}

DownloadEngineHandle
DownloadEngineFactory::newDownloadEngine(Option* op,
					 const RequestGroups& requestGroups)
{
  RequestGroups workingSet;
  RequestGroups reservedSet;
  if((size_t)op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS) < requestGroups.size()) {
    std::copy(requestGroups.begin(), requestGroups.begin()+op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS), std::back_inserter(workingSet));
    std::copy(requestGroups.begin()+op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS),
	      requestGroups.end(), std::back_inserter(reservedSet));
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
  {
    time_t stopSec = op->getAsInt(PREF_STOP);
    if(stopSec > 0) {
      e->commands.push_back(new TimedHaltCommand(CUIDCounterSingletonHolder::instance()->newID(), e.get(), stopSec));
    }
  }
  return e;
}

} // namespace aria2
