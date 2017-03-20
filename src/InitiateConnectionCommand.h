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
#ifndef D_INITIATE_CONNECTION_COMMAND_H
#define D_INITIATE_CONNECTION_COMMAND_H

#include "AbstractCommand.h"

namespace aria2 {

struct BackupConnectInfo;
class ConnectCommand;

class InitiateConnectionCommand : public AbstractCommand {
protected:
  /**
   * Connect to the server.
   * This method just send connection request to the server.
   * Using nonblocking mode of socket, this function returns immediately
   * after send connection packet to the server.
   */
  virtual bool executeInternal() CXX11_OVERRIDE;

  // hostname and port are the hostname and port number we are going
  // to connect. If proxy server is used, these values are hostname
  // and port of proxy server. addr is one of resolved address and we
  // use this address this time.  resolvedAddresses are all addresses
  // resolved.  proxyRequest is set if we are going to use proxy
  // server.
  virtual std::unique_ptr<Command>
  createNextCommand(const std::string& hostname, const std::string& addr,
                    uint16_t port,
                    const std::vector<std::string>& resolvedAddresses,
                    const std::shared_ptr<Request>& proxyRequest) = 0;

  void setConnectedAddrInfo(const std::shared_ptr<Request>& req,
                            const std::string& hostname,
                            const std::shared_ptr<SocketCore>& socket);

  std::shared_ptr<BackupConnectInfo>
  createBackupIPv4ConnectCommand(const std::string& hostname,
                                 const std::string& ipaddr, uint16_t port,
                                 Command* mainCommand);

  void setupBackupConnection(const std::string& hostname,
                             const std::string& addr, uint16_t port,
                             ConnectCommand* c);

public:
  InitiateConnectionCommand(cuid_t cuid, const std::shared_ptr<Request>& req,
                            const std::shared_ptr<FileEntry>& fileEntry,
                            RequestGroup* requestGroup, DownloadEngine* e);

  virtual ~InitiateConnectionCommand();
};

} // namespace aria2

#endif // D_INITIATE_CONNECTION_COMMAND_H
