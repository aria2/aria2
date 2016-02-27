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
#include "CheckIntegrityCommand.h"
#include "CheckIntegrityEntry.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "Logger.h"
#include "LogFactory.h"
#include "message.h"
#include "prefs.h"
#include "DownloadContext.h"
#include "a2functional.h"
#include "RecoverableException.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

CheckIntegrityCommand::CheckIntegrityCommand(cuid_t cuid,
                                             RequestGroup* requestGroup,
                                             DownloadEngine* e,
                                             CheckIntegrityEntry* entry)
    : RealtimeCommand{cuid, requestGroup, e}, entry_{entry}
{
}

CheckIntegrityCommand::~CheckIntegrityCommand()
{
  getDownloadEngine()->getCheckIntegrityMan()->dropPickedEntry();
}

bool CheckIntegrityCommand::executeInternal()
{
  if (getRequestGroup()->isHaltRequested()) {
    return true;
  }
  entry_->validateChunk();
  if (entry_->finished()) {
    // Enable control file saving here. See also
    // RequestGroup::processCheckIntegrityEntry() to know why this is
    // needed.
    getRequestGroup()->enableSaveControlFile();
    if (getRequestGroup()->downloadFinished()) {
      A2_LOG_NOTICE(
          fmt(MSG_VERIFICATION_SUCCESSFUL,
              getRequestGroup()->getDownloadContext()->getBasePath().c_str()));
      std::vector<std::unique_ptr<Command>> commands;
      entry_->onDownloadFinished(commands, getDownloadEngine());
      getDownloadEngine()->addCommand(std::move(commands));
    }
    else {
      A2_LOG_ERROR(
          fmt(MSG_VERIFICATION_FAILED,
              getRequestGroup()->getDownloadContext()->getBasePath().c_str()));
      std::vector<std::unique_ptr<Command>> commands;
      entry_->onDownloadIncomplete(commands, getDownloadEngine());
      getDownloadEngine()->addCommand(std::move(commands));
    }
    getDownloadEngine()->setNoWait(true);
    return true;
  }
  else {
    getDownloadEngine()->addCommand(std::unique_ptr<Command>(this));
    return false;
  }
}

bool CheckIntegrityCommand::handleException(Exception& e)
{
  A2_LOG_ERROR_EX(fmt(MSG_FILE_VALIDATION_FAILURE, getCuid()), e);
  A2_LOG_ERROR(
      fmt(MSG_DOWNLOAD_NOT_COMPLETE, getCuid(),
          getRequestGroup()->getDownloadContext()->getBasePath().c_str()));
  return true;
}

} // namespace aria2
