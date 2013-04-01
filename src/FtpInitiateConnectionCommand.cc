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
#include "FtpInitiateConnectionCommand.h"

#include <map>

#include "DownloadEngine.h"
#include "Option.h"
#include "Request.h"
#include "FtpNegotiationCommand.h"
#include "HttpRequest.h"
#include "Segment.h"
#include "HttpRequestCommand.h"
#include "FtpTunnelRequestCommand.h"
#include "DlAbortEx.h"
#include "Logger.h"
#include "LogFactory.h"
#include "message.h"
#include "prefs.h"
#include "HttpConnection.h"
#include "SocketCore.h"
#include "util.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "BackupIPv4ConnectCommand.h"
#include "FtpNegotiationConnectChain.h"
#include "FtpTunnelRequestConnectChain.h"
#include "HttpRequestConnectChain.h"

namespace aria2 {

FtpInitiateConnectionCommand::FtpInitiateConnectionCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 DownloadEngine* e)
  :InitiateConnectionCommand(cuid, req, fileEntry, requestGroup, e) {}

FtpInitiateConnectionCommand::~FtpInitiateConnectionCommand() {}

Command* FtpInitiateConnectionCommand::createNextCommand
(const std::string& hostname, const std::string& addr, uint16_t port,
 const std::vector<std::string>& resolvedAddresses,
 const SharedHandle<Request>& proxyRequest)
{
  Command* command;
  if(proxyRequest) {
    std::string options;
    SharedHandle<SocketCore> pooledSocket;
    std::string proxyMethod = resolveProxyMethod(getRequest()->getProtocol());
    if(proxyMethod == V_GET) {
      pooledSocket = getDownloadEngine()->popPooledSocket
        (getRequest()->getHost(), getRequest()->getPort(),
         proxyRequest->getHost(), proxyRequest->getPort());
    } else {
      pooledSocket = getDownloadEngine()->popPooledSocket
        (options, getRequest()->getHost(), getRequest()->getPort(),
         getDownloadEngine()->getAuthConfigFactory()->createAuthConfig
         (getRequest(), getOption().get())->getUser(),
         proxyRequest->getHost(), proxyRequest->getPort());
    }
    if(!pooledSocket) {
      A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                      getCuid(), addr.c_str(), port));
      createSocket();
      getSocket()->establishConnection(addr, port);

      getRequest()->setConnectedAddrInfo(hostname, addr, port);

      ConnectCommand* c = new ConnectCommand(getCuid(),
                                             getRequest(),
                                             proxyRequest,
                                             getFileEntry(),
                                             getRequestGroup(),
                                             getDownloadEngine(),
                                             getSocket());
      if(proxyMethod == V_GET) {
        // Use GET for FTP via HTTP proxy.
        getRequest()->setMethod(Request::METHOD_GET);
        SharedHandle<HttpRequestConnectChain> chain
          (new HttpRequestConnectChain());
        c->setControlChain(chain);
      } else if(proxyMethod == V_TUNNEL) {
        SharedHandle<FtpTunnelRequestConnectChain> chain
          (new FtpTunnelRequestConnectChain());
        c->setControlChain(chain);
      } else {
        // Unreachable
        assert(0);
      }
      setupBackupConnection(hostname, addr, port, c);
      command = c;
    } else {
      setConnectedAddrInfo(getRequest(), hostname, pooledSocket);
      if(proxyMethod == V_TUNNEL) {
        // options contains "baseWorkingDir"
        command = new FtpNegotiationCommand
          (getCuid(),
           getRequest(),
           getFileEntry(),
           getRequestGroup(),
           getDownloadEngine(),
           pooledSocket,
           FtpNegotiationCommand::SEQ_SEND_CWD_PREP,
           options);
      } else if(proxyMethod == V_GET) {
        // Use GET for FTP via HTTP proxy.
        getRequest()->setMethod(Request::METHOD_GET);
        SharedHandle<SocketRecvBuffer> socketRecvBuffer
          (new SocketRecvBuffer(pooledSocket));
        SharedHandle<HttpConnection> hc
          (new HttpConnection(getCuid(), pooledSocket, socketRecvBuffer));

        HttpRequestCommand* c = new HttpRequestCommand(getCuid(),
                                                       getRequest(),
                                                       getFileEntry(),
                                                       getRequestGroup(),
                                                       hc,
                                                       getDownloadEngine(),
                                                       pooledSocket);
        c->setProxyRequest(proxyRequest);
        command = c;
      } else {
        // Unreachable
        assert(0);
      }
    }
  } else {
    std::string options;
    SharedHandle<SocketCore> pooledSocket =
      getDownloadEngine()->popPooledSocket
      (options, resolvedAddresses,
       getRequest()->getPort(),
       getDownloadEngine()->getAuthConfigFactory()->createAuthConfig
       (getRequest(), getOption().get())->getUser());
    if(!pooledSocket) {
      A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                      getCuid(), addr.c_str(), port));
      createSocket();
      getSocket()->establishConnection(addr, port);
      getRequest()->setConnectedAddrInfo(hostname, addr, port);
      ConnectCommand* c = new ConnectCommand(getCuid(),
                                             getRequest(),
                                             proxyRequest, // must be null
                                             getFileEntry(),
                                             getRequestGroup(),
                                             getDownloadEngine(),
                                             getSocket());
      SharedHandle<FtpNegotiationConnectChain> chain
        (new FtpNegotiationConnectChain());
      c->setControlChain(chain);
      setupBackupConnection(hostname, addr, port, c);
      command = c;
    } else {
      // options contains "baseWorkingDir"
      command = new FtpNegotiationCommand
        (getCuid(),
         getRequest(),
         getFileEntry(),
         getRequestGroup(),
         getDownloadEngine(),
         pooledSocket,
         FtpNegotiationCommand::SEQ_SEND_CWD_PREP,
         options);
      setConnectedAddrInfo(getRequest(), hostname, pooledSocket);
    }
  }
  return command;
}

} // namespace aria2
