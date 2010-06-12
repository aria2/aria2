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
#include "HttpConnection.h"

#include <sstream>

#include "util.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "Request.h"
#include "Segment.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpHeaderProcessor.h"
#include "HttpHeader.h"
#include "Logger.h"
#include "Socket.h"
#include "Option.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"

namespace aria2 {

HttpRequestEntry::HttpRequestEntry
(const SharedHandle<HttpRequest>& httpRequest):
  _httpRequest(httpRequest),
  _proc(new HttpHeaderProcessor()) {}

HttpRequestEntry::~HttpRequestEntry() {}

HttpConnection::HttpConnection(cuid_t cuid, const SocketHandle& socket):
  _cuid(cuid), _socket(socket),
  _socketBuffer(socket),
  _logger(LogFactory::getInstance())
{}

std::string HttpConnection::eraseConfidentialInfo(const std::string& request)
{
  std::istringstream istr(request);
  std::string result;
  std::string line;
  while(getline(istr, line)) {
    static const std::string AUTH_HEADER("Authorization: Basic");
    static const std::string PROXY_AUTH_HEADER("Proxy-Authorization: Basic");
    if(util::startsWith(line, AUTH_HEADER)) {
      result += "Authorization: Basic ********\n";
    } else if(util::startsWith(line, PROXY_AUTH_HEADER)) {
      result += "Proxy-Authorization: Basic ********\n";
    } else {
      strappend(result, line, "\n");
    }
  }
  return result;
}

void HttpConnection::sendRequest(const SharedHandle<HttpRequest>& httpRequest)
{
  std::string request = httpRequest->createRequest();
  if(_logger->info()) {
    _logger->info(MSG_SENDING_REQUEST,
                  util::itos(_cuid).c_str(),
                  eraseConfidentialInfo(request).c_str());
  }
  _socketBuffer.pushStr(request);
  _socketBuffer.send();
  SharedHandle<HttpRequestEntry> entry(new HttpRequestEntry(httpRequest));
  _outstandingHttpRequests.push_back(entry);
}

void HttpConnection::sendProxyRequest
(const SharedHandle<HttpRequest>& httpRequest)
{
  std::string request = httpRequest->createProxyRequest();
  if(_logger->info()) {
    _logger->info(MSG_SENDING_REQUEST,
                  util::itos(_cuid).c_str(),
                  eraseConfidentialInfo(request).c_str());
  }
  _socketBuffer.pushStr(request);
  _socketBuffer.send();
  SharedHandle<HttpRequestEntry> entry(new HttpRequestEntry(httpRequest));
  _outstandingHttpRequests.push_back(entry);
}

SharedHandle<HttpResponse> HttpConnection::receiveResponse()
{
  if(_outstandingHttpRequests.empty()) {
    throw DL_ABORT_EX(EX_NO_HTTP_REQUEST_ENTRY_FOUND);
  }
  HttpRequestEntryHandle entry = _outstandingHttpRequests.front();
  HttpHeaderProcessorHandle proc = entry->getHttpHeaderProcessor();

  unsigned char buf[512];
  size_t size = sizeof(buf);
  _socket->peekData(buf, size);
  if(size == 0) {
    if(_socket->wantRead() || _socket->wantWrite()) {
      return SharedHandle<HttpResponse>();
    } else {
      throw DL_RETRY_EX(EX_INVALID_RESPONSE);
    }
  }
  proc->update(buf, size);
  if(!proc->eoh()) {
    _socket->readData(buf, size);
    return SharedHandle<HttpResponse>();
  }
  size_t putbackDataLength = proc->getPutBackDataLength();
  size -= putbackDataLength;
  _socket->readData(buf, size);
  if(_logger->info()) {
    _logger->info(MSG_RECEIVE_RESPONSE,
                  util::itos(_cuid).c_str(), proc->getHeaderString().c_str());
  }
  SharedHandle<HttpHeader> httpHeader = proc->getHttpResponseHeader();
  SharedHandle<HttpResponse> httpResponse(new HttpResponse());
  httpResponse->setCuid(_cuid);
  httpResponse->setHttpHeader(httpHeader);
  httpResponse->setHttpRequest(entry->getHttpRequest());

  _outstandingHttpRequests.pop_front();

  return httpResponse;
}

bool HttpConnection::isIssued(const SharedHandle<Segment>& segment) const
{
  for(HttpRequestEntries::const_iterator itr = _outstandingHttpRequests.begin(),
        eoi = _outstandingHttpRequests.end(); itr != eoi; ++itr) {
    SharedHandle<HttpRequest> httpRequest = (*itr)->getHttpRequest();
    if(httpRequest->getSegment() == segment) {
      return true;
    }
  }
  return false;
}

SharedHandle<HttpRequest> HttpConnection::getFirstHttpRequest() const
{
  if(_outstandingHttpRequests.empty()) {
    return SharedHandle<HttpRequest>();
  } else {
    return _outstandingHttpRequests.front()->getHttpRequest();
  }
}

bool HttpConnection::sendBufferIsEmpty() const
{
  return _socketBuffer.sendBufferIsEmpty();
}

void HttpConnection::sendPendingData()
{
  _socketBuffer.send();
}

} // namespace aria2
