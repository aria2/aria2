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
#include "FtpTunnelResponseCommand.h"
#include "FtpNegotiationCommand.h"
#include "DlRetryEx.h"
#include "message.h"

FtpTunnelResponseCommand::FtpTunnelResponseCommand(int cuid, Request* req,
						   DownloadEngine* e,
						   const SocketHandle& s)
  :AbstractCommand(cuid, req, e, s) {
  http = new HttpConnection(cuid, socket, req, e->option);
}

FtpTunnelResponseCommand::~FtpTunnelResponseCommand() {
  delete http;
}

bool FtpTunnelResponseCommand::executeInternal(Segment& segment) {
  HttpHeader headers;
  int status = http->receiveResponse(headers);
  if(status == 0) {
    // we didn't receive all of headers yet.
    e->commands.push_back(this);
    return false;
  }
  if(status != 200) {
    throw new DlRetryEx(EX_PROXY_CONNECTION_FAILED);
  }
  FtpNegotiationCommand* command
    = new FtpNegotiationCommand(cuid, req, e, socket);
  e->commands.push_back(command);
  return true;
}
