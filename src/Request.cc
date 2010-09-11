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
#include "Request.h"

#include <cassert>
#include <utility>

#include "util.h"
#include "FeatureConfig.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "a2functional.h"
#include "uri.h"

namespace aria2 {

const std::string Request::METHOD_GET = "GET";

const std::string Request::METHOD_HEAD = "HEAD";

const std::string Request::PROTO_HTTP("http");

const std::string Request::PROTO_HTTPS("https");

const std::string Request::PROTO_FTP("ftp");

Request::Request():
  port_(0), tryCount_(0),
  redirectCount_(0),
  supportsPersistentConnection_(true),
  keepAliveHint_(false),
  pipeliningHint_(false),
  maxPipelinedRequest_(1),
  method_(METHOD_GET),
  hasPassword_(false),
  ipv6LiteralAddress_(false),
  removalRequested_(false),
  connectedPort_(0)
{}

static std::string removeFragment(const std::string& uri)
{
  std::string::size_type sharpIndex = uri.find("#");
  if(sharpIndex == std::string::npos) {
    return uri;
  } else {
    return uri.substr(0, sharpIndex);
  }
}

static bool isHexNumber(const char c)
{
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') ||
    ('a' <= c && c <= 'f');
}

static std::string percentEncode(const std::string& src)
{
  std::string result = src;
  if(src.empty()) {
    return result;
  }
  result += "  ";
  for(int index = src.size()-1; index >= 0; --index) {
    const unsigned char c = result[index];
    // '/' is not percent encoded because src is expected to be a path.
    if(!util::inRFC3986ReservedChars(c) && !util::inRFC3986UnreservedChars(c)) {
      if(c == '%') {
        if(!isHexNumber(result[index+1]) || !isHexNumber(result[index+2])) {
          result.replace(index, 1, "%25");
        }
      } else {
        result.replace(index, 1, StringFormat("%%%02X", c).str());
      }
    }
  }
  result.erase(result.size()-2);
  return result;
}

bool Request::setUri(const std::string& uri) {
  supportsPersistentConnection_ = true;
  uri_ = uri;
  return parseUri(uri_);
}

bool Request::resetUri() {
  previousUri_ = referer_;
  supportsPersistentConnection_ = true;
  setConnectedAddrInfo(A2STR::NIL, A2STR::NIL, 0);
  return parseUri(uri_);
}

void Request::setReferer(const std::string& uri)
{
  referer_ = previousUri_ = percentEncode(removeFragment(uri));
}

bool Request::redirectUri(const std::string& uri) {
  supportsPersistentConnection_ = true;
  ++redirectCount_;
  std::string redirectedUri;
  if(uri.find("://") == std::string::npos) {
    // rfc2616 requires absolute URI should be provided by Location header
    // field, but some servers don't obey this rule.
    if(util::startsWith(uri, "/")) {
      // abosulute path
      redirectedUri = strconcat(protocol_, "://", host_, uri);
    } else {
      // relative path
      redirectedUri = strconcat(protocol_, "://", host_, dir_, "/", uri);
    }
  } else {
    redirectedUri = uri;
  }
  return parseUri(redirectedUri);
}

bool Request::parseUri(const std::string& srcUri) {
  currentUri_ = percentEncode(removeFragment(srcUri));
  uri::UriStruct us;
  if(uri::parse(us, currentUri_)) {
    protocol_.swap(us.protocol);
    host_.swap(us.host);
    port_ = us.port;
    dir_.swap(us.dir);
    file_.swap(us.file);
    query_.swap(us.query);
    username_.swap(us.username);
    password_.swap(us.password);
    hasPassword_ = us.hasPassword;
    ipv6LiteralAddress_ = us.ipv6LiteralAddress;
    return true;
  } else {
    return false;
  }
}

void Request::resetRedirectCount()
{
  redirectCount_ = 0;
}
  
void Request::setMaxPipelinedRequest(unsigned int num)
{
  maxPipelinedRequest_ = num;
}

const SharedHandle<PeerStat>& Request::initPeerStat()
{
  // Use host and protocol in original URI, because URI selector
  // selects URI based on original URI, not redirected one.
  uri::UriStruct us;
  bool v = uri::parse(us, uri_);
  assert(v);
  peerStat_.reset(new PeerStat(0, us.host, us.protocol));
  return peerStat_;
}

} // namespace aria2
