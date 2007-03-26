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
#include <sstream>

HttpConnection::HttpConnection(int cuid,
			       const SocketHandle& socket,
			       const Option* op):
  cuid(cuid), socket(socket), option(op), headerBufLength(0) {
  logger = LogFactory::getInstance();
}

string HttpConnection::eraseConfidentialInfo(const string& request)
{
  istringstream istr(request);
  ostringstream ostr;
  string line;
  while(getline(istr, line)) {
    if(Util::startsWith(line, "Authorization: Basic")) {
      ostr << "Authorization: Basic ********\n";
    } else if(Util::startsWith(line, "Proxy-Authorization: Basic")) {
      ostr << "Proxy-Authorization: Basic ********\n";
    } else {
      ostr << line << "\n";
    }
  }
  return ostr.str();
}

void HttpConnection::sendRequest(const HttpRequestHandle& httpRequest)
{
  string request = httpRequest->createRequest();
  logger->info(MSG_SENDING_REQUEST, cuid, eraseConfidentialInfo(request).c_str());
  socket->writeData(request.c_str(), request.size());
  outstandingHttpRequests.push_back(httpRequest);
}

void HttpConnection::sendProxyRequest(const HttpRequestHandle& httpRequest)
{
  string request = httpRequest->createProxyRequest();
  logger->info(MSG_SENDING_REQUEST, cuid, eraseConfidentialInfo(request).c_str());
  socket->writeData(request.c_str(), request.size());
  outstandingHttpRequests.push_back(httpRequest);
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

HttpResponseHandle HttpConnection::receiveResponse() {
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
  HttpHeaderHandle httpHeader = new HttpHeader();
  // retreive status name-value pairs, then push these into map
  while((np = header.find(delimiters[delimiterSwitch], p)) != string::npos && np != p) {
    string line = header.substr(p, np-p);
    p = np+2;
    pair<string, string> hp;
    Util::split(hp, line, ':');
    httpHeader->put(hp.first, hp.second);
  }
  HttpResponseHandle httpResponse = new HttpResponse();
  httpResponse->setCuid(cuid);
  httpResponse->setStatus(strtol(status.c_str(), 0, 10));
  httpResponse->setHttpHeader(httpHeader);
  httpResponse->setHttpRequest(outstandingHttpRequests.front());

  outstandingHttpRequests.pop_front();

  return httpResponse;
}
