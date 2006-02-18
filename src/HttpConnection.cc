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
#include "HttpConnection.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "Util.h"
#include "Base64.h"
#include "message.h"

HttpConnection::HttpConnection(int cuid, Socket* socket, const Option* op, Logger* logger):cuid(cuid), socket(socket),option(op),logger(logger) {}

void HttpConnection::sendRequest(const Request* req, const Segment& segment) {
  string request = createRequest(req, segment);
  logger->info(MSG_SENDING_HTTP_REQUEST, cuid, request.c_str());
  socket->writeData(request.c_str(), request.size());
}

void HttpConnection::sendProxyRequest(const Request* req) {
  string request = string("CONNECT ")+req->getHost()+":"+Util::llitos(req->getPort())+
    string(" HTTP/1.1\r\n")+
    "Host: "+getHost(req->getHost(), req->getPort())+"\r\n";
  if(useProxyAuth()) {
    request += "Proxy-Authorization: Basic "+
      Base64::encode(option->get("http_proxy_user")+":"+option->get("http_proxy_passwd"))+"\r\n";
  }
  request += "\r\n";
  logger->info(MSG_SENDING_HTTP_REQUEST, cuid, request.c_str());
  socket->writeData(request.c_str(), request.size());
}

string HttpConnection::getHost(const string& host, int port) {
  return host+(port == 80 || port == 443 ? "" : ":"+Util::llitos(port));
}

string HttpConnection::createRequest(const Request* req, const Segment& segment) {
  string request = string("GET ")+
    // some servers cannot permit absolute URI as requet URI.
    //req->getCurrentUrl()+
    (req->getDir() == "/" ? "/" : req->getDir()+"/")+req->getFile()+
    string(" HTTP/1.1\r\n")+
    "User-Agent: aria2\r\n"+
    "Connection: close\r\n"+
    "Accept: */*\r\n"+
    "Host: "+getHost(req->getHost(), req->getPort())+"\r\n"+
    "Pragma: no-cache\r\n"+
    "Cache-Control: no-cache\r\n";
  if(segment.sp+segment.ds > 0) {
    request += "Range: bytes="+Util::llitos(segment.sp+segment.ds)+"-"+Util::llitos(segment.ep)+"\r\n";
  }
  if(option->get("http_auth_scheme") == "BASIC") {
    request += "Authorization: Basic "+
      Base64::encode(option->get("http_user")+":"+option->get("http_passwd"))+"\r\n";
  }
  if(req->getPreviousUrl().size()) {
    request += "Referer: "+req->getPreviousUrl()+"\r\n";
  }

  string cookiesValue;
  vector<Cookie> cookies = req->cookieBox->criteriaFind(req->getHost(), req->getDir(), req->getProtocol() == "https" ? true : false);
  for(vector<Cookie>::const_iterator itr = cookies.begin(); itr != cookies.end(); itr++) {
    cookiesValue += (*itr).toString()+";";
  }
  if(cookiesValue.size()) {
    request += string("Cookie: ")+cookiesValue+"\r\n";
  }
  request += "\r\n";
  return request;
}

int HttpConnection::receiveResponse(HttpHeader& headers) {
  string header;
  char* buf = NULL;
  try {
    // read a line of the header      
    int bufSize = 256;
    // TODO limit iteration count
    while(1) {
      bufSize += 256;
      if(bufSize > 2048) {
	throw new DlAbortEx(EX_INVALID_HEADER);
      }
      buf = new char[bufSize];
      int tbufSize = bufSize-1;
      socket->peekData(buf, tbufSize);
      if(tbufSize > 0) {
	buf[tbufSize] = '\0';
      }
      header = buf;
      char* p;
      if((p = strstr(buf, "\r\n")) == buf) {
	throw new DlAbortEx(EX_NO_HEADER);
      }
      if((p = strstr(buf, "\r\n\r\n")) != NULL) {
	*(p+4) = '\0';
	header = buf;
	tbufSize = header.size();
	socket->readData(buf, tbufSize);
	delete [] buf;
	buf = NULL;
	break;
      } else {
	delete [] buf;
	buf = NULL;
      }
    }
  } catch(Exception* e) {
    if(buf != NULL) {
      delete [] buf;
    }
    throw;
  }
  // OK, i got all headers.
  logger->info(MSG_RECEIVE_RESPONSE, cuid, header.c_str());
  string::size_type p, np;
  p = np = 0;
  np = header.find("\r\n", p);
  if(np == string::npos) {
    throw new DlRetryEx(EX_NO_STATUS_HEADER);
  }
  // check HTTP status value
  string status = header.substr(9, 3);
  p = np+2;
  // retreive status name-value pairs, then push these into map
  while((np = header.find("\r\n", p)) != string::npos && np != p) {
    string line = header.substr(p, np-p);
    p = np+2;
    pair<string, string> hp;
    Util::split(hp, line, ':');
    HttpHeader::value_type nh(hp.first, hp.second);
    headers.insert(nh);
  }
  // TODO rewrite this using strtoul
  return (int)strtol(status.c_str(), NULL, 10);
}

bool HttpConnection::useProxy() {
  return option->defined("http_proxy_enabled") &&
    option->get("http_proxy_enabled") == "true";
}

bool HttpConnection::useProxyAuth() {
  return option->defined("http_proxy_auth_enabled") &&
    option->get("http_proxy_auth_enabled") == "true";
}
