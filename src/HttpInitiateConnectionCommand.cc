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
#include "HttpInitiateConnectionCommand.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "HttpConnection.h"
#include "HttpRequest.h"
#include "Segment.h"
#include "HttpRequestCommand.h"
#include "HttpProxyRequestCommand.h"
#include "DlAbortEx.h"
#include "Option.h"
#include "Logger.h"
#include "LogFactory.h"
#include "SocketCore.h"
#include "message.h"
#include "prefs.h"
#include "A2STR.h"
#include "util.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "BackupIPv4ConnectCommand.h"
#include "ConnectCommand.h"
#include "HttpRequestConnectChain.h"
#include "HttpProxyRequestConnectChain.h"

namespace aria2 {

HttpInitiateConnectionCommand::HttpInitiateConnectionCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e)
    : InitiateConnectionCommand(cuid, req, fileEntry, requestGroup, e)
{
}

HttpInitiateConnectionCommand::~HttpInitiateConnectionCommand() = default;

std::unique_ptr<Command> HttpInitiateConnectionCommand::createNextCommand(
    const std::string& hostname, const std::string& addr, uint16_t port,
    const std::vector<std::string>& resolvedAddresses,
    const std::shared_ptr<Request>& proxyRequest)
{
  if (proxyRequest) {
    std::shared_ptr<SocketCore> pooledSocket =
        getDownloadEngine()->popPooledSocket(
            getRequest()->getHost(), getRequest()->getPort(),
            proxyRequest->getHost(), proxyRequest->getPort());
    std::string proxyMethod = resolveProxyMethod(getRequest()->getProtocol());
    if (!pooledSocket) {
      A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER, getCuid(), addr.c_str(), port));
      createSocket();
      getSocket()->establishConnection(addr, port);

      getRequest()->setConnectedAddrInfo(hostname, addr, port);
      auto c = make_unique<ConnectCommand>(
          getCuid(), getRequest(), proxyRequest, getFileEntry(),
          getRequestGroup(), getDownloadEngine(), getSocket());
      if (proxyMethod == V_TUNNEL) {
        c->setControlChain(std::make_shared<HttpProxyRequestConnectChain>());
      }
      else if (proxyMethod == V_GET) {
        c->setControlChain(std::make_shared<HttpRequestConnectChain>());
      }
      else {
        // Unreachable
        assert(0);
      }
      setupBackupConnection(hostname, addr, port, c.get());
      return std::move(c);
    }
    else {
      setConnectedAddrInfo(getRequest(), hostname, pooledSocket);
      auto c = make_unique<HttpRequestCommand>(
          getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
          std::make_shared<HttpConnection>(
              getCuid(), pooledSocket,
              std::make_shared<SocketRecvBuffer>(pooledSocket)),
          getDownloadEngine(), pooledSocket);
      if (proxyMethod == V_GET) {
        c->setProxyRequest(proxyRequest);
      }
      return std::move(c);
    }
  }
  else {
    std::shared_ptr<SocketCore> pooledSocket =
        getDownloadEngine()->popPooledSocket(resolvedAddresses,
                                             getRequest()->getPort());
    if (!pooledSocket) {
      A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER, getCuid(), addr.c_str(), port));
      createSocket();
      getSocket()->establishConnection(addr, port);

      getRequest()->setConnectedAddrInfo(hostname, addr, port);
      auto c = make_unique<ConnectCommand>(getCuid(), getRequest(),
                                           proxyRequest, // must be null
                                           getFileEntry(), getRequestGroup(),
                                           getDownloadEngine(), getSocket());
      c->setControlChain(std::make_shared<HttpRequestConnectChain>());
      setupBackupConnection(hostname, addr, port, c.get());
      return std::move(c);
    }
    else {
      setSocket(pooledSocket);
      setConnectedAddrInfo(getRequest(), hostname, pooledSocket);

      return make_unique<HttpRequestCommand>(
          getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
          std::make_shared<HttpConnection>(
              getCuid(), getSocket(),
              std::make_shared<SocketRecvBuffer>(getSocket())),
          getDownloadEngine(), getSocket());
    }
  }
}

} // namespace aria2
