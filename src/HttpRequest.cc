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
#include "HttpRequest.h"

#include <cassert>
#include <numeric>

#include "Request.h"
#include "Segment.h"
#include "Range.h"
#include "CookieStorage.h"
#include "Option.h"
#include "Util.h"
#include "Base64.h"
#include "prefs.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include "TimeA2.h"

namespace aria2 {

const std::string HttpRequest::USER_AGENT("aria2");

HttpRequest::HttpRequest():entityLength(0),
			   _contentEncodingEnabled(true),
			   userAgent(USER_AGENT)
{}

SharedHandle<Segment> HttpRequest::getSegment() const
{
  return segment;
}

void HttpRequest::setSegment(const SharedHandle<Segment>& segment)
{
  this->segment = segment;
}

void HttpRequest::setRequest(const SharedHandle<Request>& request)
{
  this->request = request;
}

SharedHandle<Request> HttpRequest::getRequest() const
{
  return request;
}

off_t HttpRequest::getStartByte() const
{
  if(segment.isNull()) {
    return 0;
  } else {
    return segment->getPositionToWrite();
  }
}

off_t HttpRequest::getEndByte() const
{
  if(segment.isNull() || request.isNull()) {
    return 0;
  } else {
    if(request->isPipeliningEnabled()) {
      return segment->getPosition()+segment->getLength()-1;
    } else {
      return 0;
    }
  }
}

RangeHandle HttpRequest::getRange() const
{
  // content-length is always 0
  if(segment.isNull()) {
    return SharedHandle<Range>(new Range());
  } else {
    return SharedHandle<Range>(new Range(getStartByte(), getEndByte(), entityLength));
  }
}

bool HttpRequest::isRangeSatisfied(const RangeHandle& range) const
{
  if(segment.isNull()) {
    return true;
  }
  if((getStartByte() == range->getStartByte()) &&
     ((getEndByte() == 0) ||
      ((getEndByte() > 0) && (getEndByte() == range->getEndByte()))) &&
     ((entityLength == 0) ||
      ((entityLength > 0) && (entityLength == range->getEntityLength())))) {
    return true;
  } else {
    return false;
  }  
}

std::string HttpRequest::getHostText(const std::string& host, uint16_t port) const
{
  return  host+(port == 80 || port == 443 ? "" : ":"+Util::uitos(port));
}

std::string HttpRequest::createRequest() const
{
  SharedHandle<AuthConfig> authConfig =
    _authConfigFactory->createAuthConfig(request);
  std::string requestLine = "GET ";
  if(!_proxyRequest.isNull()) {
    if(getProtocol() == Request::PROTO_FTP &&
       request->getUsername().empty() && !authConfig->getUser().empty()) {
      // Insert user into URI, like ftp://USER@host/
      std::string uri = getCurrentURI();
      assert(uri.size() >= 6);
      uri.insert(6, Util::urlencode(authConfig->getUser())+"@");
      requestLine += uri;
    } else {
      requestLine += getCurrentURI();
    }
  } else {
    if(getDir() == A2STR::SLASH_C) {
      requestLine += getDir();
    } else {
      requestLine += getDir()+A2STR::SLASH_C;
    }
    requestLine += getFile()+getQuery();
  }
  requestLine +=
    std::string(" HTTP/1.1\r\n")+
    "User-Agent: "+userAgent+"\r\n";
  
  requestLine += "Accept: */*"; /* */
  for(std::deque<std::string>::const_iterator i = _acceptTypes.begin();
      i != _acceptTypes.end(); ++i) {
    requestLine += ","+(*i);
  }
  requestLine += "\r\n";

  if(_contentEncodingEnabled) {
    std::string acceptableEncodings;
#ifdef HAVE_LIBZ
    acceptableEncodings += "deflate, gzip";
#endif // HAVE_LIBZ
    if(!acceptableEncodings.empty()) {
      requestLine += "Accept-Encoding: "+acceptableEncodings+"\r\n";
    }
  }

  requestLine +=
    "Host: "+getHostText(getHost(), getPort())+"\r\n"+
    "Pragma: no-cache\r\n"+
    "Cache-Control: no-cache\r\n";

  if(!request->isKeepAliveEnabled() && !request->isPipeliningEnabled()) {
    requestLine += "Connection: close\r\n";
  }
  if(!segment.isNull() && segment->getLength() > 0 && 
     (request->isPipeliningEnabled() || getStartByte() > 0)) {
    requestLine += "Range: bytes="+Util::itos(getStartByte());
    requestLine += "-";
    if(request->isPipeliningEnabled()) {
      requestLine += Util::itos(getEndByte());
    }
    requestLine += "\r\n";
  }
  if(!_proxyRequest.isNull()) {
    if(request->isKeepAliveEnabled() || request->isPipeliningEnabled()) {
      requestLine += "Proxy-Connection: Keep-Alive\r\n";
    } else {
      requestLine += "Proxy-Connection: close\r\n";
    }
  }
  if(!_proxyRequest.isNull() && !_proxyRequest->getUsername().empty()) {
    requestLine += getProxyAuthString();
  }
  if(!authConfig->getUser().empty()) {
    requestLine += "Authorization: Basic "+
      Base64::encode(authConfig->getAuthText())+"\r\n";
  }
  if(getPreviousURI().size()) {
    requestLine += "Referer: "+getPreviousURI()+"\r\n";
  }
  if(!_cookieStorage.isNull()) {
    std::string cookiesValue;
    std::deque<Cookie> cookies =
      _cookieStorage->criteriaFind(getHost(),
				   getDir(),
				   Time().getTime(),
				   getProtocol() == Request::PROTO_HTTPS ?
				   true : false);
    for(std::deque<Cookie>::const_iterator itr = cookies.begin();
	itr != cookies.end(); ++itr) {
      cookiesValue += (*itr).toString()+";";
    }
    if(!cookiesValue.empty()) {
      requestLine += std::string("Cookie: ")+cookiesValue+"\r\n";
    }
  }
  // append additional headers given by user.
  for(std::deque<std::string>::const_iterator i = _headers.begin();
      i != _headers.end(); ++i) {
    requestLine += (*i)+"\r\n";
  }

  requestLine += "\r\n";
  return requestLine;
}

std::string HttpRequest::createProxyRequest() const
{
  assert(!_proxyRequest.isNull());
  std::string requestLine =
    std::string("CONNECT ")+getHost()+":"+Util::uitos(getPort())+
    std::string(" HTTP/1.1\r\n")+
    "User-Agent: "+userAgent+"\r\n"+
    "Host: "+getHost()+":"+Util::uitos(getPort())+"\r\n";
  // TODO Is "Proxy-Connection" needed here?
//   if(request->isKeepAliveEnabled() || request->isPipeliningEnabled()) {
//     requestLine += "Proxy-Connection: Keep-Alive\r\n";
//   }else {
//     requestLine += "Proxy-Connection: close\r\n";
//   }
  if(!_proxyRequest->getUsername().empty()) {
    requestLine += getProxyAuthString();
  }
  requestLine += "\r\n";
  return requestLine;
}

std::string HttpRequest::getProxyAuthString() const
{
  return "Proxy-Authorization: Basic "+
    Base64::encode(_proxyRequest->getUsername()+":"+
		   _proxyRequest->getPassword())
    +"\r\n";
}

void HttpRequest::enableContentEncoding()
{
  _contentEncodingEnabled = true;
}

void HttpRequest::disableContentEncoding()
{
  _contentEncodingEnabled = false;
}

void HttpRequest::addHeader(const std::string& headersString)
{
  std::deque<std::string> headers;
  Util::slice(headers, headersString, '\n', true);
  _headers.insert(_headers.end(), headers.begin(), headers.end());
}

void HttpRequest::addAcceptType(const std::string& type)
{
  _acceptTypes.push_back(type);
}

const std::string& HttpRequest::getPreviousURI() const
{
  return request->getPreviousUrl();
}

const std::string& HttpRequest::getHost() const
{
  return request->getHost();
}

uint16_t HttpRequest::getPort() const
{
  return request->getPort();
}

const std::string& HttpRequest::getMethod() const
{
  return request->getMethod();
}

const std::string& HttpRequest::getProtocol() const
{
  return request->getProtocol();
}

const std::string& HttpRequest::getCurrentURI() const
{
  return request->getCurrentUrl();
}
  
const std::string& HttpRequest::getDir() const
{
  return request->getDir();
}

const std::string& HttpRequest::getFile() const
{
  return request->getFile();
}

const std::string& HttpRequest::getQuery() const
{
  return request->getQuery();
}

void HttpRequest::setCookieStorage
(const SharedHandle<CookieStorage>& cookieStorage)
{
  _cookieStorage = cookieStorage;
}

SharedHandle<CookieStorage> HttpRequest::getCookieStorage() const
{
  return _cookieStorage;
}

void HttpRequest::setAuthConfigFactory
(const SharedHandle<AuthConfigFactory>& factory)
{
  _authConfigFactory = factory;
}

void HttpRequest::setProxyRequest(const SharedHandle<Request>& proxyRequest)
{
  _proxyRequest = proxyRequest;
}

bool HttpRequest::isProxyRequestSet() const
{
  return !_proxyRequest.isNull();
}

} // namespace aria2
