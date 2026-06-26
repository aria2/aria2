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
#include "SocksProxyCommand.h"

#include <cassert>

#include "Request.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "Option.h"
#include "prefs.h"
#include "SocketCore.h"
#include "SocketRecvBuffer.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "message.h"
#include "fmt.h"
#include "LogFactory.h"
#include "Logger.h"

namespace aria2 {

namespace {
const uint8_t SOCKS5_VERSION = 0x05;
const uint8_t SOCKS5_AUTH_NONE = 0x00;
const uint8_t SOCKS5_AUTH_USERPASS = 0x02;
const uint8_t SOCKS5_AUTH_NO_ACCEPTABLE = 0xFF;
const uint8_t SOCKS5_CMD_CONNECT = 0x01;
const uint8_t SOCKS5_ATYP_DOMAINNAME = 0x03;
const uint8_t SOCKS5_USERPASS_VERSION = 0x01;
const uint8_t SOCKS5_REPLY_SUCCEEDED = 0x00;
} // namespace

SocksProxyCommand::SocksProxyCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e, const std::shared_ptr<Request>& proxyRequest,
    const std::shared_ptr<SocketCore>& s,
    NextCommandFactory nextCommandFactory)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
      proxyRequest_(proxyRequest),
      recvBuffer_(std::make_shared<SocketRecvBuffer>(s)),
      nextCommandFactory_(std::move(nextCommandFactory)),
      state_(SOCKS_GREETING_SEND),
      sendOffset_(0)
{
  setTimeout(std::chrono::seconds(getOption()->getAsInt(PREF_CONNECT_TIMEOUT)));
  disableReadCheckSocket();
  setWriteCheckSocket(getSocket());
  buildGreeting();
}

SocksProxyCommand::~SocksProxyCommand() = default;

bool SocksProxyCommand::sendData()
{
  while (sendOffset_ < sendBuf_.size()) {
    ssize_t written = getSocket()->writeData(
        sendBuf_.data() + sendOffset_, sendBuf_.size() - sendOffset_);
    if (written == 0) {
      setWriteCheckSocket(getSocket());
      return false;
    }
    sendOffset_ += written;
  }
  return true;
}

bool SocksProxyCommand::recvData(size_t expect)
{
  if (recvBuffer_->getBufferLength() < expect) {
    recvBuffer_->recv();
    if (recvBuffer_->getBufferLength() < expect) {
      setReadCheckSocket(getSocket());
      return false;
    }
  }
  return true;
}

void SocksProxyCommand::buildGreeting()
{
  const std::string& user = proxyRequest_->getUri();
  bool hasAuth = !getOption()->get(PREF_ALL_PROXY_USER).empty();

  sendBuf_.clear();
  sendOffset_ = 0;
  sendBuf_.push_back(SOCKS5_VERSION);
  if (hasAuth) {
    sendBuf_.push_back(2);
    sendBuf_.push_back(SOCKS5_AUTH_NONE);
    sendBuf_.push_back(SOCKS5_AUTH_USERPASS);
  }
  else {
    sendBuf_.push_back(1);
    sendBuf_.push_back(SOCKS5_AUTH_NONE);
  }
}

void SocksProxyCommand::buildAuth()
{
  std::string user = getOption()->get(PREF_ALL_PROXY_USER);
  std::string passwd = getOption()->get(PREF_ALL_PROXY_PASSWD);

  sendBuf_.clear();
  sendOffset_ = 0;
  sendBuf_.push_back(SOCKS5_USERPASS_VERSION);
  sendBuf_.push_back(static_cast<uint8_t>(user.size()));
  sendBuf_.insert(sendBuf_.end(), user.begin(), user.end());
  sendBuf_.push_back(static_cast<uint8_t>(passwd.size()));
  sendBuf_.insert(sendBuf_.end(), passwd.begin(), passwd.end());
}

void SocksProxyCommand::buildConnect()
{
  const std::string& host = getRequest()->getHost();
  uint16_t port = getRequest()->getPort();

  sendBuf_.clear();
  sendOffset_ = 0;
  sendBuf_.push_back(SOCKS5_VERSION);
  sendBuf_.push_back(SOCKS5_CMD_CONNECT);
  sendBuf_.push_back(0x00); // RSV
  sendBuf_.push_back(SOCKS5_ATYP_DOMAINNAME);
  sendBuf_.push_back(static_cast<uint8_t>(host.size()));
  sendBuf_.insert(sendBuf_.end(), host.begin(), host.end());
  sendBuf_.push_back(static_cast<uint8_t>((port >> 8) & 0xFF));
  sendBuf_.push_back(static_cast<uint8_t>(port & 0xFF));
}

void SocksProxyCommand::processGreetingResponse()
{
  const unsigned char* buf = recvBuffer_->getBuffer();
  if (buf[0] != SOCKS5_VERSION) {
    throw DL_ABORT_EX("SOCKS5 proxy returned unexpected version.");
  }
  uint8_t method = buf[1];
  recvBuffer_->drain(2);

  if (method == SOCKS5_AUTH_NONE) {
    buildConnect();
    state_ = SOCKS_CONNECT_SEND;
  }
  else if (method == SOCKS5_AUTH_USERPASS) {
    buildAuth();
    state_ = SOCKS_AUTH_SEND;
  }
  else {
    throw DL_ABORT_EX("SOCKS5 proxy requires unsupported authentication.");
  }
}

void SocksProxyCommand::processAuthResponse()
{
  const unsigned char* buf = recvBuffer_->getBuffer();
  if (buf[0] != SOCKS5_USERPASS_VERSION || buf[1] != 0x00) {
    throw DL_RETRY_EX("SOCKS5 proxy authentication failed.");
  }
  recvBuffer_->drain(2);
  buildConnect();
  state_ = SOCKS_CONNECT_SEND;
}

bool SocksProxyCommand::processConnectResponse()
{
  const unsigned char* buf = recvBuffer_->getBuffer();
  size_t len = recvBuffer_->getBufferLength();

  if (len < 5) {
    return false;
  }

  if (buf[0] != SOCKS5_VERSION) {
    throw DL_ABORT_EX("SOCKS5 proxy returned unexpected version in reply.");
  }
  if (buf[1] != SOCKS5_REPLY_SUCCEEDED) {
    throw DL_RETRY_EX(
        fmt("SOCKS5 proxy connect failed with status 0x%02x.", buf[1]));
  }

  size_t replyLen;
  uint8_t atyp = buf[3];
  if (atyp == 0x01) { // IPv4
    replyLen = 10;
  }
  else if (atyp == 0x04) { // IPv6
    replyLen = 22;
  }
  else if (atyp == 0x03) { // Domain name
    replyLen = 7 + buf[4];
  }
  else {
    throw DL_ABORT_EX("SOCKS5 proxy returned unknown address type.");
  }

  if (len < replyLen) {
    return false;
  }
  recvBuffer_->drain(replyLen);
  return true;
}

bool SocksProxyCommand::executeInternal()
{
  switch (state_) {
  case SOCKS_GREETING_SEND:
    if (!sendData()) {
      addCommandSelf();
      return false;
    }
    A2_LOG_INFO(fmt("CUID#%" PRId64
                    " - SOCKS5 greeting sent to proxy %s:%d",
                    getCuid(), proxyRequest_->getHost().c_str(),
                    proxyRequest_->getPort()));
    state_ = SOCKS_GREETING_RECV;
    setReadCheckSocket(getSocket());
    disableWriteCheckSocket();
    addCommandSelf();
    return false;

  case SOCKS_GREETING_RECV:
    if (!recvData(2)) {
      addCommandSelf();
      return false;
    }
    processGreetingResponse();
    disableReadCheckSocket();
    setWriteCheckSocket(getSocket());
    addCommandSelf();
    return false;

  case SOCKS_AUTH_SEND:
    if (!sendData()) {
      addCommandSelf();
      return false;
    }
    state_ = SOCKS_AUTH_RECV;
    setReadCheckSocket(getSocket());
    disableWriteCheckSocket();
    addCommandSelf();
    return false;

  case SOCKS_AUTH_RECV:
    if (!recvData(2)) {
      addCommandSelf();
      return false;
    }
    processAuthResponse();
    disableReadCheckSocket();
    setWriteCheckSocket(getSocket());
    addCommandSelf();
    return false;

  case SOCKS_CONNECT_SEND:
    if (!sendData()) {
      addCommandSelf();
      return false;
    }
    A2_LOG_INFO(fmt("CUID#%" PRId64
                    " - SOCKS5 CONNECT request sent for %s:%d",
                    getCuid(), getRequest()->getHost().c_str(),
                    getRequest()->getPort()));
    state_ = SOCKS_CONNECT_RECV;
    setReadCheckSocket(getSocket());
    disableWriteCheckSocket();
    addCommandSelf();
    return false;

  case SOCKS_CONNECT_RECV: {
    recvBuffer_->recv();
    if (!processConnectResponse()) {
      setReadCheckSocket(getSocket());
      addCommandSelf();
      return false;
    }
    A2_LOG_INFO(fmt("CUID#%" PRId64
                    " - SOCKS5 tunnel established to %s:%d",
                    getCuid(), getRequest()->getHost().c_str(),
                    getRequest()->getPort()));
    getDownloadEngine()->addCommand(nextCommandFactory_(this));
    return true;
  }
  }

  // Unreachable
  assert(0);
  return false;
}

} // namespace aria2
