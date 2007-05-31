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
#include "FileAllocationCommand.h"
#include "InitiateConnectionCommandFactory.h"
#include "message.h"
#include "DownloadCommand.h"

bool FileAllocationCommand::executeInternal()
{
  _fileAllocationEntry->allocateChunk();
  
  if(_fileAllocationEntry->allocationFinished()) {
    int64_t totalLength = _requestGroup->getSegmentMan()->totalSize;
    logger->debug("%d seconds to allocate %lld byte(s)",
		  _timer.difference(), totalLength);
    
    _e->_fileAllocationMan->markCurrentFileAllocationEntryDone();
    
    if(_fileAllocationEntry->getNextDownloadCommand()) {
      _e->commands.push_back(_fileAllocationEntry->getNextDownloadCommand());
      _fileAllocationEntry->setNextDownloadCommand(0);
    } else {
      int32_t numCommandsToGenerate = 15;
      Commands commands = _requestGroup->getNextCommand(_e, numCommandsToGenerate);
      
      Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, _req, _requestGroup, _e);
      
      commands.push_front(command);
    
      _e->addCommand(commands);
    }
    return true;
  } else {
    _e->commands.push_back(this);
    return false;
  }
}

bool FileAllocationCommand::handleException(Exception* e)
{
  logger->error("CUID#%d - Exception caught while allocating file space.", e, cuid);
  delete e;
  logger->error(MSG_DOWNLOAD_NOT_COMPLETE, cuid, _requestGroup->getFilePath().c_str());
  return true;
}
