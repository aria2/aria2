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
#include "Request.h"
#include "Segment.h"
#include "Range.h"
#include "Cookie.h"
#include "CookieBox.h"
#include "Option.h"
#include "Util.h"
#include "Base64.h"
#include "prefs.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include <numeric>

namespace aria2 {

const std::string HttpRequest::USER_AGENT("aria2");

HttpRequest::HttpRequest():entityLength(0),
			   authEnabled(false),
			   proxyEnabled(false),
			   proxyAuthEnabled(false),
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
  std::string requestLine = "GET ";
  if(getProtocol() == Request::PROTO_FTP || proxyEnabled) {
    requestLine += getCurrentURI();
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
  if(proxyEnabled) {
    if(request->isKeepAliveEnabled() || request->isPipeliningEnabled()) {
      requestLine += "Proxy-Connection: Keep-Alive\r\n";
    } else {
      requestLine += "Proxy-Connection: close\r\n";
    }
  }
  if(proxyEnabled && proxyAuthEnabled) {
    requestLine += getProxyAuthString();
  }
  if(authEnabled) {
    requestLine += "Authorization: Basic "+
      Base64::encode(AuthConfigFactorySingleton::instance()->createAuthConfig(request)->getAuthText())+"\r\n";
  }
  if(getPreviousURI().size()) {
    requestLine += "Referer: "+getPreviousURI()+"\r\n";
  }
  std::string cookiesValue;
  Cookies cookies = request->cookieBox->criteriaFind(getHost(),
						     getDir(),
						     time(0),
						     getProtocol() == Request::PROTO_HTTPS ?
						     true : false);
  for(Cookies::const_iterator itr = cookies.begin(); itr != cookies.end(); itr++) {
    cookiesValue += (*itr).toString()+";";
  }
  if(cookiesValue.size()) {
    requestLine += std::string("Cookie: ")+cookiesValue+"\r\n";
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
  std::string requestLine =
    std::string("CONNECT ")+getHost()+":"+Util::uitos(getPort())+
    std::string(" HTTP/1.1\r\n")+
    "User-Agent: "+userAgent+"\r\n"+
    "Host: "+getHost()+":"+Util::uitos(getPort())+"\r\n";
  if(request->isKeepAliveEnabled() || request->isPipeliningEnabled()) {
    requestLine += "Proxy-Connection: Keep-Alive\r\n";
  }else {
    requestLine += "Proxy-Connection: close\r\n";
  }
  if(proxyAuthEnabled) {
    requestLine += getProxyAuthString();
  }
  requestLine += "\r\n";
  return requestLine;
}

std::string HttpRequest::getProxyAuthString() const {
  return "Proxy-Authorization: Basic "+
    Base64::encode(AuthConfigFactorySingleton::instance()->createAuthConfigForHttpProxy(request)->getAuthText())+"\r\n";
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

void HttpRequest::configure(const Option* option)
{
  authEnabled = option->getAsBool(PREF_HTTP_AUTH_ENABLED);
  proxyEnabled =
    option->getAsBool(PREF_HTTP_PROXY_ENABLED) &&
    option->get(PREF_HTTP_PROXY_METHOD) == V_GET;
  proxyAuthEnabled = option->getAsBool(PREF_HTTP_PROXY_AUTH_ENABLED);
}

std::string HttpRequest::getPreviousURI() const
{
  return request->getPreviousUrl();
}

std::string HttpRequest::getHost() const
{
  return request->getHost();
}

uint16_t HttpRequest::getPort() const
{
  return request->getPort();
}

std::string HttpRequest::getMethod() const
{
  return request->getMethod();
}

std::string HttpRequest::getProtocol() const
{
  return request->getProtocol();
}

std::string HttpRequest::getCurrentURI() const
{
  return request->getCurrentUrl();
}
  
std::string HttpRequest::getDir() const
{
  return request->getDir();
}

std::string HttpRequest::getFile() const
{
  return request->getFile();
}

std::string HttpRequest::getQuery() const
{
  return request->getQuery();
}

} // namespace aria2
