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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Util.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"

namespace aria2 {

HttpRequestCommand::HttpRequestCommand
(int cuid,
 const RequestHandle& req,
 RequestGroup* requestGroup,
 const HttpConnectionHandle& httpConnection,
 DownloadEngine* e,
 const SocketHandle& s)
  :AbstractCommand(cuid, req, requestGroup, e, s),
   _httpConnection(httpConnection)
{
  setTimeout(e->option->getAsInt(PREF_CONNECT_TIMEOUT));
  disableReadCheckSocket();
  setWriteCheckSocket(socket);
}

HttpRequestCommand::~HttpRequestCommand() {}

static SharedHandle<HttpRequest>
createHttpRequest(const SharedHandle<Request>& req,
		  const SharedHandle<Segment>& segment,
		  uint64_t totalLength,
		  const Option* option,
		  const RequestGroup* rg,
		  const SharedHandle<CookieStorage>& cookieStorage,
		  const SharedHandle<AuthConfigFactory>& authConfigFactory)
{
  HttpRequestHandle httpRequest(new HttpRequest());
  httpRequest->setUserAgent(option->get(PREF_USER_AGENT));
  httpRequest->setRequest(req);
  httpRequest->setSegment(segment);
  httpRequest->setEntityLength(totalLength);
  httpRequest->addHeader(option->get(PREF_HEADER));
  httpRequest->setCookieStorage(cookieStorage);
  httpRequest->setAuthConfigFactory(authConfigFactory);
  if(!rg->getAcceptFeatures().empty()) {
    const std::deque<std::string>& acceptFeatures = rg->getAcceptFeatures();
    std::string acceptFeaturesHeader = "Accept-Features: ";
    std::for_each(acceptFeatures.begin(), acceptFeatures.end(),
		  StringAppend(acceptFeaturesHeader, ","));
    httpRequest->addHeader(Util::trim(acceptFeaturesHeader, ","));
  }
  httpRequest->addAcceptType(rg->getAcceptTypes().begin(),
			     rg->getAcceptTypes().end());
  httpRequest->configure(option);

  return httpRequest;
}

bool HttpRequestCommand::executeInternal() {
  //socket->setBlockingMode();
  if(req->getProtocol() == Request::PROTO_HTTPS) {
    socket->prepareSecureConnection();
    if(!socket->initiateSecureConnection()) {
      setReadCheckSocketIf(socket, socket->wantRead());
      setWriteCheckSocketIf(socket, socket->wantWrite());
      e->commands.push_back(this);
      return false;
    }
  }
  if(_httpConnection->sendBufferIsEmpty()) {
    checkIfConnectionEstablished(socket);

    if(_segments.empty()) {
      HttpRequestHandle httpRequest
	(createHttpRequest(req, SharedHandle<Segment>(),
			   _requestGroup->getTotalLength(), e->option,
			   _requestGroup,
			   e->getCookieStorage(),
			   e->getAuthConfigFactory()));
      _httpConnection->sendRequest(httpRequest);
    } else {
      for(Segments::iterator itr = _segments.begin(); itr != _segments.end(); ++itr) {
	const SegmentHandle& segment = *itr;
	if(!_httpConnection->isIssued(segment)) {
	  HttpRequestHandle httpRequest
	    (createHttpRequest(req, segment,
			       _requestGroup->getTotalLength(), e->option,
			       _requestGroup,
			       e->getCookieStorage(),
			       e->getAuthConfigFactory()));
	  _httpConnection->sendRequest(httpRequest);
	}
      }
    }
  } else {
    _httpConnection->sendPendingData();
  }
  if(_httpConnection->sendBufferIsEmpty()) {
    Command* command = new HttpResponseCommand(cuid, req, _requestGroup,
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

} // namespace aria2
