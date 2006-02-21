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

#include "SegmentMan.h"
#include "Socket.h"
#include "Request.h"
#include "Option.h"
#include "Logger.h"
#include "HttpHeader.h"
#include <map>
#include <string>

using namespace std;

//typedef multimap<string, string> HttpHeader;

class HttpConnection {
private:
  string getHost(const string& host, int port) const;
  string createRequest(const Segment& segment) const;
  bool useProxy() const;
  bool useProxyAuth() const;
  int cuid;
  const Socket* socket;
  const Request* req;
  const Option* option;
  const Logger* logger;
  string header;
public:
  HttpConnection(int cuid, const Socket* socket, const Request* req, const Option* op, const Logger* logger);

  void sendRequest(const Segment& segment) const;
  void sendProxyRequest() const;
  int receiveResponse(HttpHeader& headers);
};

#endif // _D_HTTP_CONNECTION_H_
