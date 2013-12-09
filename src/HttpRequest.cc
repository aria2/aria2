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
#include "DownloadHandlerConstants.h"

namespace aria2 {

const std::string HttpRequest::USER_AGENT("aria2");

HttpRequest::HttpRequest()
  : cookieStorage_(nullptr),
    authConfigFactory_(nullptr),
    option_(nullptr),
    endOffsetOverride_(0),
    userAgent_(USER_AGENT),
    contentEncodingEnabled_(true),
    acceptMetalink_(false),
    noCache_(true),
    acceptGzip_(false)
{}

HttpRequest::~HttpRequest() {}

void HttpRequest::setSegment(const std::shared_ptr<Segment>& segment)
{
  segment_ = segment;
}

void HttpRequest::setRequest(const std::shared_ptr<Request>& request)
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
  }
  if(request_->isPipeliningEnabled()) {
    int64_t endByte =
      fileEntry_->gtoloff(segment_->getPosition()+segment_->getLength()-1);
    return std::min(endByte, fileEntry_->getLength()-1);
  }
  if(endOffsetOverride_ > 0) {
    return endOffsetOverride_ - 1;
  }
  return 0;
}

Range HttpRequest::getRange() const
{
  // content-length is always 0
  if(!segment_) {
    return Range();
  }
  return Range(getStartByte(), getEndByte(), fileEntry_->getLength());
}

bool HttpRequest::isRangeSatisfied(const Range& range) const
{
  if(!segment_) {
    return true;
  }
  if((getStartByte() == range.startByte) &&
     ((getEndByte() == 0) ||
      (getEndByte() == range.endByte)) &&
     ((fileEntry_->getLength() == 0) ||
      (fileEntry_->getLength() == range.entityLength))) {
    return true;
  }
  return false;
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
    if(getProtocol() == "ftp" &&
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
  if(acceptMetalink_) {
    // The mime types of Metalink are used for "transparent metalink".
    const char** metalinkTypes = getMetalinkContentTypes();
    for(size_t i = 0; metalinkTypes[i]; ++i) {
      acceptTypes += ",";
      acceptTypes += metalinkTypes[i];
    }
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
     (request_->isPipeliningEnabled() || getStartByte() > 0 ||
      getEndByte() > 0)) {
    std::string rangeHeader(fmt("bytes=%" PRId64 "-", getStartByte()));
    if(request_->isPipeliningEnabled() || getEndByte() > 0) {
      // FTP via http proxy does not support endbytes, but in that
      // case, request_->isPipeliningEnabled() is false and
      // getEndByte() is 0.
      rangeHeader += util::itos(getEndByte());
    }
    builtinHds.push_back(std::make_pair("Range:", rangeHeader));
  }
  if(proxyRequest_) {
    if(request_->isKeepAliveEnabled() || request_->isPipeliningEnabled()) {
      builtinHds.push_back(std::make_pair("Connection:", "Keep-Alive"));
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
  if(!request_->getReferer().empty()) {
    builtinHds.push_back(std::make_pair("Referer:", request_->getReferer()));
  }
  if(cookieStorage_) {
    std::string cookiesValue;
    std::string path = getDir();
    path += getFile();
    auto cookies = cookieStorage_->criteriaFind(getHost(), path,
                                                Time().getTime(),
                                                getProtocol() == "https");
    for(auto c : cookies) {
      cookiesValue += c->toString();
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
  std::string requestLine(fmt("CONNECT %s:%u HTTP/1.1\r\n"
                              "User-Agent: %s\r\n"
                              "Host: %s:%u\r\n",
                              getURIHost().c_str(),
                              getPort(),
                              userAgent_.c_str(),
                              getURIHost().c_str(),
                              getPort()));
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

void HttpRequest::setCookieStorage(CookieStorage* cookieStorage)
{
  cookieStorage_ = cookieStorage;
}

CookieStorage* HttpRequest::getCookieStorage() const
{
  return cookieStorage_;
}

void HttpRequest::setAuthConfigFactory(AuthConfigFactory* factory)
{
  authConfigFactory_ = factory;
}

void HttpRequest::setOption(const Option* option)
{
  option_ = option;
}

void HttpRequest::setProxyRequest(const std::shared_ptr<Request>& proxyRequest)
{
  proxyRequest_ = proxyRequest;
}

bool HttpRequest::isProxyRequestSet() const
{
  return proxyRequest_.get();
}

bool HttpRequest::authenticationUsed() const
{
  return authConfig_.get();
}

const std::unique_ptr<AuthConfig>& HttpRequest::getAuthConfig() const
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

std::string HttpRequest::getURIHost() const
{
  return request_->getURIHost();
}

void HttpRequest::setUserAgent(const std::string& userAgent)
{
  userAgent_ = userAgent;
}

void HttpRequest::setFileEntry(const std::shared_ptr<FileEntry>& fileEntry)
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
  for(auto& h : headers_) {
    if(util::istartsWith(h, "if-modified-since") ||
       util::istartsWith(h, "if-none-match")) {
      return true;
    }
  }
  return false;
}

} // namespace aria2
