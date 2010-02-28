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
#include "message.h"
#include "prefs.h"
#include "HttpConnection.h"
#include "Socket.h"
#include "DownloadContext.h"

namespace aria2 {

FtpInitiateConnectionCommand::FtpInitiateConnectionCommand
(int cuid,
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
  if(!proxyRequest.isNull()) {
    std::map<std::string, std::string> options;
    SharedHandle<SocketCore> pooledSocket =
      e->popPooledSocket(options, req->getHost(), req->getPort());
    std::string proxyMethod = resolveProxyMethod(req->getProtocol());
    if(pooledSocket.isNull()) {
      logger->info(MSG_CONNECTING_TO_SERVER, cuid, addr.c_str(), port);
      socket.reset(new SocketCore());
      socket->establishConnection(addr, port);
      
      if(proxyMethod == V_GET) {
        // Use GET for FTP via HTTP proxy.
        req->setMethod(Request::METHOD_GET);
        SharedHandle<HttpConnection> hc
          (new HttpConnection(cuid, socket, getOption().get()));
        
        HttpRequestCommand* c =
          new HttpRequestCommand(cuid, req, _fileEntry,
                                 _requestGroup, hc, e, socket);
        c->setConnectedAddr(hostname, addr, port);
        c->setProxyRequest(proxyRequest);
        command = c;
      } else if(proxyMethod == V_TUNNEL) {
        FtpTunnelRequestCommand* c =
          new FtpTunnelRequestCommand(cuid, req, _fileEntry,
                                      _requestGroup, e,
                                      proxyRequest, socket);
        c->setConnectedAddr(hostname, addr, port);
        command = c;
      } else {
        // TODO
        throw DL_ABORT_EX("ERROR");
      }
    } else {
      if(proxyMethod == V_TUNNEL) {
        command =
          new FtpNegotiationCommand(cuid, req, _fileEntry,
                                    _requestGroup, e, pooledSocket,
                                    FtpNegotiationCommand::SEQ_SEND_CWD,
                                    options["baseWorkingDir"]);
      } else if(proxyMethod == V_GET) {
        // Use GET for FTP via HTTP proxy.
        req->setMethod(Request::METHOD_GET);
        SharedHandle<HttpConnection> hc
          (new HttpConnection(cuid, pooledSocket, getOption().get()));
        
        HttpRequestCommand* c =
          new HttpRequestCommand(cuid, req, _fileEntry,
                                 _requestGroup, hc, e, pooledSocket);
        c->setProxyRequest(proxyRequest);
        command = c;
      } else {
        // TODO
        throw DL_ABORT_EX("ERROR");
      }
    }
  } else {
    std::map<std::string, std::string> options;
    SharedHandle<SocketCore> pooledSocket =
      e->popPooledSocket(options, resolvedAddresses, req->getPort());
    if(pooledSocket.isNull()) {
      logger->info(MSG_CONNECTING_TO_SERVER, cuid, addr.c_str(), port);
      socket.reset(new SocketCore());
      socket->establishConnection(addr, port);
      FtpNegotiationCommand* c =
        new FtpNegotiationCommand(cuid, req, _fileEntry,
                                  _requestGroup, e, socket);
      c->setConnectedAddr(hostname, addr, port);
      command = c;
    } else {
      command =
        new FtpNegotiationCommand(cuid, req, _fileEntry,
                                  _requestGroup, e, pooledSocket,
                                  FtpNegotiationCommand::SEQ_SEND_CWD,
                                  options["baseWorkingDir"]);
    }
  }
  return command;
}

} // namespace aria2
