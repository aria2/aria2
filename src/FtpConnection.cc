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
#include "FtpConnection.h"
#include "Request.h"
#include "Segment.h"
#include "Option.h"
#include "Util.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"
#include "Logger.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "Socket.h"

namespace aria2 {

FtpConnection::FtpConnection(int32_t cuid, const SocketHandle& socket,
			     const RequestHandle& req, const Option* op):
  cuid(cuid), socket(socket), req(req), option(op),
  logger(LogFactory::getInstance()) {}

FtpConnection::~FtpConnection() {}

void FtpConnection::sendUser() const
{
  std::string request = "USER "+AuthConfigFactorySingleton::instance()->createAuthConfig(req)->getUser()+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendPass() const
{
  std::string request = "PASS "+AuthConfigFactorySingleton::instance()->createAuthConfig(req)->getPassword()+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, "PASS ********");
  socket->writeData(request);
}

void FtpConnection::sendType() const
{
  std::string type;
  if(option->get(PREF_FTP_TYPE) == V_ASCII) {
    type = "A";
  } else {
    type = "I";
  }
  std::string request = "TYPE "+type+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendCwd() const
{
  std::string request = "CWD "+Util::urldecode(req->getDir())+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendSize() const
{
  std::string request = "SIZE "+Util::urldecode(req->getFile())+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendPasv() const
{
  std::string request = "PASV\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

SocketHandle FtpConnection::sendPort() const
{
  SocketHandle serverSocket;
  serverSocket->bind(0);
  serverSocket->beginListen();
  serverSocket->setNonBlockingMode();

  std::pair<std::string, int32_t> addrinfo;
  socket->getAddrInfo(addrinfo);
  int32_t ipaddr[4]; 
  sscanf(addrinfo.first.c_str(), "%d.%d.%d.%d",
	 &ipaddr[0], &ipaddr[1], &ipaddr[2], &ipaddr[3]);
  serverSocket->getAddrInfo(addrinfo);
  std::string request = "PORT "+
    Util::itos(ipaddr[0])+","+Util::itos(ipaddr[1])+","+
    Util::itos(ipaddr[2])+","+Util::itos(ipaddr[3])+","+
    Util::itos(addrinfo.second/256)+","+Util::itos(addrinfo.second%256)+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
  return serverSocket;
}

void FtpConnection::sendRest(const SegmentHandle& segment) const
{
  std::string request = "REST "+Util::itos(segment->getPositionToWrite())+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendRetr() const
{
  std::string request = "RETR "+Util::urldecode(req->getFile())+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

int32_t FtpConnection::getStatus(const std::string& response) const
{
  int32_t status;
  // When the response is not like "%d %*s",
  // we return 0.
  if(response.find_first_not_of("0123456789") != 3
     || !(response.find(" ") == 3 || response.find("-") == 3)) {
    return 0;
  }
  if(sscanf(response.c_str(), "%d %*s", &status) == 1) {
    return status;
  } else {
    return 0;
  }
}

bool FtpConnection::isEndOfResponse(int32_t status, const std::string& response) const
{
  if(response.size() <= 4) {
    return false;
  }
  // if 4th character of buf is '-', then multi line response is expected.
  if(response.at(3) == '-') {
    // multi line response
    std::string::size_type p;
    p = response.find("\r\n"+Util::itos(status)+" ");
    if(p == std::string::npos) {
      return false;
    }
  }
  if(Util::endsWith(response, "\r\n")) {
    return true;
  } else {
    return false;
  }
}

bool FtpConnection::bulkReceiveResponse(std::pair<int32_t, std::string>& response)
{
  char buf[1024];  
  while(socket->isReadable(0)) {
    int32_t size = sizeof(buf)-1;
    socket->readData(buf, size);
    if(size == 0) {
      throw new DlRetryEx(EX_GOT_EOF);
    }
    buf[size] = '\0';
    strbuf += buf;
  }
  int32_t status;
  if(strbuf.size() >= 4) {
    status = getStatus(strbuf);
    if(status == 0) {
      throw new DlAbortEx(EX_INVALID_RESPONSE);
    }
  } else {
    return false;
  }
  if(isEndOfResponse(status, strbuf)) {
    logger->info(MSG_RECEIVE_RESPONSE, cuid, strbuf.c_str());
    response.first = status;
    response.second = strbuf;
    strbuf.erase();
    return true;
  } else {
    // didn't receive response fully.
    return false;
  }
}

int32_t FtpConnection::receiveResponse()
{
  std::pair<int32_t, std::string> response;
  if(bulkReceiveResponse(response)) {
    return response.first;
  } else {
    return 0;
  }
}

#ifdef __MINGW32__
# define LONGLONG_PRINTF "%I64d"
# define ULONGLONG_PRINTF "%I64u"
# define LONGLONG_SCANF "%I64d"
# define ULONGLONG_SCANF "%I64u"
#else
# define LONGLONG_PRINTF "%lld"
# define ULONGLONG_PRINTF "%llu"
# define LONGLONG_SCANF "%Ld"
# define ULONGLONG_SCANF "%Lu"
#endif // __MINGW32__

int32_t FtpConnection::receiveSizeResponse(int64_t& size)
{
  std::pair<int32_t, std::string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 213) {
      sscanf(response.second.c_str(), "%*d " LONGLONG_SCANF, &size);
    }
    return response.first;
  } else {
    return 0;
  }
}

int32_t FtpConnection::receivePasvResponse(std::pair<std::string, int32_t>& dest)
{
  std::pair<int32_t, std::string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 227) {
      // we assume the format of response is "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)."
      int32_t h1, h2, h3, h4, p1, p2;
      std::string::size_type p = response.second.find("(");
      if(p >= 4) {
	sscanf(response.second.substr(response.second.find("(")).c_str(),
	       "(%d,%d,%d,%d,%d,%d).",
	       &h1, &h2, &h3, &h4, &p1, &p2);
	// ip address
	dest.first = Util::itos(h1)+"."+Util::itos(h2)+"."+Util::itos(h3)+"."+Util::itos(h4);
	// port number
	dest.second = 256*p1+p2;
      } else {
	throw new DlRetryEx(EX_INVALID_RESPONSE);
      }
    }
    return response.first;
  } else {
    return 0;
  }
}

} // namespace aria2
