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
#include "FtpDownloadCommand.h"
#include "Request.h"
#include "Socket.h"
#include "Segment.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "prefs.h"
#include "Option.h"
#include "FtpFinishDownloadCommand.h"
#include "FtpConnection.h"
#include "Logger.h"

namespace aria2 {

FtpDownloadCommand::FtpDownloadCommand
(int cuid,
 const RequestHandle& req,
 RequestGroup* requestGroup,
 const SharedHandle<FtpConnection>& ftpConnection,
 DownloadEngine* e,
 const SocketHandle& dataSocket,
 const SocketHandle& ctrlSocket)
  :DownloadCommand(cuid, req, requestGroup, e, dataSocket),
   _ftpConnection(ftpConnection),
   ctrlSocket(ctrlSocket) {}

FtpDownloadCommand::~FtpDownloadCommand() {}


bool FtpDownloadCommand::prepareForNextSegment()
{
  if(e->option->getAsBool(PREF_FTP_REUSE_CONNECTION) &&
     (uint64_t)_segments.front()->getPositionToWrite() == _requestGroup->getTotalLength()) {
    Command* command = new FtpFinishDownloadCommand(cuid, req, _requestGroup, _ftpConnection, e, ctrlSocket);
    e->commands.push_back(command);

    if(_requestGroup->downloadFinished()) {
      // To run checksum checking, we had to call following function here.
      DownloadCommand::prepareForNextSegment();
    }
    return true;
  } else {
    return DownloadCommand::prepareForNextSegment();
  }
}

} // namespace aria2
