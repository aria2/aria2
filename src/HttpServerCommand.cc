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
#include "LogFactory.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "HttpServerBodyCommand.h"
#include "HttpServerResponseCommand.h"
#include "RecoverableException.h"
#include "prefs.h"
#include "Option.h"
#include "util.h"
#include "wallclock.h"
#include "fmt.h"

namespace aria2 {

HttpServerCommand::HttpServerCommand
(cuid_t cuid,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& socket)
  : Command(cuid),
    e_(e),
    socket_(socket),
    httpServer_(new HttpServer(socket, e))
{
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  e_->addSocketForReadCheck(socket_, this);
  httpServer_->setUsernamePassword(e_->getOption()->get(PREF_XML_RPC_USER),
                                   e_->getOption()->get(PREF_XML_RPC_PASSWD));
#ifdef HAVE_LIBZ
  httpServer_->enableGZip();
#else // !HAVE_LIBZ
  httpServer_->disableGZip();
#endif // !HAVE_LIBZ
}

HttpServerCommand::HttpServerCommand
(cuid_t cuid,
 const SharedHandle<HttpServer>& httpServer,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& socket)
  : Command(cuid),
    e_(e),
    socket_(socket),
    httpServer_(httpServer)
{
  e_->addSocketForReadCheck(socket_, this);
}

HttpServerCommand::~HttpServerCommand()
{
  e_->deleteSocketForReadCheck(socket_, this);
}

bool HttpServerCommand::execute()
{
  if(e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  try {
    if(socket_->isReadable(0)) {
      timeoutTimer_ = global::wallclock;
      SharedHandle<HttpHeader> header;

      header = httpServer_->receiveRequest();

      if(!header) {
        e_->addCommand(this);
        return false;
      }
      if(!httpServer_->authenticate()) {
        httpServer_->disableKeepAlive();
        httpServer_->feedResponse("401 Unauthorized",
                                  "WWW-Authenticate: Basic realm=\"aria2\"",
                                  "","text/html");
        Command* command =
          new HttpServerResponseCommand(getCuid(), httpServer_, e_, socket_);
        e_->addCommand(command);
        e_->setNoWait(true);
        return true;
      }
      if(static_cast<uint64_t>
         (e_->getOption()->getAsInt(PREF_XML_RPC_MAX_REQUEST_SIZE)) <
         httpServer_->getContentLength()) {
        A2_LOG_INFO(fmt("Request too long. ContentLength=%s."
                        " See --xml-rpc-max-request-size option to loose"
                        " this limitation.",
                        util::uitos(httpServer_->getContentLength()).c_str()));
        return true;
      }
      Command* command = new HttpServerBodyCommand(getCuid(), httpServer_, e_,
                                                   socket_);
      e_->addCommand(command);
      e_->setNoWait(true);
      return true;
    } else {
      if(timeoutTimer_.difference(global::wallclock) >= 30) {
        A2_LOG_INFO("HTTP request timeout.");
        return true;
      } else {
        e_->addCommand(this);
        return false;
      }
    }
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(fmt("CUID#%lld - Error occurred while reading HTTP request",
                       getCuid()),
                   e);
    return true;
  }

}

} // namespace aria2
