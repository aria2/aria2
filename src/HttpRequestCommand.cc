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
#include "HttpRequestCommand.h"
#include "HttpResponseCommand.h"
#include "HttpInitiateConnectionCommand.h"
#include "Socket.h"
#include "Util.h"
#include "HttpConnection.h"

HttpRequestCommand::HttpRequestCommand(int cuid, Request* req, DownloadEngine* e, Socket* s):AbstractCommand(cuid, req, e, s) {
  AbstractCommand::checkSocketIsWritable = true;
  e->deleteSocketForReadCheck(socket);
  e->addSocketForWriteCheck(socket);
}

HttpRequestCommand::~HttpRequestCommand() {}

bool HttpRequestCommand::executeInternal(Segment seg) {
  socket->setNonBlockingMode();
  HttpConnection httpConnection(cuid, socket, e->option, e->logger);
  // set seg to request in order to remember the request range
  req->seg = seg;
  httpConnection.sendRequest(req, seg);

  HttpResponseCommand* command = new HttpResponseCommand(cuid, req, e, socket);
  e->commands.push(command);
  return true;
}
