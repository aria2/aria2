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
#include "HttpServerCommand.h"
#include "SocketCore.h"
#include "DownloadEngine.h"
#include "HttpServer.h"
#include "HttpHeader.h"
#include "Logger.h"
#include "LogFactory.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "HttpServerBodyCommand.h"
#include "HttpServerResponseCommand.h"
#include "RecoverableException.h"
#include "prefs.h"
#include "Option.h"
#include "util.h"
#include "wallclock.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "base64.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"
#ifdef ENABLE_WEBSOCKET
#  include "WebSocketResponseCommand.h"
#endif // ENABLE_WEBSOCKET

namespace aria2 {

HttpServerCommand::HttpServerCommand(cuid_t cuid, DownloadEngine* e,
                                     const std::shared_ptr<SocketCore>& socket,
                                     bool secure)
    : Command(cuid),
      e_(e),
      socket_(socket),
      httpServer_(std::make_shared<HttpServer>(socket)),
      writeCheck_(false)
{
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  e_->addSocketForReadCheck(socket_, this);
  httpServer_->setSecure(secure);
  httpServer_->setUsernamePassword(e_->getOption()->get(PREF_RPC_USER),
                                   e_->getOption()->get(PREF_RPC_PASSWD));
  if (e_->getOption()->getAsBool(PREF_RPC_ALLOW_ORIGIN_ALL)) {
    httpServer_->setAllowOrigin("*");
  }
#ifdef HAVE_ZLIB
  httpServer_->enableGZip();
#else  // !HAVE_ZLIB
  httpServer_->disableGZip();
#endif // !HAVE_ZLIB
  checkSocketRecvBuffer();
}

HttpServerCommand::HttpServerCommand(
    cuid_t cuid, const std::shared_ptr<HttpServer>& httpServer,
    DownloadEngine* e, const std::shared_ptr<SocketCore>& socket)
    : Command(cuid),
      e_(e),
      socket_(socket),
      httpServer_(httpServer),
      writeCheck_(false)
{
  e_->addSocketForReadCheck(socket_, this);
  checkSocketRecvBuffer();
}

HttpServerCommand::~HttpServerCommand()
{
  e_->deleteSocketForReadCheck(socket_, this);
  if (writeCheck_) {
    e_->deleteSocketForWriteCheck(socket_, this);
  }
}

void HttpServerCommand::checkSocketRecvBuffer()
{
  if (httpServer_->getSocketRecvBuffer()->bufferEmpty() &&
      socket_->getRecvBufferedLength() == 0) {
    return;
  }

  setStatus(Command::STATUS_ONESHOT_REALTIME);
  e_->setNoWait(true);
}

#ifdef ENABLE_WEBSOCKET

namespace {
// Creates server's WebSocket accept key which will be sent in
// Sec-WebSocket-Accept header field. The |clientKey| is the value
// found in Sec-WebSocket-Key header field in the request.
std::string createWebSocketServerKey(const std::string& clientKey)
{
  std::string src = clientKey;
  src += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  unsigned char digest[20];
  message_digest::digest(digest, sizeof(digest), MessageDigest::sha1().get(),
                         src.c_str(), src.size());
  return base64::encode(&digest[0], &digest[sizeof(digest)]);
}
} // namespace

namespace {
int websocketHandshake(const HttpHeader* header)
{
  if (header->getMethod() != "GET" ||
      header->find(HttpHeader::SEC_WEBSOCKET_KEY).empty()) {
    return 400;
  }
  else if (header->find(HttpHeader::SEC_WEBSOCKET_VERSION) != "13") {
    return 426;
  }
  else if (header->getRequestPath() != "/jsonrpc") {
    return 404;
  }
  else {
    return 101;
  }
}
} // namespace

#endif // ENABLE_WEBSOCKET

void HttpServerCommand::updateWriteCheck()
{
  if (httpServer_->wantWrite()) {
    if (!writeCheck_) {
      writeCheck_ = true;
      e_->addSocketForWriteCheck(socket_, this);
    }
  }
  else if (writeCheck_) {
    writeCheck_ = false;
    e_->deleteSocketForWriteCheck(socket_, this);
  }
}

bool HttpServerCommand::execute()
{
  if (e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  try {
    if (socket_->isReadable(0) || (writeCheck_ && socket_->isWritable(0)) ||
        socket_->getRecvBufferedLength() ||
        !httpServer_->getSocketRecvBuffer()->bufferEmpty()) {
      timeoutTimer_ = global::wallclock();

#ifdef ENABLE_SSL
      if (httpServer_->getSecure()) {
        // tlsAccept() just returns true if handshake has already
        // finished.
        if (!socket_->tlsAccept()) {
          updateWriteCheck();
          e_->addCommand(std::unique_ptr<Command>(this));
          return false;
        }
      }
#endif // ENABLE_SSL

      if (!httpServer_->receiveRequest()) {
        updateWriteCheck();
        e_->addCommand(std::unique_ptr<Command>(this));
        return false;
      }
      // CORS preflight request uses OPTIONS method. It is not
      // restricted by authentication.
      if (!httpServer_->authenticate() &&
          httpServer_->getMethod() != "OPTIONS") {
        httpServer_->disableKeepAlive();
        httpServer_->feedResponse(
            401, "WWW-Authenticate: Basic realm=\"aria2\"\r\n");
        e_->addCommand(make_unique<HttpServerResponseCommand>(
            getCuid(), httpServer_, e_, socket_));
        e_->setNoWait(true);
        return true;
      }
      auto& header = httpServer_->getRequestHeader();
      if (header->fieldContains(HttpHeader::UPGRADE, "websocket") &&
          header->fieldContains(HttpHeader::CONNECTION, "upgrade")) {
#ifdef ENABLE_WEBSOCKET
        int status = websocketHandshake(header.get());
        if (status == 101) {
          std::string serverKey = createWebSocketServerKey(
              header->find(HttpHeader::SEC_WEBSOCKET_KEY));
          httpServer_->feedUpgradeResponse(
              "websocket",
              fmt("Sec-WebSocket-Accept: %s\r\n", serverKey.c_str()));
          e_->addCommand(make_unique<rpc::WebSocketResponseCommand>(
              getCuid(), httpServer_, e_, socket_));
        }
        else {
          if (status == 426) {
            httpServer_->feedResponse(426, "Sec-WebSocket-Version: 13\r\n");
          }
          else {
            httpServer_->feedResponse(status);
          }
          e_->addCommand(make_unique<HttpServerResponseCommand>(
              getCuid(), httpServer_, e_, socket_));
        }
        e_->setNoWait(true);
        return true;
#else  // !ENABLE_WEBSOCKET
        httpServer_->feedResponse(400);
        e_->addCommand(make_unique<HttpServerResponseCommand>(
            getCuid(), httpServer_, e_, socket_));
        e_->setNoWait(true);
        return true;
#endif // !ENABLE_WEBSOCKET
      }
      else {
        if (e_->getOption()->getAsInt(PREF_RPC_MAX_REQUEST_SIZE) <
            httpServer_->getContentLength()) {
          A2_LOG_INFO(fmt("Request too long. ContentLength=%" PRId64 "."
                          " See --rpc-max-request-size option to loose"
                          " this limitation.",
                          httpServer_->getContentLength()));
          return true;
        }
        e_->addCommand(make_unique<HttpServerBodyCommand>(
            getCuid(), httpServer_, e_, socket_));
        e_->setNoWait(true);
        return true;
      }
    }
    else {
      if (timeoutTimer_.difference(global::wallclock()) >= 30_s) {
        A2_LOG_INFO("HTTP request timeout.");
        return true;
      }
      else {
        e_->addCommand(std::unique_ptr<Command>(this));
        return false;
      }
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(fmt("CUID#%" PRId64
                       " - Error occurred while reading HTTP request",
                       getCuid()),
                   e);
    return true;
  }
}

} // namespace aria2
