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
#ifndef _D_HTTP_CONNECTION_H_
#define _D_HTTP_CONNECTION_H_

#include "Segment.h"
#include "Socket.h"
#include "Request.h"
#include "Option.h"
#include "Logger.h"
#include "common.h"
#include "Logger.h"
#include "HttpResponse.h"
#include "HttpHeaderProcessor.h"
#include <netinet/in.h>
#include <string>

class HttpRequestEntry {
private:
  HttpRequestHandle _httpRequest;
  HttpHeaderProcessorHandle _proc;
public:
  HttpRequestEntry(const HttpRequestHandle& httpRequest,
		   const HttpHeaderProcessorHandle& proc):
    _httpRequest(httpRequest),
    _proc(proc) {}

  ~HttpRequestEntry() {}

  HttpRequestHandle getHttpRequest() const
  {
    return _httpRequest;
  }

  HttpHeaderProcessorHandle getHttpHeaderProcessor() const
  {
    return _proc;
  }
};

typedef SharedHandle<HttpRequestEntry> HttpRequestEntryHandle;
typedef deque<HttpRequestEntryHandle> HttpRequestEntries;

class HttpConnection {
private:
  int32_t cuid;
  SocketHandle socket;
  const Option* option;
  const Logger* logger;

  HttpRequestEntries outstandingHttpRequests;

  string eraseConfidentialInfo(const string& request);
public:
  HttpConnection(int32_t cuid,
		 const SocketHandle& socket,
		 const Option* op);

  /**
   * Sends Http request.
   * If segment.sp+segment.ds > 0 then Range header is added.
   * This method is used in HTTP/HTTP downloading and FTP downloading via
   * HTTP proxy(GET method).
   * @param segment indicates starting postion of the file for downloading
   */
  void sendRequest(const HttpRequestHandle& httpRequest);

  /**
   * Sends Http proxy request using CONNECT method.
   */
  void sendProxyRequest(const HttpRequestHandle& httpRequest);

  /**
   * Receives HTTP response from the server and returns HttpResponseHandle
   * object which contains response header and HttpRequestHandle object
   * for this response.
   * If a response is not fully received, received header is buffured
   * in this object and returns 0. 
   * You should continue to call this method until whole response header is
   * received and this method returns non-null HttpResponseHandle object.
   * 
   * @return HttpResponse or 0 if whole response header is not received
   */
  HttpResponseHandle receiveResponse();

  HttpRequestHandle getFirstHttpRequest() const
  {
    if(outstandingHttpRequests.size() > 0) {
      return outstandingHttpRequests.front()->getHttpRequest();
    } else {
      return 0;
    }
  }
};

typedef SharedHandle<HttpConnection> HttpConnectionHandle;

#endif // _D_HTTP_CONNECTION_H_
