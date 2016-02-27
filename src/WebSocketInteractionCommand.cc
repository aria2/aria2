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
#include "WebSocketInteractionCommand.h"
#include "SocketCore.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "WebSocketSession.h"
#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"
#include "SingletonHolder.h"
#include "Notifier.h"
#include "WebSocketSessionMan.h"

namespace aria2 {

namespace rpc {

WebSocketInteractionCommand::WebSocketInteractionCommand(
    cuid_t cuid, const std::shared_ptr<WebSocketSession>& wsSession,
    DownloadEngine* e, const std::shared_ptr<SocketCore>& socket)
    : Command(cuid),
      e_(e),
      socket_(socket),
      writeCheck_(false),
      wsSession_(wsSession)
{
  e_->getWebSocketSessionMan()->addSession(wsSession_);
  e_->addSocketForReadCheck(socket_, this);
}

WebSocketInteractionCommand::~WebSocketInteractionCommand()
{
  e_->deleteSocketForReadCheck(socket_, this);
  if (writeCheck_) {
    e_->deleteSocketForWriteCheck(socket_, this);
  }
  e_->getWebSocketSessionMan()->removeSession(wsSession_);
}

void WebSocketInteractionCommand::updateWriteCheck()
{
  if (socket_->wantWrite() || wsSession_->wantWrite()) {
    if (!writeCheck_) {
      writeCheck_ = true;
      e_->addSocketForWriteCheck(socket_, this);
    }
  }
  else if (writeCheck_) {
    writeCheck_ = false;
    e_->deleteSocketForWriteCheck(socket_, this);
  }
}

bool WebSocketInteractionCommand::execute()
{
  if (e_->isHaltRequested()) {
    return true;
  }
  if (wsSession_->onReadEvent() == -1 || wsSession_->onWriteEvent() == -1) {
    if (wsSession_->closeSent() || wsSession_->closeReceived()) {
      A2_LOG_INFO(
          fmt("CUID#%" PRId64 " - WebSocket session terminated.", getCuid()));
    }
    else {
      A2_LOG_INFO(fmt("CUID#%" PRId64 " - WebSocket session terminated"
                      " (Possibly due to EOF).",
                      getCuid()));
    }
    return true;
  }
  if (wsSession_->finish()) {
    return true;
  }
  updateWriteCheck();
  e_->addCommand(std::unique_ptr<Command>(this));
  return false;
}

} // namespace rpc

} // namespace aria2
