/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "FtpConnection.h"
#include "Util.h"
#include "DlAbortEx.h"
#include "message.h"

FtpConnection::FtpConnection(int cuid, const Socket* socket, const Request* req, const Option* op, const Logger* logger):cuid(cuid), socket(socket), req(req), option(op), logger(logger) {}

FtpConnection::~FtpConnection() {}

void FtpConnection::sendUser() const {
  string request = "USER "+option->get("ftp_user")+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendPass() const {
  string request = "PASS "+option->get("ftp_passwd")+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, "PASS ********");
  socket->writeData(request);
}

void FtpConnection::sendType() const {
  string request = "TYPE "+option->get("ftp_type")+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendCwd() const {
  string request = "CWD "+req->getDir()+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendSize() const {
  string request = "SIZE "+req->getFile()+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendPasv() const {
  string request = "PASV\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

Socket* FtpConnection::sendPort() const {
  Socket* serverSocket = new Socket();
  try {
    serverSocket->beginListen();

    pair<string, int> addrinfo;
    socket->getAddrInfo(addrinfo);
    int ipaddr[4]; 
    sscanf(addrinfo.first.c_str(), "%d.%d.%d.%d",
	   &ipaddr[0], &ipaddr[1], &ipaddr[2], &ipaddr[3]);
    serverSocket->getAddrInfo(addrinfo);
    string request = "PORT "+
      Util::itos(ipaddr[0])+","+Util::itos(ipaddr[1])+","+
      Util::itos(ipaddr[2])+","+Util::itos(ipaddr[3])+","+
      Util::itos(addrinfo.second/256)+","+Util::itos(addrinfo.second%256)+"\r\n";
    logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
    socket->writeData(request);
  } catch (Exception* ex) {
    delete serverSocket;
    throw;
  }
  return serverSocket;
}

void FtpConnection::sendRest(const Segment& segment) const {
  string request = "REST "+Util::llitos(segment.sp+segment.ds)+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

void FtpConnection::sendRetr() const {
  string request = "RETR "+req->getFile()+"\r\n";
  logger->info(MSG_SENDING_FTP_REQUEST, cuid, request.c_str());
  socket->writeData(request);
}

int FtpConnection::getStatus(string response) const {
  int status;
  // TODO we must handle when the response is not like "%d %*s"
  // In this case, we return 0
  if(sscanf(response.c_str(), "%d %*s", &status) == 1) {
    return status;
  } else {
    return 0;
  }
}

bool FtpConnection::isEndOfResponse(int status, string response) const {
  if(response.size() <= 4) {
    return false;
  }
  // if forth character of buf is '-', then multi line response is expected.
  if(response.at(3) == '-') {
    // multi line response
    string::size_type p;
    p = response.find("\r\n"+Util::itos(status)+" ");
    if(p == string::npos) {
      return false;
    }
  }
  if(Util::endsWith(response, "\r\n")) {
    return true;
  } else {
    return false;
  }
}

bool FtpConnection::bulkReceiveResponse(pair<int, string>& response) {
  char buf[1024];  
  while(socket->isReadable(0)) {
    int size = sizeof(buf)-1;
    socket->readData(buf, size);
    buf[size] = '\0';
    strbuf += buf;
  }
  int status;
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

int FtpConnection::receiveResponse() {
  pair<int, string> response;
  if(bulkReceiveResponse(response)) {
    return response.first;
  } else {
    return 0;
  }
}

int FtpConnection::receiveSizeResponse(long long int& size) {
  pair<int, string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 213) {
      sscanf(response.second.c_str(), "%*d %Ld", &size);
    }
    return response.first;
  } else {
    return 0;
  }
}

int FtpConnection::receivePasvResponse(pair<string, int>& dest) {
  pair<int, string> response;
  if(bulkReceiveResponse(response)) {
    if(response.first == 227) {
      // we assume the format of response is "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)."
      int h1, h2, h3, h4, p1, p2;
      string::size_type p = response.second.find("(");
      if(p >= 4) {
	sscanf(response.second.substr(response.second.find("(")).c_str(),
	       "(%d,%d,%d,%d,%d,%d).",
	       &h1, &h2, &h3, &h4, &p1, &p2);
	// ip address
	dest.first = Util::itos(h1)+"."+Util::itos(h2)+"."+Util::itos(h3)+"."+Util::itos(h4);
	// port number
	dest.second = 256*p1+p2;
      } else {
	throw new DlAbortEx(EX_INVALID_RESPONSE);
      }
    }
    return response.first;
  } else {
    return 0;
  }
}
