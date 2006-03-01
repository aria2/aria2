/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_HTTP_CONNECTION_H_
#define _D_HTTP_CONNECTION_H_

#include "Segment.h"
#include "Socket.h"
#include "Request.h"
#include "Option.h"
#include "Logger.h"
#include "HttpHeader.h"
#include "common.h"
#include <string>

using namespace std;

class HttpConnection {
private:
  string getHost(const string& host, int port) const;
  string createRequest(const Segment& segment) const;
  bool useProxy() const;
  bool useProxyAuth() const;
  bool useProxyGet() const;
  string getProxyAuthString() const;
  int cuid;
  const Socket* socket;
  const Request* req;
  const Option* option;
  const Logger* logger;
  string header;
public:
  HttpConnection(int cuid, const Socket* socket, const Request* req, const Option* op, const Logger* logger);

  /**
   * Sends Http request.
   * If segment.sp+segment.ds > 0 then Range header is added.
   * This method is used in HTTP/HTTP downloading and FTP downloading via
   * HTTP proxy(GET method).
   * @param segment indicates starting postion of the file for downloading
   */
  void sendRequest(const Segment& segment) const;

  /**
   * Sends Http proxy request using CONNECT method.
   */
  void sendProxyRequest() const;

  /**
   * Receives HTTP response from the server and store the response header
   * into the variable headers.
   * If response header is not fully received, received header is buffured
   * in this object and headers is undefined and this method returns 0. 
   * You should continue to call this method until whole response header is
   * received and this method returns non-zero value.
   * 
   * @param headers holder to store HTTP response header
   * @return HTTP status or 0 if whole response header is not received
   */
  int receiveResponse(HttpHeader& headers);
};

#endif // _D_HTTP_CONNECTION_H_
