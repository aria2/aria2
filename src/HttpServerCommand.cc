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
#include "HttpServerCommand.h"
#include "SocketCore.h"
#include "DownloadEngine.h"
#include "HttpServer.h"
#include "HttpHeader.h"
#include "Logger.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "HttpServerBodyCommand.h"
#include "HttpServerResponseCommand.h"
#include "RecoverableException.h"
#include "prefs.h"
#include "Option.h"
#include "util.h"
#include "DownloadContext.h"

namespace aria2 {

HttpServerCommand::HttpServerCommand(int32_t cuid, DownloadEngine* e,
                                     const SharedHandle<SocketCore>& socket):
  Command(cuid),
  _e(e),
  _socket(socket),
  _httpServer(new HttpServer(socket, e))
{
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  _e->addSocketForReadCheck(_socket, this);
  _httpServer->setUsernamePassword(_e->option->get(PREF_XML_RPC_USER),
                                   _e->option->get(PREF_XML_RPC_PASSWD));
}

HttpServerCommand::HttpServerCommand(int32_t cuid,
                                     const SharedHandle<HttpServer>& httpServer,
                                     DownloadEngine* e,
                                     const SharedHandle<SocketCore>& socket):
  Command(cuid),
  _e(e),
  _socket(socket),
  _httpServer(httpServer)
{
  _e->addSocketForReadCheck(_socket, this);
}

HttpServerCommand::~HttpServerCommand()
{
  _e->deleteSocketForReadCheck(_socket, this);
}

bool HttpServerCommand::execute()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    return true;
  }
  try {
    if(_socket->isReadable(0)) {
      _timeout.reset();
      SharedHandle<HttpHeader> header;

      header = _httpServer->receiveRequest();

      if(header.isNull()) {
        _e->commands.push_back(this);
        return false;
      }
      if(!_httpServer->authenticate()) {
        _httpServer->disableKeepAlive();
        _httpServer->feedResponse("401 Unauthorized",
                                  "WWW-Authenticate: Basic realm=\"aria2\"",
                                  "","text/html");
        Command* command =
          new HttpServerResponseCommand(cuid, _httpServer, _e, _socket);
        _e->commands.push_back(command);
        _e->setNoWait(true);
        return true;
      }
      if(static_cast<uint64_t>
         (_e->option->getAsInt(PREF_XML_RPC_MAX_REQUEST_SIZE)) <
         _httpServer->getContentLength()) {
        logger->info("Request too long. ContentLength=%s."
                     " See --xml-rpc-max-request-size option to loose"
                     " this limitation.",
                     util::uitos(_httpServer->getContentLength()).c_str());
        return true;
      }
      Command* command = new HttpServerBodyCommand(cuid, _httpServer, _e,
                                                   _socket);
      _e->commands.push_back(command);
      _e->setNoWait(true);
      return true;
    } else {
      if(_timeout.elapsed(30)) {
        logger->info("HTTP request timeout.");
        return true;
      } else {
        _e->commands.push_back(this);
        return false;
      }
    }
  } catch(RecoverableException& e) {
    logger->info("CUID#%d - Error occurred while reading HTTP request",
                 e, cuid);
    return true;
  }

}

} // namespace aria2
