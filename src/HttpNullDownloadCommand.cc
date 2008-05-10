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
#include "HttpNullDownloadCommand.h"
#include "HttpConnection.h"
#include "HttpResponse.h"
#include "message.h"
#include "SocketCore.h"
#include "TransferEncoding.h"
#include "DlRetryEx.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "Logger.h"
#include "HttpRequest.h"
#include "Segment.h"
#include "Util.h"
#include "StringFormat.h"
#include "DlAbortEx.h"

namespace aria2 {

HttpNullDownloadCommand::HttpNullDownloadCommand
(int cuid,
 const SharedHandle<Request>& req,
 RequestGroup* requestGroup,
 const SharedHandle<HttpConnection>& httpConnection,
 const SharedHandle<HttpResponse>& httpResponse,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& s):
  AbstractCommand(cuid, req, requestGroup, e, s),
  _httpConnection(httpConnection),
  _httpResponse(httpResponse),
  _totalLength(_httpResponse->getEntityLength()),
  _receivedBytes(0)
{}

HttpNullDownloadCommand::~HttpNullDownloadCommand() {}

void HttpNullDownloadCommand::setTransferDecoder
(const SharedHandle<TransferEncoding>& transferDecoder)
{
  _transferDecoder = transferDecoder;
}

bool HttpNullDownloadCommand::executeInternal()
{
  if(_totalLength == 0 && _transferDecoder.isNull()) {
    return processResponse();
  }
  const size_t BUFSIZE = 16*1024;
  unsigned char buf[BUFSIZE];
  size_t bufSize = BUFSIZE;

  try {
    socket->readData(buf, bufSize);

    if(_transferDecoder.isNull()) {
      _receivedBytes += bufSize;
    } else {
      // _receivedBytes is not updated if transferEncoding is set.
      size_t infbufSize = 16*1024;
      unsigned char infbuf[infbufSize];
      _transferDecoder->inflate(infbuf, infbufSize, buf, bufSize);
    }
    if(_totalLength != 0 && bufSize == 0) {
      throw DlRetryEx(EX_GOT_EOF);
    }
  } catch(RecoverableException& e) {
    logger->debug(EX_EXCEPTION_CAUGHT, e);
    return processResponse();
  }

  if(bufSize == 0) {
    // Since this method is called by DownloadEngine only when the socket is
    // readable, bufSize == 0 means server shutdown the connection.
    // So socket cannot be reused in this case.
    return prepareForRetry(0);
  } else if((!_transferDecoder.isNull() && _transferDecoder->finished())
	    || (_transferDecoder.isNull() && _totalLength == _receivedBytes)) {
    if(!_transferDecoder.isNull()) _transferDecoder->end();

    if(req->supportsPersistentConnection()) {
      std::pair<std::string, uint16_t> peerInfo;
      socket->getPeerInfo(peerInfo);
      e->poolSocket(peerInfo.first, peerInfo.second, socket);
    }
    return processResponse();
  } else {
    e->commands.push_back(this);
    return false;
  }
}

bool HttpNullDownloadCommand::processResponse()
{
  if(_httpResponse->isRedirect()) {
    _httpResponse->processRedirect();
    logger->info(MSG_REDIRECT, cuid, _httpResponse->getRedirectURI().c_str());
    return prepareForRetry(0);
  } else if(_httpResponse->hasRetryAfter()) {
    return prepareForRetry(_httpResponse->getRetryAfter());
  } else if(_httpResponse->getResponseStatus() >= "400") {
    if(_httpResponse->getResponseStatus() == "401") {
      throw DlAbortEx(EX_AUTH_FAILED);
    }else if(_httpResponse->getResponseStatus() == "404") {
      throw DlAbortEx(MSG_RESOURCE_NOT_FOUND);
    } else {
      throw DlAbortEx(StringFormat(EX_BAD_STATUS, Util::parseUInt(_httpResponse->getResponseStatus())).str());
    }
  } else {
    return prepareForRetry(0);
  }
}

} // namespace aria2
