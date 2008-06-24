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
#include "A2STR.h"

namespace aria2 {

const std::string FtpConnection::A("A");

const std::string FtpConnection::I("I");

FtpConnection::FtpConnection(int32_t cuid, const SocketHandle& socket,
			     const RequestHandle& req, const Option* op):
  cuid(cuid), socket(socket), req(req), option(op),
  logger(LogFactory::getInstance()) {}

FtpConnection::~FtpConnection() {}

void FtpConnection::sendUser() const
{
  std::string request = "USER "+AuthConfigFactorySingleton::instance()->createAuthConfig(req)->getUser()+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, "USER ********");
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
    type = FtpConnection::A;
  } else {
    type = FtpConnection::I;
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
  static const std::string request("PASV\r\n");
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

SocketHandle FtpConnection::sendPort() const
{
  SocketHandle serverSocket(new SocketCore());
  serverSocket->bind(0);
  serverSocket->beginListen();
  serverSocket->setNonBlockingMode();

  std::pair<std::string, uint16_t> addrinfo;
  socket->getAddrInfo(addrinfo);
  unsigned int ipaddr[4]; 
  sscanf(addrinfo.first.c_str(), "%u.%u.%u.%u",
	 &ipaddr[0], &ipaddr[1], &ipaddr[2], &ipaddr[3]);
  serverSocket->getAddrInfo(addrinfo);
  std::string request = "PORT "+
    Util::uitos(ipaddr[0])+","+Util::uitos(ipaddr[1])+","+
    Util::uitos(ipaddr[2])+","+Util::uitos(ipaddr[3])+","+
    Util::uitos(addrinfo.second/256)+","+Util::uitos(addrinfo.second%256)+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
  return serverSocket;
}

void FtpConnection::sendRest(const SegmentHandle& segment) const
{
  std::string request = "REST ";
  if(segment.isNull()) {
    request += "0";
  } else {
    request += Util::itos(segment->getPositionToWrite());
  }
  request += "\r\n";

  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendRetr() const
{
  std::string request = "RETR "+Util::urldecode(req->getFile())+"\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

unsigned int FtpConnection::getStatus(const std::string& response) const
{
  unsigned int status;
  // When the response is not like "%u %*s",
  // we return 0.
  if(response.find_first_not_of("0123456789") != 3
     || !(response.find(" ") == 3 || response.find("-") == 3)) {
    return 0;
  }
  if(sscanf(response.c_str(), "%u %*s", &status) == 1) {
    return status;
  } else {
    return 0;
  }
}

bool FtpConnection::isEndOfResponse(unsigned int status, const std::string& response) const
{
  if(response.size() <= 4) {
    return false;
  }
  // if 4th character of buf is '-', then multi line response is expected.
  if(response.at(3) == '-') {
    // multi line response
    std::string::size_type p;
    p = response.find("\r\n"+Util::uitos(status)+" ");
    if(p == std::string::npos) {
      return false;
    }
  }
  if(Util::endsWith(response, A2STR::CRLF)) {
    return true;
  } else {
    return false;
  }
}

bool FtpConnection::bulkReceiveResponse(std::pair<unsigned int, std::string>& response)
{
  char buf[1024];  
  while(socket->isReadable(0)) {
    size_t size = sizeof(buf)-1;
    socket->readData(buf, size);
    if(size == 0) {
      throw DlRetryEx(EX_GOT_EOF);
    }
    buf[size] = '\0';
    strbuf += buf;
  }
  unsigned int status;
  if(strbuf.size() >= 4) {
    status = getStatus(strbuf);
    if(status == 0) {
      throw DlAbortEx(EX_INVALID_RESPONSE);
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

unsigned int FtpConnection::receiveResponse()
{
  std::pair<unsigned int, std::string> response;
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

unsigned int FtpConnection::receiveSizeResponse(uint64_t& size)
{
  std::pair<unsigned int, std::string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 213) {
      sscanf(response.second.c_str(), "%*u " LONGLONG_SCANF, &size);
    }
    return response.first;
  } else {
    return 0;
  }
}

unsigned int FtpConnection::receivePasvResponse(std::pair<std::string, uint16_t>& dest)
{
  std::pair<unsigned int, std::string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 227) {
      // we assume the format of response is "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)."
      unsigned int h1, h2, h3, h4, p1, p2;
      std::string::size_type p = response.second.find("(");
      if(p >= 4) {
	sscanf(response.second.substr(response.second.find("(")).c_str(),
	       "(%u,%u,%u,%u,%u,%u).",
	       &h1, &h2, &h3, &h4, &p1, &p2);
	// ip address
	dest.first = Util::uitos(h1)+"."+Util::uitos(h2)+"."+Util::uitos(h3)+"."+Util::uitos(h4);
	// port number
	dest.second = 256*p1+p2;
      } else {
	throw DlRetryEx(EX_INVALID_RESPONSE);
      }
    }
    return response.first;
  } else {
    return 0;
  }
}

unsigned int FtpConnection::receiveRetrResponse(uint64_t& size)
{
  std::pair<unsigned int, std::string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 150 || response.first == 125) {
      // Attempting to get file size from the response.
      // We assume the response is like:
      // 150 Opening BINARY mode data connection for aria2.tar.bz2 (12345 bytes)
      // If the attempt is failed, size is unchanged.
      std::string& res = response.second;
      std::string::size_type start;
      if((start = res.find_first_of("(")) != std::string::npos &&
	 (start = res.find_first_not_of("( ", start)) != std::string::npos) {

	// now start points to the first byte of the size string.
	std::string::size_type end =
	  res.find_first_not_of("0123456789", start);

	if(end != std::string::npos) {
	  size = Util::parseULLInt(res.substr(start, end-start));
	}
      }
    }
    return response.first;
  } else {
    return 0;
  }
}

} // namespace aria2
