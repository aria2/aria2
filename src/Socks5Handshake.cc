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
#include "Socks5Handshake.h"

#include <algorithm>

#include "SocketCore.h"
#include "a2netcompat.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "util.h"

namespace aria2 {

namespace {
const uint8_t SOCKS5_VERSION = 0x05;
const uint8_t SOCKS5_AUTH_NONE = 0x00;
const uint8_t SOCKS5_AUTH_USERPASS = 0x02;
const uint8_t SOCKS5_CMD_CONNECT = 0x01;
const uint8_t SOCKS5_ATYP_IPV4 = 0x01;
const uint8_t SOCKS5_ATYP_DOMAINNAME = 0x03;
const uint8_t SOCKS5_ATYP_IPV6 = 0x04;
const uint8_t SOCKS5_USERPASS_VERSION = 0x01;
const uint8_t SOCKS5_REPLY_SUCCEEDED = 0x00;
} // namespace

Socks5Handshake::Socks5Handshake(const std::string& user,
                                 const std::string& passwd,
                                 const std::string& targetHost,
                                 const std::string& targetAddr,
                                 uint16_t targetPort)
    : user_(user),
      passwd_(passwd),
      targetHost_(targetHost),
      targetAddr_(targetAddr),
      targetPort_(targetPort),
      state_(GREETING_SEND),
      sendOffset_(0),
      recvNeeded_(0)
{
  if (targetAddr_.empty() && util::isNumericHost(targetHost_)) {
    targetAddr_ = targetHost_;
  }
  if (user_.size() > 255 || passwd_.size() > 255) {
    throw DL_ABORT_EX("SOCKS5 username or password is too long.");
  }
  if (targetAddr_.empty() && targetHost_.size() > 255) {
    throw DL_ABORT_EX("Hostname is too long for SOCKS5 CONNECT.");
  }

  sendBuf_.push_back(SOCKS5_VERSION);
  if (user_.empty()) {
    sendBuf_.push_back(1);
    sendBuf_.push_back(SOCKS5_AUTH_NONE);
  }
  else {
    sendBuf_.push_back(2);
    sendBuf_.push_back(SOCKS5_AUTH_NONE);
    sendBuf_.push_back(SOCKS5_AUTH_USERPASS);
  }
}

bool Socks5Handshake::flush(SocketCore& socket)
{
  while (sendOffset_ < sendBuf_.size()) {
    ssize_t written = socket.writeData(sendBuf_.data() + sendOffset_,
                                       sendBuf_.size() - sendOffset_);
    if (written == 0) {
      return false;
    }
    sendOffset_ += written;
  }
  return true;
}

bool Socks5Handshake::fill(SocketCore& socket)
{
  while (recvBuf_.size() < recvNeeded_) {
    unsigned char buf[256];
    size_t len = std::min(sizeof(buf), recvNeeded_ - recvBuf_.size());
    socket.readData(buf, len);
    if (len == 0) {
      if (!socket.wantRead() && !socket.wantWrite()) {
        throw DL_RETRY_EX("Got EOF from the SOCKS5 proxy.");
      }
      return false;
    }
    recvBuf_.insert(recvBuf_.end(), buf, buf + len);
  }
  return true;
}

void Socks5Handshake::startRecv(State state, size_t n)
{
  state_ = state;
  recvBuf_.clear();
  recvNeeded_ = n;
}

void Socks5Handshake::startSend(State state)
{
  state_ = state;
  sendOffset_ = 0;
}

void Socks5Handshake::buildConnect()
{
  sendBuf_.clear();
  sendBuf_.push_back(SOCKS5_VERSION);
  sendBuf_.push_back(SOCKS5_CMD_CONNECT);
  sendBuf_.push_back(0x00); // RSV
  unsigned char addr[sizeof(in6_addr)];
  if (targetAddr_.empty()) {
    sendBuf_.push_back(SOCKS5_ATYP_DOMAINNAME);
    sendBuf_.push_back(static_cast<uint8_t>(targetHost_.size()));
    sendBuf_.insert(sendBuf_.end(), targetHost_.begin(), targetHost_.end());
  }
  else if (inetPton(AF_INET, targetAddr_.c_str(), addr) == 0) {
    sendBuf_.push_back(SOCKS5_ATYP_IPV4);
    sendBuf_.insert(sendBuf_.end(), addr, addr + sizeof(in_addr));
  }
  else if (inetPton(AF_INET6, targetAddr_.c_str(), addr) == 0) {
    sendBuf_.push_back(SOCKS5_ATYP_IPV6);
    sendBuf_.insert(sendBuf_.end(), addr, addr + sizeof(in6_addr));
  }
  else {
    throw DL_ABORT_EX(
        fmt("Invalid address for SOCKS5 CONNECT: %s", targetAddr_.c_str()));
  }
  sendBuf_.push_back(static_cast<uint8_t>((targetPort_ >> 8) & 0xFF));
  sendBuf_.push_back(static_cast<uint8_t>(targetPort_ & 0xFF));
  startSend(CONNECT_SEND);
}

void Socks5Handshake::processGreetingResponse()
{
  if (recvBuf_[0] != SOCKS5_VERSION) {
    throw DL_ABORT_EX("SOCKS5 proxy returned unexpected version.");
  }
  uint8_t method = recvBuf_[1];
  if (method == SOCKS5_AUTH_USERPASS && !user_.empty()) {
    sendBuf_.clear();
    sendBuf_.push_back(SOCKS5_USERPASS_VERSION);
    sendBuf_.push_back(static_cast<uint8_t>(user_.size()));
    sendBuf_.insert(sendBuf_.end(), user_.begin(), user_.end());
    sendBuf_.push_back(static_cast<uint8_t>(passwd_.size()));
    sendBuf_.insert(sendBuf_.end(), passwd_.begin(), passwd_.end());
    startSend(AUTH_SEND);
  }
  else if (method == SOCKS5_AUTH_NONE) {
    buildConnect();
  }
  else {
    throw DL_ABORT_EX("SOCKS5 proxy requires unsupported authentication.");
  }
}

void Socks5Handshake::processAuthResponse()
{
  if (recvBuf_[0] != SOCKS5_USERPASS_VERSION || recvBuf_[1] != 0x00) {
    throw DL_RETRY_EX("SOCKS5 proxy authentication failed.");
  }
  buildConnect();
}

void Socks5Handshake::processConnectHead()
{
  if (recvBuf_[0] != SOCKS5_VERSION) {
    throw DL_ABORT_EX("SOCKS5 proxy returned unexpected version in reply.");
  }
  if (recvBuf_[1] != SOCKS5_REPLY_SUCCEEDED) {
    throw DL_RETRY_EX(
        fmt("SOCKS5 proxy connect failed with status 0x%02x.", recvBuf_[1]));
  }
  // BND.ADDR and BND.PORT follow.
  switch (recvBuf_[3]) {
  case SOCKS5_ATYP_IPV4:
    startRecv(CONNECT_RECV_TAIL, 4 + 2);
    break;
  case SOCKS5_ATYP_IPV6:
    startRecv(CONNECT_RECV_TAIL, 16 + 2);
    break;
  case SOCKS5_ATYP_DOMAINNAME:
    startRecv(CONNECT_RECV_DOMAIN_LEN, 1);
    break;
  default:
    throw DL_ABORT_EX("SOCKS5 proxy returned unknown address type.");
  }
}

Socks5Handshake::Status Socks5Handshake::advance(SocketCore& socket)
{
  for (;;) {
    switch (state_) {
    case GREETING_SEND:
      if (!flush(socket)) {
        return WANT_WRITE;
      }
      startRecv(GREETING_RECV, 2);
      break;

    case GREETING_RECV:
      if (!fill(socket)) {
        return WANT_READ;
      }
      processGreetingResponse();
      break;

    case AUTH_SEND:
      if (!flush(socket)) {
        return WANT_WRITE;
      }
      startRecv(AUTH_RECV, 2);
      break;

    case AUTH_RECV:
      if (!fill(socket)) {
        return WANT_READ;
      }
      processAuthResponse();
      break;

    case CONNECT_SEND:
      if (!flush(socket)) {
        return WANT_WRITE;
      }
      startRecv(CONNECT_RECV_HEAD, 4);
      break;

    case CONNECT_RECV_HEAD:
      if (!fill(socket)) {
        return WANT_READ;
      }
      processConnectHead();
      break;

    case CONNECT_RECV_DOMAIN_LEN:
      if (!fill(socket)) {
        return WANT_READ;
      }
      // BND.ADDR (domain name) and BND.PORT follow.
      startRecv(CONNECT_RECV_TAIL, recvBuf_[0] + 2);
      break;

    case CONNECT_RECV_TAIL:
      if (!fill(socket)) {
        return WANT_READ;
      }
      state_ = FINISHED;
      break;

    case FINISHED:
      return DONE;
    }
  }
}

} // namespace aria2
