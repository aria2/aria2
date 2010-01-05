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
#include "FtpFinishDownloadCommand.h"

#include <map>

#include "Request.h"
#include "DownloadEngine.h"
#include "prefs.h"
#include "Option.h"
#include "FtpConnection.h"
#include "message.h"
#include "StringFormat.h"
#include "DlAbortEx.h"
#include "SocketCore.h"
#include "RequestGroup.h"
#include "Logger.h"
#include "DownloadContext.h"

namespace aria2 {

FtpFinishDownloadCommand::FtpFinishDownloadCommand
(int cuid,
 const RequestHandle& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 const SharedHandle<FtpConnection>& ftpConnection,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& socket)
  :AbstractCommand(cuid, req, fileEntry, requestGroup, e, socket),
   _ftpConnection(ftpConnection)
{
  e->addSocketForReadCheck(socket, this);
}

FtpFinishDownloadCommand::~FtpFinishDownloadCommand()
{
  e->deleteSocketForReadCheck(socket, this);
}

// overrides AbstractCommand::execute().
// AbstractCommand::_segments is empty.
bool FtpFinishDownloadCommand::execute()
{
  if(_requestGroup->isHaltRequested()) {
    return true;
  }
  try {
    unsigned int status = _ftpConnection->receiveResponse();
    if(status == 0) {
      e->commands.push_back(this);
      return false;
    }
    if(status != 226) {
      throw DL_ABORT_EX(StringFormat(EX_BAD_STATUS, status).str());
    }
    if(getOption()->getAsBool(PREF_FTP_REUSE_CONNECTION)) {
      std::map<std::string, std::string> options;
      options["baseWorkingDir"] = _ftpConnection->getBaseWorkingDir();
      e->poolSocket(req, isProxyDefined(), socket, options);
    }
  } catch(RecoverableException& e) {
    logger->info(EX_EXCEPTION_CAUGHT, e);
  }
  if(_requestGroup->downloadFinished()) {
    return true;
  } else {
    return prepareForRetry(0);
  }
}

// This function never be called.
bool FtpFinishDownloadCommand::executeInternal() { return true; }

} // namespace aria2
