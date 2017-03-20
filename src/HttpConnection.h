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
#ifndef D_HTTP_CONNECTION_H
#define D_HTTP_CONNECTION_H

#include "common.h"

#include <string>
#include <deque>
#include <memory>

#include "SocketBuffer.h"
#include "Command.h"

namespace aria2 {

class HttpRequest;
class HttpResponse;
class HttpHeaderProcessor;
class Option;
class Segment;
class SocketCore;
class SocketRecvBuffer;

class HttpRequestEntry {
private:
  std::unique_ptr<HttpRequest> httpRequest_;
  std::unique_ptr<HttpHeaderProcessor> proc_;

public:
  HttpRequestEntry(std::unique_ptr<HttpRequest> httpRequest);

  // Resets proc_ by recreating the object.  Thus any object obtained
  // by getHttpRequest() before this call is invalidated.
  void resetHttpHeaderProcessor();

  const std::unique_ptr<HttpRequest>& getHttpRequest() const
  {
    return httpRequest_;
  }

  std::unique_ptr<HttpRequest> popHttpRequest();

  const std::unique_ptr<HttpHeaderProcessor>& getHttpHeaderProcessor() const;
};

typedef std::deque<std::unique_ptr<HttpRequestEntry>> HttpRequestEntries;

class HttpConnection {
private:
  cuid_t cuid_;
  std::shared_ptr<SocketCore> socket_;
  std::shared_ptr<SocketRecvBuffer> socketRecvBuffer_;
  SocketBuffer socketBuffer_;

  HttpRequestEntries outstandingHttpRequests_;

  std::string eraseConfidentialInfo(const std::string& request);
  void sendRequest(std::unique_ptr<HttpRequest> httpRequest,
                   std::string request);

public:
  HttpConnection(cuid_t cuid, const std::shared_ptr<SocketCore>& socket,
                 const std::shared_ptr<SocketRecvBuffer>& socketRecvBuffer);
  ~HttpConnection();

  /**
   * Sends Http request.
   * If segment.sp+segment.ds > 0 then Range header is added.
   * This method is used in HTTP/HTTP downloading and FTP downloading via
   * HTTP proxy(GET method).
   * @param segment indicates starting position of the file for downloading
   */
  void sendRequest(std::unique_ptr<HttpRequest> httpRequest);

  /**
   * Sends Http proxy request using CONNECT method.
   */
  void sendProxyRequest(std::unique_ptr<HttpRequest> httpRequest);

  /**
   * Receives HTTP response from the server and returns HttpResponseHandle
   * object which contains response header and HttpRequestHandle object
   * for this response.
   * If a response is not fully received, received header is buffured
   * in this object and returns 0.
   * You should continue to call this method until whole response header is
   * received and this method returns non-null HttpResponseHandle object.
   *
   * @return HttpResponse or 0 if whole response header is not received
   */
  std::unique_ptr<HttpResponse> receiveResponse();

  bool isIssued(const std::shared_ptr<Segment>& segment) const;

  bool sendBufferIsEmpty() const;

  void sendPendingData();

  const std::shared_ptr<SocketRecvBuffer>& getSocketRecvBuffer() const
  {
    return socketRecvBuffer_;
  }
};

} // namespace aria2

#endif // D_HTTP_CONNECTION_H
