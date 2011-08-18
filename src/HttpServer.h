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
#ifndef D_HTTP_SERVER_H
#define D_HTTP_SERVER_H

#include "common.h"

#include <string>
#include <sstream>
#include <vector>

#include "SharedHandle.h"
#include "SocketBuffer.h"

namespace aria2 {

class SocketCore;
class HttpHeader;
class HttpHeaderProcessor;
class DownloadEngine;
class SocketRecvBuffer;

class HttpServer {
private:
  SharedHandle<SocketCore> socket_;
  SharedHandle<SocketRecvBuffer> socketRecvBuffer_;
  SocketBuffer socketBuffer_;
  DownloadEngine* e_;
  SharedHandle<HttpHeaderProcessor> headerProcessor_;
  SharedHandle<HttpHeader> lastRequestHeader_;
  uint64_t lastContentLength_;
  std::stringstream lastBody_;
  bool keepAlive_;
  bool gzip_;
  std::string username_;
  std::string password_;
  bool acceptsPersistentConnection_;
  bool acceptsGZip_;
  std::string allowOrigin_;
public:
  HttpServer(const SharedHandle<SocketCore>& socket, DownloadEngine* e);

  ~HttpServer();

  SharedHandle<HttpHeader> receiveRequest();

  bool receiveBody();

  std::string getBody() const;

  const std::string& getMethod() const;

  const std::string& getRequestPath() const;

  void feedResponse(const std::string& text, const std::string& contentType);

  void feedResponse(const std::string& status,
                    const std::string& headers,
                    const std::string& text,
                    const std::string& contentType);

  bool authenticate();

  void setUsernamePassword
  (const std::string& username, const std::string& password);

  ssize_t sendResponse();

  bool sendBufferIsEmpty() const;

  bool supportsPersistentConnection() const
  {
    return keepAlive_ && acceptsPersistentConnection_;
  }

  bool supportsGZip() const
  {
    return gzip_ && acceptsGZip_;
  }

  void enableKeepAlive() { keepAlive_ = true; }

  void disableKeepAlive() { keepAlive_ = false; }

  void enableGZip() { gzip_ = true; }

  void disableGZip() { gzip_ = false; }

  uint64_t getContentLength() const { return lastContentLength_; }

  const SharedHandle<SocketRecvBuffer>& getSocketRecvBuffer() const
  {
    return socketRecvBuffer_;
  }

  void setAllowOrigin(const std::string& allowOrigin)
  {
    allowOrigin_ = allowOrigin;
  }
};

} // namespace aria2

#endif // D_HTTP_SERVER_H
