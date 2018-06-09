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
#include "HttpServer.h"

#include <sstream>

#include "HttpHeader.h"
#include "SocketCore.h"
#include "HttpHeaderProcessor.h"
#include "DlAbortEx.h"
#include "message.h"
#include "util.h"
#include "util_security.h"
#include "LogFactory.h"
#include "Logger.h"
#include "base64.h"
#include "a2functional.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "TimeA2.h"
#include "array_fun.h"
#include "JsonDiskWriter.h"
#ifdef ENABLE_XML_RPC
#  include "XmlRpcDiskWriter.h"
#endif // ENABLE_XML_RPC

namespace aria2 {

std::unique_ptr<util::security::HMAC> HttpServer::hmac_;

HttpServer::HttpServer(const std::shared_ptr<SocketCore>& socket)
    : socket_(socket),
      socketRecvBuffer_(std::make_shared<SocketRecvBuffer>(socket_)),
      socketBuffer_(socket),
      headerProcessor_(
          make_unique<HttpHeaderProcessor>(HttpHeaderProcessor::SERVER_PARSER)),
      lastContentLength_(0),
      bodyConsumed_(0),
      reqType_(RPC_TYPE_NONE),
      keepAlive_(true),
      gzip_(false),
      acceptsGZip_(false),
      secure_(false)
{
}

HttpServer::~HttpServer() = default;

namespace {
const char* getStatusString(int status)
{
  switch (status) {
  case 100:
    return "100 Continue";
  case 101:
    return "101 Switching Protocols";
  case 200:
    return "200 OK";
  case 201:
    return "201 Created";
  case 202:
    return "202 Accepted";
  case 203:
    return "203 Non-Authoritative Information";
  case 204:
    return "204 No Content";
  case 205:
    return "205 Reset Content";
  case 206:
    return "206 Partial Content";
  case 300:
    return "300 Multiple Choices";
  case 301:
    return "301 Moved Permanently";
  case 302:
    return "302 Found";
  case 303:
    return "303 See Other";
  case 304:
    return "304 Not Modified";
  case 305:
    return "305 Use Proxy";
  // case 306: return "306 (Unused)";
  case 307:
    return "307 Temporary Redirect";
  case 400:
    return "400 Bad Request";
  case 401:
    return "401 Unauthorized";
  case 402:
    return "402 Payment Required";
  case 403:
    return "403 Forbidden";
  case 404:
    return "404 Not Found";
  case 405:
    return "405 Method Not Allowed";
  case 406:
    return "406 Not Acceptable";
  case 407:
    return "407 Proxy Authentication Required";
  case 408:
    return "408 Request Timeout";
  case 409:
    return "409 Conflict";
  case 410:
    return "410 Gone";
  case 411:
    return "411 Length Required";
  case 412:
    return "412 Precondition Failed";
  case 413:
    return "413 Request Entity Too Large";
  case 414:
    return "414 Request-URI Too Long";
  case 415:
    return "415 Unsupported Media Type";
  case 416:
    return "416 Requested Range Not Satisfiable";
  case 417:
    return "417 Expectation Failed";
  // RFC 2817 defines 426 status code.
  case 426:
    return "426 Upgrade Required";
  case 500:
    return "500 Internal Server Error";
  case 501:
    return "501 Not Implemented";
  case 502:
    return "502 Bad Gateway";
  case 503:
    return "503 Service Unavailable";
  case 504:
    return "504 Gateway Timeout";
  case 505:
    return "505 HTTP Version Not Supported";
  default:
    return "";
  }
}
} // namespace

bool HttpServer::receiveRequest()
{
  if (socketRecvBuffer_->bufferEmpty()) {
    if (socketRecvBuffer_->recv() == 0 && !socket_->wantRead() &&
        !socket_->wantWrite()) {
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
  }
  if (headerProcessor_->parse(socketRecvBuffer_->getBuffer(),
                              socketRecvBuffer_->getBufferLength())) {
    lastRequestHeader_ = headerProcessor_->getResult();
    A2_LOG_INFO(fmt("HTTP Server received request\n%s",
                    headerProcessor_->getHeaderString().c_str()));
    socketRecvBuffer_->drain(headerProcessor_->getLastBytesProcessed());
    bodyConsumed_ = 0;
    if (setupResponseRecv() < 0) {
      A2_LOG_INFO("Request path is invalid. Ignore the request body.");
    }
    const std::string& contentLengthHdr =
        lastRequestHeader_->find(HttpHeader::CONTENT_LENGTH);
    if (!contentLengthHdr.empty()) {
      if (!util::parseLLIntNoThrow(lastContentLength_, contentLengthHdr) ||
          lastContentLength_ < 0) {
        throw DL_ABORT_EX(
            fmt("Invalid Content-Length=%s", contentLengthHdr.c_str()));
      }
    }
    else {
      lastContentLength_ = 0;
    }
    headerProcessor_->clear();

    std::vector<Scip> acceptEncodings;
    const std::string& acceptEnc =
        lastRequestHeader_->find(HttpHeader::ACCEPT_ENCODING);
    util::splitIter(acceptEnc.begin(), acceptEnc.end(),
                    std::back_inserter(acceptEncodings), ',', true);
    acceptsGZip_ = false;
    for (std::vector<Scip>::const_iterator i = acceptEncodings.begin(),
                                           eoi = acceptEncodings.end();
         i != eoi; ++i) {
      if (util::strieq((*i).first, (*i).second, "gzip")) {
        acceptsGZip_ = true;
        break;
      }
    }
    return true;
  }
  else {
    socketRecvBuffer_->drain(headerProcessor_->getLastBytesProcessed());
    return false;
  }
}

bool HttpServer::receiveBody()
{
  if (lastContentLength_ == bodyConsumed_) {
    return true;
  }
  if (socketRecvBuffer_->bufferEmpty()) {
    if (socketRecvBuffer_->recv() == 0 && !socket_->wantRead() &&
        !socket_->wantWrite()) {
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
  }
  size_t length =
      std::min(socketRecvBuffer_->getBufferLength(),
               static_cast<size_t>(lastContentLength_ - bodyConsumed_));
  if (lastBody_) {
    lastBody_->writeData(socketRecvBuffer_->getBuffer(), length, 0);
  }
  socketRecvBuffer_->drain(length);
  bodyConsumed_ += length;
  return lastContentLength_ == bodyConsumed_;
}

const std::string& HttpServer::getMethod() const
{
  return lastRequestHeader_->getMethod();
}

const std::string& HttpServer::getRequestPath() const
{
  return lastRequestHeader_->getRequestPath();
}

void HttpServer::feedResponse(std::string text, const std::string& contentType)
{
  feedResponse(200, "", std::move(text), contentType);
}

void HttpServer::feedResponse(int status, const std::string& headers,
                              std::string text, const std::string& contentType)
{
  std::string httpDate = Time().toHTTPDate();
  std::string header =
      fmt("HTTP/1.1 %s\r\n"
          "Date: %s\r\n"
          "Content-Length: %lu\r\n"
          "Expires: %s\r\n"
          "Cache-Control: no-cache\r\n",
          getStatusString(status), httpDate.c_str(),
          static_cast<unsigned long>(text.size()), httpDate.c_str());
  if (!contentType.empty()) {
    header += "Content-Type: ";
    header += contentType;
    header += "\r\n";
  }
  if (!allowOrigin_.empty()) {
    header += "Access-Control-Allow-Origin: ";
    header += allowOrigin_;
    header += "\r\n";
  }
  if (supportsGZip()) {
    header += "Content-Encoding: gzip\r\n";
  }
  if (!supportsPersistentConnection()) {
    header += "Connection: close\r\n";
  }
  header += headers;
  header += "\r\n";
  A2_LOG_DEBUG(fmt("HTTP Server sends response:\n%s", header.c_str()));
  socketBuffer_.pushStr(std::move(header));
  socketBuffer_.pushStr(std::move(text));
}

void HttpServer::feedUpgradeResponse(const std::string& protocol,
                                     const std::string& headers)
{
  std::string header = fmt("HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: %s\r\n"
                           "Connection: Upgrade\r\n"
                           "%s"
                           "\r\n",
                           protocol.c_str(), headers.c_str());
  A2_LOG_DEBUG(fmt("HTTP Server sends upgrade response:\n%s", header.c_str()));
  socketBuffer_.pushStr(std::move(header));
}

ssize_t HttpServer::sendResponse() { return socketBuffer_.send(); }

bool HttpServer::sendBufferIsEmpty() const
{
  return socketBuffer_.sendBufferIsEmpty();
}

bool HttpServer::authenticate()
{
  if (!username_) {
    return true;
  }

  const std::string& authHeader =
      lastRequestHeader_->find(HttpHeader::AUTHORIZATION);
  if (authHeader.empty()) {
    return false;
  }
  auto p = util::divide(std::begin(authHeader), std::end(authHeader), ' ');
  if (!util::streq(p.first.first, p.first.second, "Basic")) {
    return false;
  }

  std::string userpass = base64::decode(p.second.first, p.second.second);
  auto up = util::divide(std::begin(userpass), std::end(userpass), ':', false);
  std::string username(up.first.first, up.first.second);
  std::string password(up.second.first, up.second.second);
  return *username_ == hmac_->getResult(username) &&
         (!password_ || *password_ == hmac_->getResult(password));
}

void HttpServer::setUsernamePassword(const std::string& username,
                                     const std::string& password)
{
  using namespace util::security;

  if (!hmac_) {
    hmac_ = HMAC::createRandom();
  }

  if (!username.empty()) {
    username_ = make_unique<HMACResult>(hmac_->getResult(username));
  }
  else {
    username_.reset();
  }

  if (!password.empty()) {
    password_ = make_unique<HMACResult>(hmac_->getResult(password));
  }
  else {
    password_.reset();
  }
}

int HttpServer::setupResponseRecv()
{
  std::string path = createPath();
  if (getMethod() == "GET") {
    if (path == "/jsonrpc") {
      reqType_ = RPC_TYPE_JSONP;
      lastBody_.reset();
      return 0;
    }
  }
  else if (getMethod() == "POST") {
    if (path == "/jsonrpc") {
      if (reqType_ != RPC_TYPE_JSON) {
        reqType_ = RPC_TYPE_JSON;
        lastBody_ = make_unique<json::JsonDiskWriter>();
      }
      return 0;
    }
#ifdef ENABLE_XML_RPC
    if (path == "/rpc") {
      if (reqType_ != RPC_TYPE_XML) {
        reqType_ = RPC_TYPE_XML;
        lastBody_ = make_unique<rpc::XmlRpcDiskWriter>();
      }
      return 0;
    }
#endif // ENABLE_XML_RPC
  }
  reqType_ = RPC_TYPE_NONE;
  lastBody_.reset();
  return -1;
}

std::string HttpServer::createPath() const
{
  std::string reqPath = getRequestPath();
  size_t i;
  size_t len = reqPath.size();
  for (i = 0; i < len; ++i) {
    if (reqPath[i] == '#' || reqPath[i] == '?') {
      break;
    }
  }
  reqPath = reqPath.substr(0, i);
  if (reqPath.empty()) {
    reqPath = "/";
  }
  return reqPath;
}

std::string HttpServer::createQuery() const
{
  std::string reqPath = getRequestPath();
  size_t i;
  size_t len = reqPath.size();
  for (i = 0; i < len; ++i) {
    if (reqPath[i] == '#' || reqPath[i] == '?') {
      break;
    }
  }
  if (i == len || reqPath[i] == '#') {
    return "";
  }
  else {
    size_t start = i;
    for (; i < len; ++i) {
      if (reqPath[i] == '#') {
        break;
      }
    }
    return reqPath.substr(start, i - start);
  }
}

DiskWriter* HttpServer::getBody() const { return lastBody_.get(); }

bool HttpServer::supportsPersistentConnection() const
{
  return keepAlive_ && lastRequestHeader_ && lastRequestHeader_->isKeepAlive();
}

bool HttpServer::wantRead() const { return socket_->wantRead(); }

bool HttpServer::wantWrite() const { return socket_->wantWrite(); }

} // namespace aria2
