/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "HttpServerResponseCommand.h"
#include "SocketCore.h"
#include "DownloadEngine.h"
#include "HttpServer.h"
#include "Logger.h"
#include "HttpServerCommand.h"
#include "RequestGroupMan.h"
#include "RecoverableException.h"
#include "FileEntry.h"
#include "wallclock.h"
#include "util.h"

namespace aria2 {

HttpServerResponseCommand::HttpServerResponseCommand
(cuid_t cuid,
 const SharedHandle<HttpServer>& httpServer,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& socket):
  Command(cuid),
  _e(e),
  _socket(socket),
  _httpServer(httpServer)
{
  setStatus(Command::STATUS_ONESHOT_REALTIME); 
  _e->addSocketForWriteCheck(_socket, this);
}

HttpServerResponseCommand::~HttpServerResponseCommand()
{
  _e->deleteSocketForWriteCheck(_socket, this);
}

bool HttpServerResponseCommand::execute()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    return true;
  }
  try {
    _httpServer->sendResponse();
  } catch(RecoverableException& e) {
    if(logger->info()) {
      logger->info("CUID#%s - Error occurred while transmitting response body.",
                   e, util::itos(cuid).c_str());
    }
    return true;
  }
  if(_httpServer->sendBufferIsEmpty()) {
    if(logger->info()) {
      logger->info("CUID#%s - HttpServer: all response transmitted.",
                   util::itos(cuid).c_str());
    }
    if(_httpServer->supportsPersistentConnection()) {
      if(logger->info()) {
        logger->info("CUID#%s - Persist connection.", util::itos(cuid).c_str());
      }
      _e->commands.push_back
        (new HttpServerCommand(cuid, _httpServer, _e, _socket));
    }
    return true;
  } else {
    if(_timeout.difference(global::wallclock) >= 10) {
      if(logger->info()) {
        logger->info("CUID#%s - HttpServer: Timeout while trasmitting"
                     " response.", util::itos(cuid).c_str());
      }
      return true;
    } else {
      _e->commands.push_back(this);
      return false;
    }
  }
}

} // namespace aria2
