/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "AbstractHttpServerResponseCommand.h"
#include "SocketCore.h"
#include "DownloadEngine.h"
#include "HttpServer.h"
#include "Logger.h"
#include "LogFactory.h"
#include "HttpServerCommand.h"
#include "RequestGroupMan.h"
#include "RecoverableException.h"
#include "wallclock.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

AbstractHttpServerResponseCommand::AbstractHttpServerResponseCommand
(cuid_t cuid,
 const SharedHandle<HttpServer>& httpServer,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& socket)
 : Command(cuid),
   e_(e),
   socket_(socket),
   httpServer_(httpServer)
{
  setStatus(Command::STATUS_ONESHOT_REALTIME); 
  e_->addSocketForWriteCheck(socket_, this);
}

AbstractHttpServerResponseCommand::~AbstractHttpServerResponseCommand()
{
  e_->deleteSocketForWriteCheck(socket_, this);
}

bool AbstractHttpServerResponseCommand::execute()
{
  if(e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  try {
    httpServer_->sendResponse();
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX
      (fmt("CUID#%" PRId64 " - Error occurred while transmitting response body.",
           getCuid()),
       e);
    return true;
  }
  if(httpServer_->sendBufferIsEmpty()) {
    A2_LOG_INFO(fmt("CUID#%" PRId64 " - HttpServer: all response transmitted.",
                    getCuid()));
    afterSend(httpServer_, e_);
    return true;
  } else {
    if(timeoutTimer_.difference(global::wallclock()) >= 10) {
      A2_LOG_INFO(fmt("CUID#%" PRId64 " - HttpServer: Timeout while trasmitting"
                      " response.",
                      getCuid()));
      return true;
    } else {
      e_->addCommand(this);
      return false;
    }
  }
}

} // namespace aria2
