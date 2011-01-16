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
#include "AbstractProxyResponseCommand.h"
#include "HttpConnection.h"
#include "Request.h"
#include "Segment.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpRequestCommand.h"
#include "Socket.h"
#include "DlRetryEx.h"
#include "message.h"
#include "HttpHeader.h"
#include "DownloadContext.h"
#include "SocketRecvBuffer.h"

namespace aria2 {

AbstractProxyResponseCommand::AbstractProxyResponseCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 const HttpConnectionHandle& httpConnection,
 DownloadEngine* e,
 const SocketHandle& s)
  :AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
   httpConnection_(httpConnection) {}

AbstractProxyResponseCommand::~AbstractProxyResponseCommand() {}

bool AbstractProxyResponseCommand::executeInternal() {
  SharedHandle<HttpResponse> httpResponse = httpConnection_->receiveResponse();
  if(!httpResponse) {
    // the server has not responded our request yet.
    getDownloadEngine()->addCommand(this);
    return false;
  }
  if(httpResponse->getStatusCode() != 200) {
    throw DL_RETRY_EX(EX_PROXY_CONNECTION_FAILED);
  }
  getDownloadEngine()->addCommand(getNextCommand());
  return true;
}

} // namespace aria2
