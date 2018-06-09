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
#include "FtpConnection.h"

#include <array>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "Request.h"
#include "Segment.h"
#include "Option.h"
#include "util.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"
#include "Logger.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "SocketCore.h"
#include "A2STR.h"
#include "fmt.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include "util.h"
#include "error_code.h"

namespace aria2 {

FtpConnection::FtpConnection(cuid_t cuid,
                             const std::shared_ptr<SocketCore>& socket,
                             const std::shared_ptr<Request>& req,
                             const std::shared_ptr<AuthConfig>& authConfig,
                             const Option* op)
    : cuid_(cuid),
      socket_(socket),
      req_(req),
      authConfig_(authConfig),
      option_(op),
      socketBuffer_(socket),
      baseWorkingDir_("/")
{
}

FtpConnection::~FtpConnection() = default;

bool FtpConnection::sendUser()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "USER ";
    request += authConfig_->getUser();
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, "USER ********"));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendPass()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "PASS ";
    request += authConfig_->getPassword();
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, "PASS ********"));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendType()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "TYPE ";
    request += (option_->get(PREF_FTP_TYPE) == V_ASCII ? 'A' : 'I');
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendPwd()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "PWD\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendCwd(const std::string& dir)
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "CWD ";
    request += util::percentDecode(dir.begin(), dir.end());
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendMdtm()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "MDTM ";
    request +=
        util::percentDecode(req_->getFile().begin(), req_->getFile().end());
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendSize()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "SIZE ";
    request +=
        util::percentDecode(req_->getFile().begin(), req_->getFile().end());
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendEpsv()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request("EPSV\r\n");
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendPasv()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request("PASV\r\n");
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

std::shared_ptr<SocketCore> FtpConnection::createServerSocket()
{
  auto endpoint = socket_->getAddrInfo();
  auto serverSocket = std::make_shared<SocketCore>();
  serverSocket->bind(endpoint.addr.c_str(), 0, AF_UNSPEC);
  serverSocket->beginListen();
  return serverSocket;
}

bool FtpConnection::sendEprt(const std::shared_ptr<SocketCore>& serverSocket)
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    auto endpoint = serverSocket->getAddrInfo();
    auto request =
        fmt("EPRT |%d|%s|%u|\r\n", endpoint.family == AF_INET ? 1 : 2,
            endpoint.addr.c_str(), endpoint.port);
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendPort(const std::shared_ptr<SocketCore>& serverSocket)
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    auto endpoint = socket_->getAddrInfo();
    int ipaddr[4];
    sscanf(endpoint.addr.c_str(), "%d.%d.%d.%d", &ipaddr[0], &ipaddr[1],
           &ipaddr[2], &ipaddr[3]);
    auto svEndpoint = serverSocket->getAddrInfo();
    auto request =
        fmt("PORT %d,%d,%d,%d,%d,%d\r\n", ipaddr[0], ipaddr[1], ipaddr[2],
            ipaddr[3], svEndpoint.port / 256, svEndpoint.port % 256);
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendRest(const std::shared_ptr<Segment>& segment)
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request =
        fmt("REST %" PRId64 "\r\n", segment ? segment->getPositionToWrite()
                                            : static_cast<int64_t>(0LL));
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool FtpConnection::sendRetr()
{
  if (socketBuffer_.sendBufferIsEmpty()) {
    std::string request = "RETR ";
    request +=
        util::percentDecode(req_->getFile().begin(), req_->getFile().end());
    request += "\r\n";
    A2_LOG_INFO(fmt(MSG_SENDING_REQUEST, cuid_, request.c_str()));
    socketBuffer_.pushStr(std::move(request));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

int FtpConnection::getStatus(const std::string& response) const
{
  int status;
  // When the response is not like "%d %*s",
  // we return 0.
  if (response.find_first_not_of("0123456789") != 3 ||
      !(response.find(" ") == 3 || response.find("-") == 3)) {
    return 0;
  }
  if (sscanf(response.c_str(), "%d %*s", &status) == 1) {
    return status;
  }
  else {
    return 0;
  }
}

// Returns the length of the response if the whole response has been received.
// The length includes \r\n.
// If the whole response has not been received, then returns std::string::npos.
std::string::size_type
FtpConnection::findEndOfResponse(int status, const std::string& buf) const
{
  if (buf.size() <= 4) {
    return std::string::npos;
  }
  // if 4th character of buf is '-', then multi line response is expected.
  if (buf.at(3) == '-') {
    // multi line response
    std::string::size_type p;
    p = buf.find(fmt("\r\n%d ", status));
    if (p == std::string::npos) {
      return std::string::npos;
    }
    p = buf.find("\r\n", p + 6);
    if (p == std::string::npos) {
      return std::string::npos;
    }
    else {
      return p + 2;
    }
  }
  else {
    // single line response
    std::string::size_type p = buf.find("\r\n");
    if (p == std::string::npos) {
      return std::string::npos;
    }
    else {
      return p + 2;
    }
  }
}

bool FtpConnection::bulkReceiveResponse(std::pair<int, std::string>& response)
{
  std::array<char, 1_k> buf;
  while (1) {
    size_t size = buf.size();
    socket_->readData(buf.data(), size);
    if (size == 0) {
      if (socket_->wantRead() || socket_->wantWrite()) {
        break;
      }
      else {
        throw DL_RETRY_EX(EX_GOT_EOF);
      }
    }
    if (strbuf_.size() + size > MAX_RECV_BUFFER) {
      throw DL_RETRY_EX(fmt("Max FTP recv buffer reached. length=%lu",
                            static_cast<unsigned long>(strbuf_.size() + size)));
    }
    strbuf_.append(std::begin(buf), std::begin(buf) + size);
  }
  int status;
  if (strbuf_.size() >= 4) {
    status = getStatus(strbuf_);
    if (status == 0) {
      throw DL_ABORT_EX2(EX_INVALID_RESPONSE, error_code::FTP_PROTOCOL_ERROR);
    }
  }
  else {
    return false;
  }
  std::string::size_type length;
  if ((length = findEndOfResponse(status, strbuf_)) != std::string::npos) {
    response.first = status;
    response.second.assign(strbuf_.begin(), strbuf_.begin() + length);
    A2_LOG_INFO(fmt(MSG_RECEIVE_RESPONSE, cuid_, response.second.c_str()));
    strbuf_.erase(0, length);
    return true;
  }
  else {
    // didn't receive response fully.
    return false;
  }
}

int FtpConnection::receiveResponse()
{
  std::pair<int, std::string> response;
  if (bulkReceiveResponse(response)) {
    return response.first;
  }
  else {
    return 0;
  }
}

#ifdef __MINGW32__
#  define LONGLONG_PRINTF "%I64d"
#  define ULONGLONG_PRINTF "%I64u"
#  define LONGLONG_SCANF "%I64d"
#  define ULONGLONG_SCANF "%I64u"
#else
#  define LONGLONG_PRINTF "%" PRId64 ""
#  define ULONGLONG_PRINTF "%llu"
#  define LONGLONG_SCANF "%Ld"
// Mac OSX uses "%llu" for 64bits integer.
#  define ULONGLONG_SCANF "%Lu"
#endif // __MINGW32__

int FtpConnection::receiveSizeResponse(int64_t& size)
{
  std::pair<int, std::string> response;
  if (bulkReceiveResponse(response)) {
    if (response.first == 213) {
      auto rp = util::divide(std::begin(response.second),
                             std::end(response.second), ' ');
      if (!util::parseLLIntNoThrow(
              size, std::string(rp.second.first, rp.second.second)) ||
          size < 0) {
        throw DL_ABORT_EX("Size must be positive integer");
      }
    }
    return response.first;
  }
  else {
    return 0;
  }
}

namespace {
template <typename T>
bool getInteger(T* dest, const char* first, const char* last)
{
  int res = 0;
  // We assume *dest won't overflow.
  for (; first != last; ++first) {
    if (util::isDigit(*first)) {
      res *= 10;
      res += (*first) - '0';
    }
    else {
      return false;
    }
  }
  *dest = res;
  return true;
}
} // namespace

int FtpConnection::receiveMdtmResponse(Time& time)
{
  // MDTM command, specified in RFC3659.
  std::pair<int, std::string> response;
  if (bulkReceiveResponse(response)) {
    if (response.first == 213) {
      char buf[15]; // YYYYMMDDhhmmss+\0, milli second part is dropped.
      sscanf(response.second.c_str(), "%*u %14s", buf);
      if (strlen(buf) == 14) {
        // We don't use Time::parse(buf,"%Y%m%d%H%M%S") here because Mac OS X
        // and included strptime doesn't parse data for this format.
        struct tm tm;
        memset(&tm, 0, sizeof(tm));
        if (getInteger(&tm.tm_sec, &buf[12], &buf[14]) &&
            getInteger(&tm.tm_min, &buf[10], &buf[12]) &&
            getInteger(&tm.tm_hour, &buf[8], &buf[10]) &&
            getInteger(&tm.tm_mday, &buf[6], &buf[8]) &&
            getInteger(&tm.tm_mon, &buf[4], &buf[6]) &&
            getInteger(&tm.tm_year, &buf[0], &buf[4])) {
          tm.tm_mon -= 1;
          tm.tm_year -= 1900;
          time = Time(timegm(&tm));
        }
        else {
          time = Time::null();
        }
      }
      else {
        time = Time::null();
      }
    }
    return response.first;
  }
  else {
    return 0;
  }
}

int FtpConnection::receiveEpsvResponse(uint16_t& port)
{
  std::pair<int, std::string> response;
  if (bulkReceiveResponse(response)) {
    if (response.first == 229) {
      port = 0;
      std::string::size_type leftParen = response.second.find("(");
      std::string::size_type rightParen = response.second.find(")");
      if (leftParen == std::string::npos || rightParen == std::string::npos ||
          leftParen > rightParen) {
        return response.first;
      }
      std::vector<Scip> rd;
      util::splitIter(response.second.begin() + leftParen + 1,
                      response.second.begin() + rightParen,
                      std::back_inserter(rd), '|', true, true);
      uint32_t portTemp = 0;
      if (rd.size() == 5 &&
          util::parseUIntNoThrow(portTemp,
                                 std::string(rd[3].first, rd[3].second))) {
        if (0 < portTemp && portTemp <= UINT16_MAX) {
          port = portTemp;
        }
      }
    }
    return response.first;
  }
  else {
    return 0;
  }
}

int FtpConnection::receivePasvResponse(std::pair<std::string, uint16_t>& dest)
{
  std::pair<int, std::string> response;
  if (bulkReceiveResponse(response)) {
    if (response.first == 227) {
      // we assume the format of response is "227 Entering Passive
      // Mode (h1,h2,h3,h4,p1,p2)."
      int h1, h2, h3, h4, p1, p2;
      std::string::size_type p = response.second.find("(");
      if (p >= 4) {
        sscanf(response.second.c_str() + p, "(%d,%d,%d,%d,%d,%d).", &h1, &h2,
               &h3, &h4, &p1, &p2);
        // ip address
        dest.first = fmt("%d.%d.%d.%d", h1, h2, h3, h4);
        // port number
        dest.second = 256 * p1 + p2;
      }
      else {
        throw DL_RETRY_EX(EX_INVALID_RESPONSE);
      }
    }
    return response.first;
  }
  else {
    return 0;
  }
}

int FtpConnection::receivePwdResponse(std::string& pwd)
{
  std::pair<int, std::string> response;
  if (bulkReceiveResponse(response)) {
    if (response.first == 257) {
      std::string::size_type first;
      std::string::size_type last;

      if ((first = response.second.find("\"")) != std::string::npos &&
          (last = response.second.find("\"", ++first)) != std::string::npos) {
        pwd.assign(response.second.begin() + first,
                   response.second.begin() + last);
      }
      else {
        throw DL_ABORT_EX2(EX_INVALID_RESPONSE, error_code::FTP_PROTOCOL_ERROR);
      }
    }
    return response.first;
  }
  else {
    return 0;
  }
}

void FtpConnection::setBaseWorkingDir(const std::string& baseWorkingDir)
{
  baseWorkingDir_ = baseWorkingDir;
}

const std::string& FtpConnection::getUser() const
{
  return authConfig_->getUser();
}

} // namespace aria2
