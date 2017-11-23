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
#include <memory>

#include "SocketBuffer.h"

namespace aria2 {

class SocketCore;
class HttpHeader;
class HttpHeaderProcessor;
class DownloadEngine;
class SocketRecvBuffer;
class DiskWriter;

namespace util {
namespace security {
class HMAC;
class HMACResult;
} // namespace security
} // namespace util

enum RequestType { RPC_TYPE_NONE, RPC_TYPE_XML, RPC_TYPE_JSON, RPC_TYPE_JSONP };

// HTTP server class handling RPC request from the client.  It is not
// intended to be a generic HTTP server.
class HttpServer {
private:
  static std::unique_ptr<util::security::HMAC> hmac_;

  std::shared_ptr<SocketCore> socket_;
  std::shared_ptr<SocketRecvBuffer> socketRecvBuffer_;
  SocketBuffer socketBuffer_;
  std::unique_ptr<HttpHeaderProcessor> headerProcessor_;
  std::unique_ptr<HttpHeader> lastRequestHeader_;
  int64_t lastContentLength_;
  // How many bytes are consumed. The total number of bytes is
  // lastContentLength_.
  int64_t bodyConsumed_;
  RequestType reqType_;
  std::unique_ptr<DiskWriter> lastBody_;
  bool keepAlive_;
  bool gzip_;
  std::unique_ptr<util::security::HMACResult> username_;
  std::unique_ptr<util::security::HMACResult> password_;
  bool acceptsGZip_;
  std::string allowOrigin_;
  bool secure_;

public:
  HttpServer(const std::shared_ptr<SocketCore>& socket);

  ~HttpServer();

  bool receiveRequest();

  bool receiveBody();

  const std::string& getMethod() const;

  const std::string& getRequestPath() const;

  int setupResponseRecv();

  std::string createPath() const;

  std::string createQuery() const;

  DiskWriter* getBody() const;

  RequestType getRequestType() const { return reqType_; }

  void feedResponse(std::string text, const std::string& contentType);

  // Feeds HTTP response with the status code |status| (e.g.,
  // 200). The |headers| is zero or more lines of HTTP header field
  // and each line must end with "\r\n". The |text| is the response
  // body. The |contentType" is the content-type of the response body.
  void feedResponse(int status, const std::string& headers = "",
                    std::string text = "", const std::string& contentType = "");

  // Feeds "101 Switching Protocols" response. The |protocol| will
  // appear in Upgrade header field. The |headers| is zero or more
  // lines of HTTP header field and each line must end with "\r\n".
  void feedUpgradeResponse(const std::string& protocol,
                           const std::string& headers);

  bool authenticate();

  void setUsernamePassword(const std::string& username,
                           const std::string& password);

  ssize_t sendResponse();

  bool sendBufferIsEmpty() const;

  bool supportsPersistentConnection() const;

  bool supportsGZip() const { return gzip_ && acceptsGZip_; }

  void enableKeepAlive() { keepAlive_ = true; }

  void disableKeepAlive() { keepAlive_ = false; }

  void enableGZip() { gzip_ = true; }

  void disableGZip() { gzip_ = false; }

  int64_t getContentLength() const { return lastContentLength_; }

  const std::shared_ptr<SocketRecvBuffer>& getSocketRecvBuffer() const
  {
    return socketRecvBuffer_;
  }

  const std::string& getAllowOrigin() const { return allowOrigin_; }

  void setAllowOrigin(const std::string& allowOrigin)
  {
    allowOrigin_ = allowOrigin;
  }

  const std::shared_ptr<SocketCore>& getSocket() const { return socket_; }

  const std::unique_ptr<HttpHeader>& getRequestHeader() const
  {
    return lastRequestHeader_;
  }

  void setSecure(bool f) { secure_ = f; }

  bool getSecure() const { return secure_; }

  bool wantRead() const;
  bool wantWrite() const;
};

} // namespace aria2

#endif // D_HTTP_SERVER_H
