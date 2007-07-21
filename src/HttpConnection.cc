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
#include "DlAbortEx.h"
#include "Util.h"
#include "Base64.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"
#include <sstream>

HttpConnection::HttpConnection(int32_t cuid,
			       const SocketHandle& socket,
			       const Option* op):
  cuid(cuid), socket(socket), option(op), logger(LogFactory::getInstance())
{}

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
  outstandingHttpRequests.push_back(new HttpRequestEntry(httpRequest,
							 new HttpHeaderProcessor()));
}

void HttpConnection::sendProxyRequest(const HttpRequestHandle& httpRequest)
{
  string request = httpRequest->createProxyRequest();
  logger->info(MSG_SENDING_REQUEST, cuid, eraseConfidentialInfo(request).c_str());
  socket->writeData(request.c_str(), request.size());
  outstandingHttpRequests.push_back(new HttpRequestEntry(httpRequest,
							 new HttpHeaderProcessor()));
}

HttpResponseHandle HttpConnection::receiveResponse() {
  if(outstandingHttpRequests.size() == 0) {
    throw new DlAbortEx(EX_NO_HTTP_REQUEST_ENTRY_FOUND);
  }
  HttpRequestEntryHandle entry = outstandingHttpRequests.front();
  HttpHeaderProcessorHandle proc = entry->getHttpHeaderProcessor();

  char buf[512];
  int32_t size = sizeof(buf);
  socket->peekData(buf, size);
  if(size == 0) {
    throw new DlRetryEx(EX_INVALID_RESPONSE);
  }
  proc->update(buf, size);
  if(!proc->eoh()) {
    socket->readData(buf, size);
    return 0;
  }
  int32_t putbackDataLength = proc->getPutBackDataLength();
  size -= putbackDataLength;
  socket->readData(buf, size);

  // OK, we got all headers.
  logger->info(MSG_RECEIVE_RESPONSE, cuid, proc->getHeaderString().c_str());

  pair<string, HttpHeaderHandle> httpStatusHeader = proc->getHttpStatusHeader();
  HttpResponseHandle httpResponse = new HttpResponse();
  httpResponse->setCuid(cuid);
  httpResponse->setStatus(strtol(httpStatusHeader.first.c_str(), 0, 10));
  httpResponse->setHttpHeader(httpStatusHeader.second);
  httpResponse->setHttpRequest(entry->getHttpRequest());

  outstandingHttpRequests.pop_front();

  return httpResponse;
}
