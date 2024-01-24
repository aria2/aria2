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
#include "SocksProxySocket.h"

#include <netinet/in.h>
#include <sstream>

#include "SocketCore.h"
#include "a2functional.h"

namespace {
const char C_SOCKS_VER = '\x05';
const char C_AUTH_NONE = '\xff';
const char C_AUTH_USERPASS_VER = '\x01';
const char C_AUTH_USERPASS_OK = '\x00';
const char C_OK = '\x00';
} // namespace

namespace aria2 {

enum SocksProxyAddrType {
  SOCKS_ADDR_INET = 1,
  SOCKS_ADDR_DOMAIN = 3,
  SOCKS_ADDR_INET6 = 4,
};

SocksProxySocket::SocksProxySocket(int family) : family_(family) {}

SocksProxySocket::~SocksProxySocket() = default;

void SocksProxySocket::establish(const std::string& host, uint16_t port)
{
  socket_ = make_unique<SocketCore>();
  socket_->establishConnection(host, port);
  socket_->setBlockingMode();
}

void SocksProxySocket::establish(std::unique_ptr<SocketCore> socket)
{
  socket_ = std::move(socket);
}

int SocksProxySocket::negotiateAuth(std::vector<SocksProxyAuthMethod> expected)
{
  std::stringstream req;
  req << C_SOCKS_VER;
  req << static_cast<char>(expected.size());
  for (auto c : expected) {
    req << static_cast<char>(c);
  }
  socket_->writeData(req.str());

  char res[2];
  size_t resLen = sizeof(res);
  socket_->readData(res, resLen);
  int authMethod = res[1];
  if (authMethod == C_AUTH_NONE) {
    socket_->closeConnection();
    return -1;
  }
  return authMethod;
}

int SocksProxySocket::authByUserpass(const std::string& user,
                                     const std::string& passwd)
{
  std::stringstream req;
  req << C_AUTH_USERPASS_VER;
  req << static_cast<char>(user.length()) << user;
  req << static_cast<char>(passwd.length()) << passwd;
  socket_->writeData(req.str());

  char res[2];
  size_t resLen = sizeof(res);
  socket_->readData(res, resLen);
  if (res[1] != C_AUTH_USERPASS_OK) {
    socket_->closeConnection();
  }
  return res[1];
}

bool SocksProxySocket::authByUserpassOrNone(const std::string& user,
                                            const std::string& passwd)
{
  bool noAuth = user.empty() || passwd.empty();
  if (noAuth) {
    int authMethod =
        negotiateAuth(std::vector<SocksProxyAuthMethod>{SOCKS_AUTH_NO_AUTH});
    if (authMethod < 0) {
      return false;
    }
  }
  else {
    int authMethod = negotiateAuth(std::vector<SocksProxyAuthMethod>{
        SOCKS_AUTH_NO_AUTH, SOCKS_AUTH_USERPASS});
    if (authMethod < 0) {
      return false;
    }
    if (authMethod == SOCKS_AUTH_USERPASS) {
      int status = authByUserpass(user, passwd);
      if (status != 0) {
        return false;
      }
    }
  }
  return true;
}

void SocksProxySocket::sendCmd(SocksProxyCmd cmd, const std::string& dstAddr,
                               uint16_t dstPort, bool allowEmpty)
{
  std::stringstream req;
  req << C_SOCKS_VER << static_cast<char>(cmd) << '\x00';
  if (family_ == AF_INET) {
    if (!allowEmpty || !dstAddr.empty()) {
      char addrBuf[10];
      net::getBinAddr(addrBuf, dstAddr);
      req << static_cast<char>(SOCKS_ADDR_INET) << std::string(addrBuf, 4);
    }
    else {
      req << std::string(4, '\x00');
    }
  }
  else {
    if (!allowEmpty || !dstAddr.empty()) {
      char addrBuf[20];
      net::getBinAddr(addrBuf, dstAddr);
      req << static_cast<char>(SOCKS_ADDR_INET6) << std::string(addrBuf, 16);
    }
    else {
      req << std::string(16, '\x00');
    }
  }
  if (dstPort) {
    uint16_t listenPortBuf = htons(dstPort);
    req << std::string(reinterpret_cast<const char*>(&listenPortBuf), 2);
  }
  else {
    req << std::string(2, '\x00');
  }
  socket_->writeData(req.str());
}

int SocksProxySocket::receiveReply(int& bndFamily, std::string& bndAddr,
                                   uint16_t& bndPort)
{
  char res[5];
  size_t resLen = sizeof(res);
  socket_->readData(res, resLen);
  int rep = res[1];
  if (rep != C_OK) {
    socket_->closeConnection();
    return rep;
  }

  if (res[3] == SOCKS_ADDR_INET) {
    char addrBuf[6];
    addrBuf[0] = res[4];
    size_t addrLen = sizeof(addrBuf) - 1;
    socket_->readData(addrBuf + 1, addrLen);
    char addrStrBuf[20];
    inetNtop(AF_INET, addrBuf, addrStrBuf, 20);
    bndFamily = AF_INET;
    bndAddr = std::string(addrStrBuf);
    bndPort = ntohs(*reinterpret_cast<uint16_t*>(addrBuf + 4));
  }
  else if (res[3] == SOCKS_ADDR_INET6) {
    char addrBuf[18];
    addrBuf[0] = res[4];
    size_t addrLen = sizeof(addrBuf) - 1;
    socket_->readData(addrBuf + 1, addrLen);
    char addrStrBuf[50];
    inetNtop(AF_INET6, addrBuf, addrStrBuf, 50);
    bndFamily = AF_INET6;
    bndAddr = std::string(addrStrBuf);
    bndPort = ntohs(*reinterpret_cast<uint16_t*>(addrBuf + 16));
  }
  else if (res[3] == SOCKS_ADDR_DOMAIN) {
    // 2 more bytes to hold port temporarily.
    size_t resLen = res[4] + 2;
    bndAddr = std::string(resLen, '\x00');
    socket_->readData(&bndAddr[0], resLen);
    bndFamily = AF_INET + AF_INET6;
    bndPort = ntohs(*reinterpret_cast<uint16_t*>(&bndAddr[0] + res[4]));
    bndAddr.resize(res[4]);
  }
  else {
    socket_->closeConnection();
    return -1;
  }
  return rep;
}

int SocksProxySocket::startUdpAssociate(const std::string& listenAddr,
                                        uint16_t listenPort,
                                        std::string& bndAddr, uint16_t& bndPort)
{
  sendCmd(SOCKS_CMD_UDP_ASSOCIATE, listenAddr, listenPort, true);

  int bFamily;
  std::string bAddr;
  uint16_t bPort;
  int rep = receiveReply(bFamily, bAddr, bPort);
  if (rep != C_OK) {
    return rep;
  }

  bndAddr = bAddr;
  bndPort = bPort;
  return rep;
}

} // namespace aria2
