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
#include "HttpRequestCommand.h"

#include <algorithm>

#include "Request.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "HttpResponseCommand.h"
#include "HttpConnection.h"
#include "HttpRequest.h"
#include "SegmentMan.h"
#include "Segment.h"
#include "Option.h"
#include "Socket.h"
#include "prefs.h"
#include "a2functional.h"
#include "util.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "DownloadContext.h"

namespace aria2 {

HttpRequestCommand::HttpRequestCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 const HttpConnectionHandle& httpConnection,
 DownloadEngine* e,
 const SocketHandle& s)
  :AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
   _httpConnection(httpConnection)
{
  setTimeout(getOption()->getAsInt(PREF_CONNECT_TIMEOUT));
  disableReadCheckSocket();
  setWriteCheckSocket(socket);
}

HttpRequestCommand::~HttpRequestCommand() {}

static SharedHandle<HttpRequest>
createHttpRequest(const SharedHandle<Request>& req,
                  const SharedHandle<FileEntry>& fileEntry,
                  const SharedHandle<Segment>& segment,
                  uint64_t totalLength,
                  const SharedHandle<Option>& option,
                  const RequestGroup* rg,
                  const SharedHandle<CookieStorage>& cookieStorage,
                  const SharedHandle<AuthConfigFactory>& authConfigFactory,
                  const SharedHandle<Request>& proxyRequest)
{
  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  httpRequest->setUserAgent(option->get(PREF_USER_AGENT));
  httpRequest->setRequest(req);
  httpRequest->setFileEntry(fileEntry);
  httpRequest->setSegment(segment);
  httpRequest->addHeader(option->get(PREF_HEADER));
  httpRequest->setCookieStorage(cookieStorage);
  httpRequest->setAuthConfigFactory(authConfigFactory, option.get());
  httpRequest->setProxyRequest(proxyRequest);
  httpRequest->addAcceptType(rg->getAcceptTypes().begin(),
                             rg->getAcceptTypes().end());
  if(option->getAsBool(PREF_HTTP_ACCEPT_GZIP)) {
    httpRequest->enableAcceptGZip();
  } else {
    httpRequest->disableAcceptGZip();
  }
  if(option->getAsBool(PREF_HTTP_NO_CACHE)) {
    httpRequest->enableNoCache();
  } else {
    httpRequest->disableNoCache();
  }
  return httpRequest;
}

bool HttpRequestCommand::executeInternal() {
  //socket->setBlockingMode();
  if(req->getProtocol() == Request::PROTO_HTTPS) {
    socket->prepareSecureConnection();
    if(!socket->initiateSecureConnection(req->getHost())) {
      setReadCheckSocketIf(socket, socket->wantRead());
      setWriteCheckSocketIf(socket, socket->wantWrite());
      e->commands.push_back(this);
      return false;
    }
  }
  if(_httpConnection->sendBufferIsEmpty()) {
    if(!checkIfConnectionEstablished
       (socket, _connectedHostname, _connectedAddr, _connectedPort)) {
      return true;
    }

    if(_segments.empty()) {
      SharedHandle<HttpRequest> httpRequest
        (createHttpRequest(req,
                           _fileEntry,
                           SharedHandle<Segment>(),
                           _requestGroup->getTotalLength(),
                           getOption(),
                           _requestGroup,
                           e->getCookieStorage(),
                           e->getAuthConfigFactory(),
                           _proxyRequest));
      _httpConnection->sendRequest(httpRequest);
    } else {
      for(std::vector<SharedHandle<Segment> >::const_iterator itr =
            _segments.begin(), eoi = _segments.end(); itr != eoi; ++itr) {
        const SharedHandle<Segment>& segment = *itr;
        if(!_httpConnection->isIssued(segment)) {
          SharedHandle<HttpRequest> httpRequest
            (createHttpRequest(req,
                               _fileEntry,
                               segment,
                               _requestGroup->getTotalLength(),
                               getOption(),
                               _requestGroup,
                               e->getCookieStorage(),
                               e->getAuthConfigFactory(),
                               _proxyRequest));
          _httpConnection->sendRequest(httpRequest);
        }
      }
    }
  } else {
    _httpConnection->sendPendingData();
  }
  if(_httpConnection->sendBufferIsEmpty()) {
    Command* command = new HttpResponseCommand(cuid, req, _fileEntry,
                                               _requestGroup,
                                               _httpConnection, e, socket);
    e->commands.push_back(command);
    return true;
  } else {
    setReadCheckSocketIf(socket, socket->wantRead());
    setWriteCheckSocketIf(socket, socket->wantWrite());
    e->commands.push_back(this);
    return false;
  }
}

void HttpRequestCommand::setProxyRequest
(const SharedHandle<Request>& proxyRequest)
{
  _proxyRequest = proxyRequest;
}

} // namespace aria2
