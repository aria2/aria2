/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "HttpServer.h"

#include <sstream>

#include "HttpHeader.h"
#include "SocketCore.h"
#include "HttpHeaderProcessor.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Base64.h"
#include "a2functional.h"

namespace aria2 {

HttpServer::HttpServer(const SharedHandle<SocketCore>& socket,
		       DownloadEngine* e):
  _socket(socket),
  _socketBuffer(socket),
  _e(e),
  _headerProcessor(new HttpHeaderProcessor()),
  _logger(LogFactory::getInstance()),
  _keepAlive(true)
{}

HttpServer::~HttpServer() {}

SharedHandle<HttpHeader> HttpServer::receiveRequest()
{
  size_t size = 512;
  unsigned char buf[size];
  _socket->peekData(buf, size);
  if(size == 0 && !(_socket->wantRead() || _socket->wantWrite())) {
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  _headerProcessor->update(buf, size);
  if(!_headerProcessor->eoh()) {
    _socket->readData(buf, size);
    return SharedHandle<HttpHeader>();
  }
  size_t putbackDataLength = _headerProcessor->getPutBackDataLength();
  size -= putbackDataLength;
  _socket->readData(buf, size);

  SharedHandle<HttpHeader> header = _headerProcessor->getHttpRequestHeader();
  if(!header.isNull()) {
    _logger->info("HTTP Server received request\n%s",
		  _headerProcessor->getHeaderString().c_str());
    _lastRequestHeader = header;
    _lastBody.clear();
    _lastBody.str("");
    _lastContentLength =
      _lastRequestHeader->getFirstAsUInt(HttpHeader::CONTENT_LENGTH);
    _headerProcessor->clear();
  }

  return header;
}

bool HttpServer::receiveBody()
{
  if(_lastContentLength == 0) {
    return true;
  }
  const size_t BUFLEN = 4096;
  char buf[BUFLEN];
  size_t length = std::min(BUFLEN,
			   static_cast<size_t>
			   (_lastContentLength-_lastBody.tellg()));
  _socket->readData(buf, length);
  if(length == 0 && !(_socket->wantRead() || _socket->wantWrite())) {
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  _lastBody.write(buf, length);
  return _lastContentLength == static_cast<uint64_t>(_lastBody.tellp());
}

std::string HttpServer::getBody() const
{
  return _lastBody.str();
}

const std::string& HttpServer::getRequestPath() const
{
  return _lastRequestHeader->getRequestPath();
}

bool HttpServer::supportsPersistentConnection() const
{
  if(!_keepAlive) {
    return false;
  }

  std::string connection =
    util::toLower(_lastRequestHeader->getFirst(HttpHeader::CONNECTION));

  return connection.find(HttpHeader::CLOSE) == std::string::npos &&
    (_lastRequestHeader->getVersion() == HttpHeader::HTTP_1_1 ||
     connection.find("keep-alive") != std::string::npos);
}

void HttpServer::feedResponse(const std::string& text, const std::string& contentType)
{
  feedResponse("200 OK", "", text, contentType);
}

void HttpServer::feedResponse(const std::string& status,
			      const std::string& headers,
			      const std::string& text,
			      const std::string& contentType)
{
  std::string header = "HTTP/1.1 ";
  strappend(header, status, "\r\n",
	    "Content-Type: ", contentType, "\r\n",
	    "Content-Length: ", util::uitos(text.size()), "\r\n");

  if(!supportsPersistentConnection()) {
    header += "Connection: close\r\n";
  }
  if(!headers.empty()) {
    header += headers;
    if(!util::endsWith(headers, "\r\n")) {
      header += "\r\n";
    }
  }

  header += "\r\n";

  _logger->debug("HTTP Server sends response:\n%s", header.c_str());
		 
  _socketBuffer.feedSendBuffer(header);
  _socketBuffer.feedSendBuffer(text);
}

ssize_t HttpServer::sendResponse()
{
  return _socketBuffer.send();
}

bool HttpServer::sendBufferIsEmpty() const
{
  return _socketBuffer.sendBufferIsEmpty();
}

bool HttpServer::authenticate()
{
  if(_username.empty()) {
    return true;
  }

  std::string authHeader = _lastRequestHeader->getFirst("Authorization");
  if(authHeader.empty()) {
    return false;
  }
  std::pair<std::string, std::string> p = util::split(authHeader, " ");
  if(p.first != "Basic") {
    return false;
  }
  std::string userpass = Base64::decode(p.second);
  std::pair<std::string, std::string> userpassPair = util::split(userpass, ":");
  return _username == userpassPair.first && _password == userpassPair.second;
}

} // namespace aria2
