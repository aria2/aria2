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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include <vector>

#include "Segment.h"
#include "Range.h"
#include "CookieStorage.h"
#include "Option.h"
#include "util.h"
#include "base64.h"
#include "prefs.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include "TimeA2.h"
#include "array_fun.h"
#include "Request.h"

namespace aria2 {

const std::string HttpRequest::USER_AGENT("aria2");

HttpRequest::HttpRequest():contentEncodingEnabled_(true),
                           userAgent_(USER_AGENT),
                           noCache_(true),
                           acceptGzip_(false),
                           endOffsetOverride_(0)
{}

HttpRequest::~HttpRequest() {}

void HttpRequest::setSegment(const SharedHandle<Segment>& segment)
{
  segment_ = segment;
}

void HttpRequest::setRequest(const SharedHandle<Request>& request)
{
  request_ = request;
}

int64_t HttpRequest::getStartByte() const
{
  if(!segment_) {
    return 0;
  } else {
    return fileEntry_->gtoloff(segment_->getPositionToWrite());
  }
}

int64_t HttpRequest::getEndByte() const
{
  if(!segment_ || !request_) {
    return 0;
  } else {
    if(request_->isPipeliningEnabled()) {
      int64_t endByte =
        fileEntry_->gtoloff(segment_->getPosition()+segment_->getLength()-1);
      return std::min(endByte, fileEntry_->getLength()-1);
    } else {
      return 0;
    }
  }
}

RangeHandle HttpRequest::getRange() const
{
  // content-length is always 0
  if(!segment_) {
    return SharedHandle<Range>(new Range());
  } else {
    return SharedHandle<Range>(new Range(getStartByte(), getEndByte(),
                                         fileEntry_->getLength()));
  }
}

bool HttpRequest::isRangeSatisfied(const RangeHandle& range) const
{
  if(!segment_) {
    return true;
  }
  if((getStartByte() == range->getStartByte()) &&
     ((getEndByte() == 0) ||
      (getEndByte() == range->getEndByte())) &&
     ((fileEntry_->getLength() == 0) ||
      (fileEntry_->getLength() == range->getEntityLength()))) {
    return true;
  } else {
    return false;
  }  
}

namespace {
std::string getHostText(const std::string& host, uint16_t port)
{
  std::string hosttext = host;
  if(!(port == 80 || port == 443)) {
    hosttext += fmt(":%u", port);;
  }
  return hosttext;
}
} // namespace

std::string HttpRequest::createRequest()
{
  authConfig_ = authConfigFactory_->createAuthConfig(request_, option_);
  std::string requestLine = request_->getMethod();
  requestLine += " ";
  if(proxyRequest_) {
    if(getProtocol() == Request::PROTO_FTP &&
       request_->getUsername().empty() && authConfig_) {
      // Insert user into URI, like ftp://USER@host/
      std::string uri = getCurrentURI();
      assert(uri.size() >= 6);
      uri.insert(6, util::percentEncode(authConfig_->getUser())+"@");
      requestLine += uri;
    } else {
      requestLine += getCurrentURI();
    }
  } else {
    requestLine += getDir();
    requestLine += getFile();
    requestLine += getQuery();
  }
  requestLine += " HTTP/1.1\r\n";

  std::vector<std::pair<std::string, std::string> > builtinHds;
  builtinHds.reserve(20);
  builtinHds.push_back(std::make_pair("User-Agent:", userAgent_));
  std::string acceptTypes = "*/*";
  for(std::vector<std::string>::const_iterator i = acceptTypes_.begin(),
        eoi = acceptTypes_.end(); i != eoi; ++i) {
    acceptTypes += ",";
    acceptTypes += *i;
  }
  builtinHds.push_back(std::make_pair("Accept:", acceptTypes));
  if(contentEncodingEnabled_) {
    std::string acceptableEncodings;
#ifdef HAVE_ZLIB
    if(acceptGzip_) {
      acceptableEncodings += "deflate, gzip";
    }
#endif // HAVE_ZLIB
    if(!acceptableEncodings.empty()) {
      builtinHds.push_back
        (std::make_pair("Accept-Encoding:", acceptableEncodings));
    }
  }
  builtinHds.push_back
    (std::make_pair("Host:", getHostText(getURIHost(), getPort())));
  if(noCache_) {
    builtinHds.push_back(std::make_pair("Pragma:", "no-cache"));
    builtinHds.push_back(std::make_pair("Cache-Control:", "no-cache"));
  }
  if(!request_->isKeepAliveEnabled() && !request_->isPipeliningEnabled()) {
    builtinHds.push_back(std::make_pair("Connection:", "close"));
  }
  if(segment_ && segment_->getLength() > 0 && 
     (request_->isPipeliningEnabled() || getStartByte() > 0)) {
    std::string rangeHeader(fmt("bytes=%" PRId64 "-", getStartByte()));
    if(request_->isPipeliningEnabled()) {
      rangeHeader += util::itos(getEndByte());
    } else if(getProtocol() != Request::PROTO_FTP && endOffsetOverride_ > 0) {
      // FTP via http proxy does not support endbytes 
      rangeHeader += util::itos(endOffsetOverride_-1);
    }
    builtinHds.push_back(std::make_pair("Range:", rangeHeader));
  }
  if(proxyRequest_) {
    if(request_->isKeepAliveEnabled() || request_->isPipeliningEnabled()) {
      builtinHds.push_back(std::make_pair("Proxy-Connection:", "Keep-Alive"));
    } else {
      builtinHds.push_back(std::make_pair("Proxy-Connection:", "close"));
    }
  }
  if(proxyRequest_ && !proxyRequest_->getUsername().empty()) {
    builtinHds.push_back(getProxyAuthString());
  }
  if(authConfig_) {
    std::string authText = authConfig_->getAuthText();
    std::string val = "Basic ";
    val += base64::encode(authText.begin(), authText.end());
    builtinHds.push_back(std::make_pair("Authorization:", val));
  }
  if(!getPreviousURI().empty()) {
    builtinHds.push_back(std::make_pair("Referer:", getPreviousURI()));
  }
  if(cookieStorage_) {
    std::string cookiesValue;
    std::string path = getDir();
    path += getFile();
    std::vector<Cookie> cookies =
      cookieStorage_->criteriaFind(getHost(), path,
                                   Time().getTime(),
                                   getProtocol() == Request::PROTO_HTTPS);
    for(std::vector<Cookie>::const_iterator itr = cookies.begin(),
          eoi = cookies.end(); itr != eoi; ++itr) {
      cookiesValue += (*itr).toString();
      cookiesValue += ";";
    }
    if(!cookiesValue.empty()) {
      builtinHds.push_back(std::make_pair("Cookie:", cookiesValue));
    }
  }
  if(!ifModSinceHeader_.empty()) {
    builtinHds.push_back
      (std::make_pair("If-Modified-Since:", ifModSinceHeader_));
  }
  for(std::vector<std::pair<std::string, std::string> >::const_iterator i =
        builtinHds.begin(), eoi = builtinHds.end(); i != eoi; ++i) {
    std::vector<std::string>::const_iterator j = headers_.begin();
    std::vector<std::string>::const_iterator jend = headers_.end();
    for(; j != jend; ++j) {
      if(util::startsWith(*j, (*i).first)) {
        break;
      }
    }
    if(j == jend) {
      requestLine += (*i).first;
      requestLine += " ";
      requestLine += (*i).second;
      requestLine += "\r\n";
    }
  }
  // append additional headers given by user.
  for(std::vector<std::string>::const_iterator i = headers_.begin(),
        eoi = headers_.end(); i != eoi; ++i) {
    requestLine += *i;
    requestLine += "\r\n";
  }
  requestLine += "\r\n";
  return requestLine;
}

std::string HttpRequest::createProxyRequest() const
{
  assert(proxyRequest_);
  //std::string hostport(fmt("%s:%u", getURIHost().c_str(), getPort()));
  std::string requestLine(fmt("CONNECT %s:%u HTTP/1.1\r\n"
                              "User-Agent: %s\r\n"
                              "Host: %s:%u\r\n",
                              getURIHost().c_str(),
                              getPort(),
                              userAgent_.c_str(),
                              getURIHost().c_str(),
                              getPort()));
  // TODO Is "Proxy-Connection" needed here?
  //   if(request->isKeepAliveEnabled() || request->isPipeliningEnabled()) {
  //     requestLine += "Proxy-Connection: Keep-Alive\r\n";
  //   }else {
  //     requestLine += "Proxy-Connection: close\r\n";
  //   }
  if(!proxyRequest_->getUsername().empty()) {
    std::pair<std::string, std::string> auth = getProxyAuthString();
    requestLine += auth.first;
    requestLine += " ";
    requestLine += auth.second;
    requestLine += "\r\n";
  }
  requestLine += "\r\n";
  return requestLine;
}

std::pair<std::string, std::string> HttpRequest::getProxyAuthString() const
{
  std::string authText = proxyRequest_->getUsername();
  authText += ":";
  authText += proxyRequest_->getPassword();
  std::string val = "Basic ";
  val += base64::encode(authText.begin(), authText.end());
  return std::make_pair("Proxy-Authorization:", val);
}

void HttpRequest::enableContentEncoding()
{
  contentEncodingEnabled_ = true;
}

void HttpRequest::disableContentEncoding()
{
  contentEncodingEnabled_ = false;
}

void HttpRequest::addHeader(const std::string& headersString)
{
  util::split(headersString.begin(), headersString.end(),
              std::back_inserter(headers_), '\n', true);
}

void HttpRequest::clearHeader()
{
  headers_.clear();
}

void HttpRequest::addAcceptType(const std::string& type)
{
  acceptTypes_.push_back(type);
}

void HttpRequest::setCookieStorage
(const SharedHandle<CookieStorage>& cookieStorage)
{
  cookieStorage_ = cookieStorage;
}

void HttpRequest::setAuthConfigFactory
(const SharedHandle<AuthConfigFactory>& factory, const Option* option)
{
  authConfigFactory_ = factory;
  option_ = option;
}

void HttpRequest::setProxyRequest(const SharedHandle<Request>& proxyRequest)
{
  proxyRequest_ = proxyRequest;
}

bool HttpRequest::isProxyRequestSet() const
{
  return proxyRequest_;
}

bool HttpRequest::authenticationUsed() const
{
  return authConfig_;
}

const SharedHandle<AuthConfig>& HttpRequest::getAuthConfig() const
{
  return authConfig_;
}

int64_t HttpRequest::getEntityLength() const
{
  assert(fileEntry_);
  return fileEntry_->getLength();
}

const std::string& HttpRequest::getHost() const
{
  return request_->getHost();
}

uint16_t HttpRequest::getPort() const
{
  return request_->getPort();
}

const std::string& HttpRequest::getMethod() const
{
  return request_->getMethod();
}

const std::string& HttpRequest::getProtocol() const
{
  return request_->getProtocol();
}

const std::string& HttpRequest::getCurrentURI() const
{
  return request_->getCurrentUri();
}
  
const std::string& HttpRequest::getDir() const
{
  return request_->getDir();
}

const std::string& HttpRequest::getFile() const
{
  return request_->getFile();
}

const std::string& HttpRequest::getQuery() const
{
  return request_->getQuery();
}

const std::string& HttpRequest::getPreviousURI() const
{
  return request_->getPreviousUri();
}

std::string HttpRequest::getURIHost() const
{
  return request_->getURIHost();
}

void HttpRequest::setUserAgent(const std::string& userAgent)
{
  userAgent_ = userAgent;
}

void HttpRequest::setFileEntry(const SharedHandle<FileEntry>& fileEntry)
{
  fileEntry_ = fileEntry;
}

void HttpRequest::setIfModifiedSinceHeader(const std::string& hd)
{
  ifModSinceHeader_ = hd;
}

bool HttpRequest::conditionalRequest() const
{
  if(!ifModSinceHeader_.empty()) {
    return true;
  }
  for(std::vector<std::string>::const_iterator i = headers_.begin(),
        eoi = headers_.end(); i != eoi; ++i) {
    if(util::istartsWith(*i, "if-modified-since") ||
       util::istartsWith(*i, "if-none-match")) {
      return true;
    }
  }
  return false;
}

} // namespace aria2
