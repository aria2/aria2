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
#include "DlRetryEx.h"
#include "Util.h"
#include "Base64.h"
#include "message.h"
#include "prefs.h"

HttpConnection::HttpConnection(int cuid, const Socket* socket, const Request* req, const Option* op, const Logger* logger):
  cuid(cuid), socket(socket), req(req), option(op), logger(logger) {}

void HttpConnection::sendRequest(const Segment& segment) const {
  string request = createRequest(segment);
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request.c_str(), request.size());
}

void HttpConnection::sendProxyRequest() const {
  string request =
    string("CONNECT ")+req->getHost()+":"+Util::llitos(req->getPort())+
    string(" HTTP/1.1\r\n")+
    "Host: "+getHost(req->getHost(), req->getPort())+"\r\n";
  if(useProxyAuth()) {
    request += "Proxy-Authorization: Basic "+
      Base64::encode(option->get(PREF_HTTP_PROXY_USER)+":"+
		     option->get(PREF_HTTP_PROXY_PORT))+"\r\n";
  }
  request += "\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request.c_str(), request.size());
}

string HttpConnection::getHost(const string& host, int port) const {
  return host+(port == 80 || port == 443 ? "" : ":"+Util::llitos(port));
}

string HttpConnection::createRequest(const Segment& segment) const {
  string request = string("GET ")+
    (req->getProtocol() == "ftp" ?
     req->getCurrentUrl() :
     ((req->getDir() == "/" ? "/" : req->getDir()+"/")+req->getFile()))+
    string(" HTTP/1.1\r\n")+
    "User-Agent: aria2\r\n"+
    "Connection: close\r\n"+
    "Accept: */*\r\n"+        /* */
    "Host: "+getHost(req->getHost(), req->getPort())+"\r\n"+
    "Pragma: no-cache\r\n"+
    "Cache-Control: no-cache\r\n";
  if(segment.sp+segment.ds > 0) {
    request += "Range: bytes="+
      Util::llitos(segment.sp+segment.ds)+"-"+Util::llitos(segment.ep)+"\r\n";
  }
  if(option->get(PREF_HTTP_AUTH_SCHEME) == V_BASIC) {
    request += "Authorization: Basic "+
      Base64::encode(option->get(PREF_HTTP_USER)+":"+
		     option->get(PREF_HTTP_PASSWD))+"\r\n";
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
  char buf[512];
  while(socket->isReadable(0)) {
    int size = sizeof(buf)-1;
    socket->peekData(buf, size);
    buf[size] = '\0';
    int hlenTemp = header.size();
    header += buf;
    string::size_type p;
    if((p = header.find("\r\n\r\n")) == string::npos) {
      socket->readData(buf, size);
    } else {
      if(Util::endsWith(header, "\r\n\r\n")) {
	socket->readData(buf, size);
      } else {
	header.erase(p+4);
	size = p+4-hlenTemp;
	socket->readData(buf, size);
      }
      break;
    }
  }
  if(!Util::endsWith(header, "\r\n\r\n")) {
    return 0;
  }
  // OK, we got all headers.
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
    headers.put(hp.first, hp.second);
  }
  return (int)strtol(status.c_str(), NULL, 10);
}

bool HttpConnection::useProxy() const {
  return option->get(PREF_HTTP_PROXY_ENABLED) == V_TRUE;
}

bool HttpConnection::useProxyAuth() const {
  return option->get(PREF_HTTP_PROXY_AUTH_ENABLED) == V_TRUE;
}
