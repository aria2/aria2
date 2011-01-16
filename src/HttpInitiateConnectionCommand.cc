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
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "A2STR.h"
#include "util.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"

namespace aria2 {

HttpInitiateConnectionCommand::HttpInitiateConnectionCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 DownloadEngine* e)
  : InitiateConnectionCommand(cuid, req, fileEntry, requestGroup, e)
{}

HttpInitiateConnectionCommand::~HttpInitiateConnectionCommand() {}

Command* HttpInitiateConnectionCommand::createNextCommand
(const std::string& hostname, const std::string& addr, uint16_t port,
 const std::vector<std::string>& resolvedAddresses,
 const SharedHandle<Request>& proxyRequest)
{
  Command* command;
  if(proxyRequest) {
    SharedHandle<SocketCore> pooledSocket =
      getDownloadEngine()->popPooledSocket
      (getRequest()->getHost(), getRequest()->getPort(),
       proxyRequest->getHost(), proxyRequest->getPort());
    std::string proxyMethod = resolveProxyMethod(getRequest()->getProtocol());
    if(!pooledSocket) {
      A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                      getCuid(), addr.c_str(), port));
      createSocket();
      getSocket()->establishConnection(addr, port);

      getRequest()->setConnectedAddrInfo(hostname, addr, port);
      if(proxyMethod == V_TUNNEL) {
        HttpProxyRequestCommand* c =
          new HttpProxyRequestCommand(getCuid(),
                                      getRequest(),
                                      getFileEntry(),
                                      getRequestGroup(),
                                      getDownloadEngine(),
                                      proxyRequest,
                                      getSocket());
        command = c;
      } else if(proxyMethod == V_GET) {
        SharedHandle<SocketRecvBuffer> socketRecvBuffer
          (new SocketRecvBuffer(getSocket()));
        SharedHandle<HttpConnection> httpConnection
          (new HttpConnection(getCuid(), getSocket(), socketRecvBuffer));
        HttpRequestCommand* c = new HttpRequestCommand(getCuid(),
                                                       getRequest(),
                                                       getFileEntry(),
                                                       getRequestGroup(),
                                                       httpConnection,
                                                       getDownloadEngine(),
                                                       getSocket());
        c->setProxyRequest(proxyRequest);
        command = c;
      } else {
        // TODO
        throw DL_ABORT_EX("ERROR");
      }
    } else {
      setConnectedAddrInfo(getRequest(), hostname, pooledSocket);
      SharedHandle<SocketRecvBuffer> socketRecvBuffer
        (new SocketRecvBuffer(pooledSocket));
      SharedHandle<HttpConnection> httpConnection
        (new HttpConnection(getCuid(), pooledSocket, socketRecvBuffer));
      HttpRequestCommand* c = new HttpRequestCommand(getCuid(),
                                                     getRequest(),
                                                     getFileEntry(),
                                                     getRequestGroup(),
                                                     httpConnection,
                                                     getDownloadEngine(),
                                                     pooledSocket);
      if(proxyMethod == V_GET) {
        c->setProxyRequest(proxyRequest);
      }
      command = c;
    }
  } else {
    SharedHandle<SocketCore> pooledSocket =
      getDownloadEngine()->popPooledSocket
      (resolvedAddresses, getRequest()->getPort());
    if(!pooledSocket) {
      A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                      getCuid(), addr.c_str(), port));
      createSocket();
      getSocket()->establishConnection(addr, port);
      getRequest()->setConnectedAddrInfo(hostname, addr, port);
    } else {
      setSocket(pooledSocket);
      setConnectedAddrInfo(getRequest(), hostname, pooledSocket);
    }
    SharedHandle<SocketRecvBuffer> socketRecvBuffer
      (new SocketRecvBuffer(getSocket()));
    SharedHandle<HttpConnection> httpConnection
      (new HttpConnection(getCuid(), getSocket(), socketRecvBuffer));
    HttpRequestCommand* c =
      new HttpRequestCommand(getCuid(), getRequest(), getFileEntry(),
                             getRequestGroup(),
                             httpConnection,
                             getDownloadEngine(),
                             getSocket());
    command = c;
  }
  return command;
}

} // namespace aria2
