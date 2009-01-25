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
#include "HttpHeader.h"
#include "SocketCore.h"
#include "HttpHeaderProcessor.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Util.h"

namespace aria2 {

HttpServer::HttpServer(const SharedHandle<SocketCore>& socket,
		       DownloadEngine* e):
  _socket(socket),
  _socketBuffer(socket),
  _e(e),
  _headerProcessor(new HttpHeaderProcessor())
{}

HttpServer::~HttpServer() {}

SharedHandle<HttpHeader> HttpServer::receiveRequest()
{
  size_t size = 512;
  unsigned char buf[size];
  _socket->peekData(buf, size);
  if(size == 0 && !(_socket->wantRead() || _socket->wantWrite())) {
    throw DlAbortEx(EX_EOF_FROM_PEER);
  }
  _headerProcessor->update(buf, size);
  if(!_headerProcessor->eoh()) {
    _socket->readData(buf, size);
    return SharedHandle<HttpHeader>();
  }
  size_t putbackDataLength = _headerProcessor->getPutBackDataLength();
  size -= putbackDataLength;
  _socket->readData(buf, size);

  return _headerProcessor->getHttpRequestHeader();
}

void HttpServer::feedResponse(const std::string& text)
{
  std::string header = "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: "+Util::uitos(text.size())+"\r\n"
    "Connection: close\r\n"
    "\r\n";
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

} // namespace aria2
