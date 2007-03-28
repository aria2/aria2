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
#include "Util.h"
#include "Base64.h"
#include "prefs.h"

RangeHandle HttpRequest::getRange() const
{
  // content-length is always 0
  if(segment->isNull()) {
    return new Range(0, 0, 0);
  } else {
    return new Range(getStartByte(), getEndByte(), entityLength);
  }
}

bool HttpRequest::isRangeSatisfied(const RangeHandle& range) const
{
  if(segment->isNull()) {
    return true;
  }
  if(getStartByte() == range->getStartByte() &&
     (getEndByte() == 0 ||
      getEndByte() > 0 && getEndByte() == range->getEndByte()) &&
     (entityLength == 0 ||
      entityLength > 0 && entityLength == range->getEntityLength())) {
    return true;
  } else {
    return false;
  }  
}

string HttpRequest::getHostText(const string& host, in_port_t port) const
{
  return  host+(port == 80 || port == 443 ? "" : ":"+Util::llitos(port));
}

string HttpRequest::createRequest() const
{
  string requestLine = "GET ";
  if(getProtocol() == "ftp" || proxyEnabled) {
    requestLine += getCurrentURI();
  } else {
    if(getDir() == "/") {
      requestLine += getDir();
    } else {
      requestLine += getDir()+"/";
    }
    requestLine += getFile();
  }
  requestLine +=
    string(" HTTP/1.1\r\n")+
    "User-Agent: "+userAgent+"\r\n"+
    "Accept: */*\r\n"+        /* */
    "Host: "+getHostText(getHost(), getPort())+"\r\n"+
    "Pragma: no-cache\r\n"+
    "Cache-Control: no-cache\r\n";
  if(!request->isKeepAlive()) {
    requestLine += "Connection: close\r\n";
  }
  if(segment->length > 0) {
    requestLine += "Range: bytes="+Util::llitos(getStartByte());
    requestLine += "-";
    if(request->isKeepAlive()) {
      requestLine += Util::llitos(getEndByte());
    }
    requestLine += "\r\n";
  }
  if(proxyEnabled) {
    requestLine += "Proxy-Connection: close\r\n";
  }
  if(proxyEnabled && proxyAuthEnabled) {
    requestLine += getProxyAuthString();
  }
  if(authEnabled) {
    requestLine += "Authorization: Basic "+
	Base64::encode(request->resolveHttpAuthConfig()->getAuthText())+"\r\n";
  }
  if(getPreviousURI().size()) {
    requestLine += "Referer: "+getPreviousURI()+"\r\n";
  }

  string cookiesValue;
  Cookies cookies = request->cookieBox->criteriaFind(getHost(),
						     getDir(),
						     getProtocol() == "https" ?
						     true : false);
  for(Cookies::const_iterator itr = cookies.begin(); itr != cookies.end(); itr++) {
    cookiesValue += (*itr).toString()+";";
  }
  if(cookiesValue.size()) {
    requestLine += string("Cookie: ")+cookiesValue+"\r\n";
  }
  requestLine += "\r\n";
  return requestLine;
}

string HttpRequest::createProxyRequest() const
{
  string requestLine =
    string("CONNECT ")+getHost()+":"+Util::itos(getPort())+
    string(" HTTP/1.1\r\n")+
    "User-Agent: "+Util::urlencode(userAgent)+"\r\n"+
    "Proxy-Connection: close\r\n"+
    "Host: "+getHost()+":"+Util::itos(getPort())+"\r\n";
  if(proxyAuthEnabled) {
    requestLine += getProxyAuthString();
  }
  requestLine += "\r\n";
  return requestLine;
}

string HttpRequest::getProxyAuthString() const {
  return "Proxy-Authorization: Basic "+
    Base64::encode(request->resolveHttpProxyAuthConfig()->getAuthText())+"\r\n";
}

void HttpRequest::configure(const Option* option)
{
  authEnabled = option->get(PREF_HTTP_AUTH_ENABLED) == V_TRUE;
  proxyEnabled =
    option->get(PREF_HTTP_PROXY_ENABLED) == V_TRUE &&
    option->get(PREF_HTTP_PROXY_METHOD) == V_GET;
  proxyAuthEnabled = option->get(PREF_HTTP_PROXY_AUTH_ENABLED) == V_TRUE;
}
