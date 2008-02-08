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
#include "StreamFileAllocationEntry.h"
#include "CUIDCounter.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "Request.h"
#include "prefs.h"
#include "RequestGroup.h"
#include "InitiateConnectionCommandFactory.h"
#include <algorithm>

namespace aria2 {

StreamFileAllocationEntry::StreamFileAllocationEntry(const RequestHandle& currentRequest,
						     RequestGroup* requestGroup,
						     Command* nextCommand):
  FileAllocationEntry(requestGroup, nextCommand),
  _currentRequest(currentRequest)
{}

StreamFileAllocationEntry::~StreamFileAllocationEntry() {}

Commands StreamFileAllocationEntry::prepareForNextAction(DownloadEngine* e)
{
  Commands commands;
  if(_nextCommand) {
    commands.push_back(popNextCommand());
    // try remaining uris
    Commands streamCommands = _requestGroup->createNextCommandWithAdj(e, -1);
    std::copy(streamCommands.begin(), streamCommands.end(), std::back_inserter(commands));
  } else {
    if(_currentRequest.isNull()) {
      commands = _requestGroup->createNextCommandWithAdj(e, 0);
    } else {
      Commands streamCommands = _requestGroup->createNextCommandWithAdj(e, -1);
      Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(CUIDCounterSingletonHolder::instance()->newID(),
											   _currentRequest, _requestGroup, e);
      
      commands.push_back(command);
      std::copy(streamCommands.begin(), streamCommands.end(), std::back_inserter(commands));
    }
  }
  return commands;
}

} // namespace aria2
