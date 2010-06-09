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
#include "HttpSkipResponseCommand.h"
#include "HttpConnection.h"
#include "HttpResponse.h"
#include "message.h"
#include "SocketCore.h"
#include "Decoder.h"
#include "DlRetryEx.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "Logger.h"
#include "HttpRequest.h"
#include "Segment.h"
#include "util.h"
#include "StringFormat.h"
#include "DlAbortEx.h"
#include "HttpHeader.h"
#include "prefs.h"
#include "Option.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "DownloadContext.h"
#include "RequestGroupMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "ServerStatMan.h"

namespace aria2 {

HttpSkipResponseCommand::HttpSkipResponseCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 const SharedHandle<HttpConnection>& httpConnection,
 const SharedHandle<HttpResponse>& httpResponse,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& s):
  AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
  _httpConnection(httpConnection),
  _httpResponse(httpResponse),
  _totalLength(_httpResponse->getEntityLength()),
  _receivedBytes(0)
{}

HttpSkipResponseCommand::~HttpSkipResponseCommand() {}

void HttpSkipResponseCommand::setTransferEncodingDecoder
(const SharedHandle<Decoder>& decoder)
{
  _transferEncodingDecoder = decoder;
}

bool HttpSkipResponseCommand::executeInternal()
{
  if(getRequest()->getMethod() == Request::METHOD_HEAD ||
     (_totalLength == 0 && _transferEncodingDecoder.isNull())) {
    // If request method is HEAD or content-length header is present and
    // it's value is 0, then pool socket for reuse.
    // If content-length header is not present, then EOF is expected in the end.
    // In this case, the content is thrown away and socket cannot be pooled. 
    if(getRequest()->getMethod() == Request::METHOD_HEAD ||
       _httpResponse->getHttpHeader()->defined(HttpHeader::CONTENT_LENGTH)) {
      poolConnection();
    }
    return processResponse();
  }
  const size_t BUFSIZE = 16*1024;
  unsigned char buf[BUFSIZE];
  size_t bufSize = BUFSIZE;

  try {
    getSocket()->readData(buf, bufSize);

    if(_transferEncodingDecoder.isNull()) {
      _receivedBytes += bufSize;
    } else {
      // _receivedBytes is not updated if transferEncoding is set.
      // The return value is safely ignored here.
      _transferEncodingDecoder->decode(buf, bufSize);
    }
    if(_totalLength != 0 && bufSize == 0 &&
       !getSocket()->wantRead() && !getSocket()->wantWrite()) {
      throw DL_RETRY_EX(EX_GOT_EOF);
    }
  } catch(RecoverableException& e) {
    if(getLogger()->debug()) {
      getLogger()->debug(EX_EXCEPTION_CAUGHT, e);
    }
    return processResponse();
  }

  bool finished = false;
  if(_transferEncodingDecoder.isNull()) {
    if(bufSize == 0) {
      if(!getSocket()->wantRead() && !getSocket()->wantWrite()) {
        return processResponse();
      }
    } else {
      finished = (_totalLength == _receivedBytes);
    }
  } else {
    finished = _transferEncodingDecoder->finished();
  }
  if(finished) {
    poolConnection();
    return processResponse();
  } else {
    setWriteCheckSocketIf(getSocket(), getSocket()->wantWrite());
    getDownloadEngine()->addCommand(this);
    return false;
  }
}

void HttpSkipResponseCommand::poolConnection() const
{
  if(getRequest()->supportsPersistentConnection()) {
    getDownloadEngine()->poolSocket
      (getRequest(), createProxyRequest(), getSocket());
  }
}

bool HttpSkipResponseCommand::processResponse()
{
  if(_httpResponse->isRedirect()) {
    unsigned int rnum =
      _httpResponse->getHttpRequest()->getRequest()->getRedirectCount();
    if(rnum >= Request::MAX_REDIRECT) {
      throw DL_ABORT_EX
        (StringFormat("Too many redirects: count=%u", rnum).str());
    }
    _httpResponse->processRedirect();
    return prepareForRetry(0);
  } else if(_httpResponse->getResponseStatus() >= HttpHeader::S400) {
    if(_httpResponse->getResponseStatus() == HttpHeader::S401) {
      if(getOption()->getAsBool(PREF_HTTP_AUTH_CHALLENGE) &&
         !_httpResponse->getHttpRequest()->authenticationUsed() &&
         getDownloadEngine()->getAuthConfigFactory()->activateBasicCred
         (getRequest()->getHost(), getRequest()->getDir(), getOption().get())) {
        return prepareForRetry(0);
      } else {
        throw DL_ABORT_EX(EX_AUTH_FAILED);
      }
    }else if(_httpResponse->getResponseStatus() == HttpHeader::S404) {
      throw DL_ABORT_EX2(MSG_RESOURCE_NOT_FOUND,
                         downloadresultcode::RESOURCE_NOT_FOUND);
    } else {
      throw DL_ABORT_EX
        (StringFormat
         (EX_BAD_STATUS,
          util::parseUInt(_httpResponse->getResponseStatus())).str());
    }
  } else {
    return prepareForRetry(0);
  }
}

void HttpSkipResponseCommand::disableSocketCheck()
{
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

} // namespace aria2
