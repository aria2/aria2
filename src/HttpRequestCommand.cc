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

namespace aria2 {

HttpRequestCommand::HttpRequestCommand(int cuid,
				       const RequestHandle& req,
				       RequestGroup* requestGroup,
				       const HttpConnectionHandle& httpConnection,
				       DownloadEngine* e,
				       const SocketHandle& s)
  :AbstractCommand(cuid, req, requestGroup, e, s),
   _httpConnection(httpConnection)
{
  disableReadCheckSocket();
  setWriteCheckSocket(socket);
}

HttpRequestCommand::~HttpRequestCommand() {}

bool HttpRequestCommand::executeInternal() {
  socket->setBlockingMode();
  if(req->getProtocol() == "https") {
    socket->initiateSecureConnection();
  }
  if(e->option->get(PREF_ENABLE_HTTP_PIPELINING) == V_TRUE) {
    req->setKeepAlive(true);
  } else if(e->option->get(PREF_ENABLE_HTTP_KEEP_ALIVE) == V_TRUE &&
	    !_requestGroup->getSegmentMan().isNull() &&
	    _requestGroup->getSegmentMan()->countFreePieceFrom(_segments.front()->getIndex()+1) <= 4) {
    // TODO Do we need to consider the case where content-length is unknown?
    // TODO parameterize the value which enables keep-alive, '4'
    req->setKeepAlive(true);
  } else {
    req->setKeepAlive(false);
  }

  if(_segments.empty()) {
    HttpRequestHandle httpRequest(new HttpRequest());
    httpRequest->setUserAgent(e->option->get(PREF_USER_AGENT));
    httpRequest->setRequest(req);
    httpRequest->setEntityLength(_requestGroup->getTotalLength());
    httpRequest->configure(e->option);
    
    _httpConnection->sendRequest(httpRequest);
  } else {
    for(Segments::iterator itr = _segments.begin(); itr != _segments.end(); ++itr) {
      SegmentHandle segment = *itr;
      if(!_httpConnection->isIssued(segment)) {
	HttpRequestHandle httpRequest(new HttpRequest());
	httpRequest->setUserAgent(e->option->get(PREF_USER_AGENT));
	httpRequest->setRequest(req);
	httpRequest->setSegment(segment);
	httpRequest->setEntityLength(_requestGroup->getTotalLength());
	httpRequest->configure(e->option);

	_httpConnection->sendRequest(httpRequest);
      }
    }
  }
  Command* command = new HttpResponseCommand(cuid, req, _requestGroup, _httpConnection, e, socket);
  e->commands.push_back(command);
  return true;
}

} // namespace aria2
