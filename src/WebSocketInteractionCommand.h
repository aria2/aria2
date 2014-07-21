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
#ifndef D_WEB_SOCKET_INTERACTION_COMMAND_H
#define D_WEB_SOCKET_INTERACTION_COMMAND_H

#include "Command.h"

#include <memory>

namespace aria2 {

class DownloadEngine;
class SocketCore;

namespace rpc {

class WebSocketSession;

class WebSocketInteractionCommand : public Command {
private:
  DownloadEngine* e_;
  std::shared_ptr<SocketCore> socket_;
  bool writeCheck_;
  std::shared_ptr<WebSocketSession> wsSession_;
public:
  WebSocketInteractionCommand(cuid_t cuid,
                              const std::shared_ptr<WebSocketSession>& wsSession,
                              DownloadEngine* e,
                              const std::shared_ptr<SocketCore>& socket);

  virtual ~WebSocketInteractionCommand();

  virtual bool execute() CXX11_OVERRIDE;

  std::shared_ptr<WebSocketSession>& getSession()
  {
    return wsSession_;
  }

  void updateWriteCheck();
};

} // namespace rpc

} // namespace aria2

#endif // D_WEB_SOCKET_INTERACTION_COMMAND_H
