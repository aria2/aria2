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
#ifndef D_SOCKS_PROXY_COMMAND_H
#define D_SOCKS_PROXY_COMMAND_H

#include "AbstractCommand.h"

#include <vector>
#include <functional>

namespace aria2 {

class SocketRecvBuffer;

class SocksProxyCommand : public AbstractCommand {
public:
  using NextCommandFactory =
      std::function<std::unique_ptr<Command>(SocksProxyCommand*)>;

  SocksProxyCommand(cuid_t cuid, const std::shared_ptr<Request>& req,
                    const std::shared_ptr<FileEntry>& fileEntry,
                    RequestGroup* requestGroup, DownloadEngine* e,
                    const std::shared_ptr<Request>& proxyRequest,
                    const std::shared_ptr<SocketCore>& s,
                    NextCommandFactory nextCommandFactory);

  virtual ~SocksProxyCommand();

protected:
  virtual bool executeInternal() CXX11_OVERRIDE;

private:
  enum State {
    SOCKS_GREETING_SEND,
    SOCKS_GREETING_RECV,
    SOCKS_AUTH_SEND,
    SOCKS_AUTH_RECV,
    SOCKS_CONNECT_SEND,
    SOCKS_CONNECT_RECV,
  };

  bool sendData();
  bool recvData(size_t expect);
  void buildGreeting();
  void buildAuth();
  void buildConnect();
  void processGreetingResponse();
  void processAuthResponse();
  bool processConnectResponse();

  std::shared_ptr<Request> proxyRequest_;
  std::shared_ptr<SocketRecvBuffer> recvBuffer_;
  NextCommandFactory nextCommandFactory_;
  State state_;
  std::vector<unsigned char> sendBuf_;
  size_t sendOffset_;
};

} // namespace aria2

#endif // D_SOCKS_PROXY_COMMAND_H
