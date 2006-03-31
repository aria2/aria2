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
#include "HttpProxyResponseCommand.h"
#include "HttpRequestCommand.h"
#include "DlRetryEx.h"
#include "message.h"

HttpProxyResponseCommand::HttpProxyResponseCommand(int cuid, Request* req, DownloadEngine* e, const Socket* s):AbstractCommand(cuid, req, e, s) {
  http = new HttpConnection(cuid, socket, req, e->option, e->logger);
}

HttpProxyResponseCommand::~HttpProxyResponseCommand() {
  delete http;
}

bool HttpProxyResponseCommand::executeInternal(Segment segment) {
  HttpHeader headers;
  int status = http->receiveResponse(headers);
  if(status == 0) {
    // we didn't receive all of headers yet.
    e->commands.push(this);
    return false;
  }
  if(status != 200) {
    throw new DlRetryEx(EX_PROXY_CONNECTION_FAILED);
  }
  HttpRequestCommand* command = new HttpRequestCommand(cuid, req, e, socket);
  e->commands.push(command);
  return true;
}

