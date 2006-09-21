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
#include "HttpConnection.h"
#include "DlRetryEx.h"
#include "Util.h"
#include "Base64.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"

HttpConnection::HttpConnection(int cuid, const SocketHandle& socket,
			       const Request* req, const Option* op):
  cuid(cuid), socket(socket), req(req), option(op), headerBufLength(0) {
  logger = LogFactory::getInstance();
}

void HttpConnection::sendRequest(const Segment& segment) const {
  string request = createRequest(segment);
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request.c_str(), request.size());
}

void HttpConnection::sendProxyRequest() const {
  string request =
    string("CONNECT ")+req->getHost()+":"+Util::llitos(req->getPort())+
    string(" HTTP/1.1\r\n")+
    "User-Agent: "+USER_AGENT+"\r\n"+
    "Proxy-Connection: close\r\n"+
    "Host: "+getHost(req->getHost(), req->getPort())+"\r\n";
  if(useProxyAuth()) {
    request += getProxyAuthString();
  }
  request += "\r\n";
  logger->info(MSG_SENDING_REQUEST, cuid, request.c_str());
  socket->writeData(request.c_str(), request.size());
}

string HttpConnection::getProxyAuthString() const {
  return "Proxy-Authorization: Basic "+
    Base64::encode(option->get(PREF_HTTP_PROXY_USER)+":"+
		   option->get(PREF_HTTP_PROXY_PASSWD))+"\r\n";
}

string HttpConnection::getHost(const string& host, int port) const {
  return host+(port == 80 || port == 443 ? "" : ":"+Util::llitos(port));
}

string HttpConnection::createRequest(const Segment& segment) const {
  string request = string("GET ")+
    (req->getProtocol() == "ftp" || useProxy() && useProxyGet() ?
     req->getCurrentUrl() :
     ((req->getDir() == "/" ? "/" : req->getDir()+"/")+req->getFile()))+
    string(" HTTP/1.1\r\n")+
    "User-Agent: "+USER_AGENT+"\r\n"+
    // use persistent connection
    //"Connection: close\r\n"+
    "Accept: */*\r\n"+        /* */
    "Host: "+getHost(req->getHost(), req->getPort())+"\r\n"+
    "Pragma: no-cache\r\n"+
    "Cache-Control: no-cache\r\n";
  if(!req->isKeepAlive()) {
    request += "Connection: close\r\n";
  }
  if(segment.length > 0) {
    request += "Range: bytes="+
      Util::llitos(segment.getPosition()+segment.writtenLength);
    request += "-";
    if(req->isKeepAlive()) {
      request += Util::llitos(segment.getPosition()+segment.length-1);
    }
    request += "\r\n";
  }
  if(useProxy() && useProxyAuth() && useProxyGet()) {
    request += "Proxy-Connection: close\r\n";
    request += getProxyAuthString();
  }
  if(option->get(PREF_HTTP_AUTH_ENABLED) == V_TRUE) {
    if(option->get(PREF_HTTP_AUTH_SCHEME) == V_BASIC) {
      request += "Authorization: Basic "+
	Base64::encode(option->get(PREF_HTTP_USER)+":"+
		       option->get(PREF_HTTP_PASSWD))+"\r\n";
    }
  }
  if(req->getPreviousUrl().size()) {
    request += "Referer: "+req->getPreviousUrl()+"\r\n";
  }

  string cookiesValue;
  Cookies cookies = req->cookieBox->criteriaFind(req->getHost(), req->getDir(), req->getProtocol() == "https" ? true : false);
  for(Cookies::const_iterator itr = cookies.begin(); itr != cookies.end(); itr++) {
    cookiesValue += (*itr).toString()+";";
  }
  if(cookiesValue.size()) {
    request += string("Cookie: ")+cookiesValue+"\r\n";
  }
  request += "\r\n";
  return request;
}

int HttpConnection::findEndOfHeader(const char* buf, const char* substr, int bufLength) const {
  const char* p = buf;
  while(bufLength > p-buf && bufLength-(p-buf) >= (int)strlen(substr)) {
    if(memcmp(p, substr, strlen(substr)) == 0) {
      return p-buf;
    }
    p++;
  }
  return -1;
}

int HttpConnection::receiveResponse(HttpHeader& headers) {
  //char buf[512];
  string header;
  int delimiterSwitch = 0;
  char* delimiters[] = { "\r\n", "\n" };

  int size = HEADERBUF_SIZE-headerBufLength;
  if(size < 0) {
    // TODO too large header
    throw new DlRetryEx("too large header > 4096");
  }
  socket->peekData(headerBuf+headerBufLength, size);
  if(size == 0) {
    throw new DlRetryEx(EX_INVALID_RESPONSE);
  }
  //buf[size] = '\0';
  int hlenTemp = headerBufLength+size;
  //header += buf;
  //string::size_type p;
  int eohIndex;

  if((eohIndex = findEndOfHeader(headerBuf, "\r\n\r\n", hlenTemp)) == -1 &&
     (eohIndex = findEndOfHeader(headerBuf, "\n\n", hlenTemp)) == -1) {
    socket->readData(headerBuf+headerBufLength, size);
    headerBufLength += size;
  } else {
    if(headerBuf[eohIndex] == '\n') {
      // for crapping non-standard HTTP server
      delimiterSwitch = 1;
    } else {
      delimiterSwitch = 0;
    }
    headerBuf[eohIndex+strlen(delimiters[delimiterSwitch])*2] = '\0';
    header = headerBuf;
    size = eohIndex+strlen(delimiters[delimiterSwitch])*2-headerBufLength;
    socket->readData(headerBuf+headerBufLength, size);
  }
  if(!Util::endsWith(header, "\r\n\r\n") && !Util::endsWith(header, "\n\n")) {
    return 0;
  }
  // OK, we got all headers.
  logger->info(MSG_RECEIVE_RESPONSE, cuid, header.c_str());
  string::size_type p, np;
  p = np = 0;
  np = header.find(delimiters[delimiterSwitch], p);
  if(np == string::npos) {
    throw new DlRetryEx(EX_NO_STATUS_HEADER);
  }
  // check HTTP status value
  if(header.size() <= 12) {
    throw new DlRetryEx(EX_NO_STATUS_HEADER);
  }
  string status = header.substr(9, 3);
  p = np+2;
  // retreive status name-value pairs, then push these into map
  while((np = header.find(delimiters[delimiterSwitch], p)) != string::npos && np != p) {
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

bool HttpConnection::useProxyGet() const {
  return option->get(PREF_HTTP_PROXY_METHOD) == V_GET;
}
