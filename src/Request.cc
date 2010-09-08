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

#include <utility>

#include "util.h"
#include "FeatureConfig.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "a2functional.h"

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
  removalRequested_(false)
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
  const std::string uri = percentEncode(removeFragment(srcUri));
  currentUri_ = uri;
  host_ = A2STR::NIL;
  port_ = 0;
  dir_ = A2STR::NIL;
  file_ = A2STR::NIL;
  query_ = A2STR::NIL;
  username_ = A2STR::NIL;
  password_ = A2STR::NIL;
  hasPassword_ = false;
  ipv6LiteralAddress_ = false;

  // http://user:password@aria2.sourceforge.net:80/dir/file?query
  //        |            ||                    || |   |    |
  //        |            ||             hostLast| |   |    |
  //        |            ||              portFirst|   |    |
  //    authorityFirst   ||             authorityLast |    |
  //                     ||                       |   |    |
  //                   userInfoLast               |   |    |
  //                      |                       |   |    |
  //                     hostPortFirst            |   |    |
  //                                              |   |    |
  //                                       dirFirst dirLast|
  //                                                       |
  //                                                queryFirst

  // find query part
  std::string::const_iterator queryFirst = uri.begin();
  for(; queryFirst != uri.end(); ++queryFirst) {
    if(*queryFirst == '?') break;
  }
  query_ = std::string(queryFirst, uri.end());
  // find protocol
  std::string::size_type protocolOffset = uri.find("://");
  if(protocolOffset == std::string::npos) return false;
  protocol_ = std::string(uri.begin(), uri.begin()+protocolOffset);
  uint16_t defPort;
  if((defPort = FeatureConfig::getInstance()->getDefaultPort(protocol_)) == 0) {
    return false;
  }
  // find authority
  std::string::const_iterator authorityFirst = uri.begin()+protocolOffset+3;
  std::string::const_iterator authorityLast = authorityFirst;
  for(; authorityLast != queryFirst; ++authorityLast) {
    if(*authorityLast == '/') break;
  }
  if(authorityFirst == authorityLast) {
    // No authority found
    return false;
  }
  // find userinfo(username and password) in authority if they exist
  std::string::const_iterator userInfoLast = authorityLast;
  std::string::const_iterator hostPortFirst = authorityFirst;
  for(; userInfoLast != authorityFirst-1; --userInfoLast) {
    if(*userInfoLast == '@') {
      hostPortFirst = userInfoLast;
      ++hostPortFirst;
      std::string::const_iterator userLast = authorityFirst;
      for(; userLast != userInfoLast; ++userLast) {
        if(*userLast == ':') {
          password_ = util::percentDecode(std::string(userLast+1,userInfoLast));
          hasPassword_ = true;
          break;
        }
      }
      username_ = util::percentDecode(std::string(authorityFirst, userLast));
      break;
    }
  }
  std::string::const_iterator hostLast = hostPortFirst;
  std::string::const_iterator portFirst = authorityLast;
  if(*hostPortFirst == '[') {
    // Detected IPv6 literal address in square brackets
    for(; hostLast != authorityLast; ++hostLast) {
      if(*hostLast == ']') {
        ++hostLast;
        if(hostLast == authorityLast) {
          ipv6LiteralAddress_ = true;
        } else {
          if(*hostLast == ':') {
            portFirst = hostLast;
            ++portFirst;
            ipv6LiteralAddress_ = true;
          }
        }
        break;
      }
    }
    if(!ipv6LiteralAddress_) {
      return false;
    }
  } else {
    for(; hostLast != authorityLast; ++hostLast) {
      if(*hostLast == ':') {
        portFirst = hostLast;
        ++portFirst;
        break;
      }
    }
  }
  if(hostPortFirst == hostLast) {
    // No host
    return false;
  }
  if(portFirst == authorityLast) {
    // If port is not specified, then we set it to default port of
    // its protocol..
    port_ = defPort;
  } else {
    uint32_t tempPort;
    if(util::parseUIntNoThrow(tempPort, std::string(portFirst, authorityLast))){
      if(65535 < tempPort) {
        return false;
      }
      port_ = tempPort;      
    } else {
      return false;
    }
  }
  if(ipv6LiteralAddress_) {
    host_ = std::string(hostPortFirst+1, hostLast-1);
  } else {
    host_ = std::string(hostPortFirst, hostLast);
  }
  // find directory and file part
  std::string::const_iterator dirLast = authorityLast;
  for(std::string::const_iterator i = authorityLast;
      i != queryFirst; ++i) {
    if(*i == '/') {
      dirLast = i;
    }
  }
  if(dirLast != queryFirst) {
    file_ = std::string(dirLast+1, queryFirst);
  }
  // Erase duplicated slashes.
  std::string::const_iterator dirFirst = authorityLast;
  for(; dirFirst != dirLast; ++dirFirst) {
    if(*dirFirst != '/') {
      --dirFirst;
      break;
    }
  }
  for(; dirLast != dirFirst; --dirLast) {
    if(*dirLast != '/') {
      ++dirLast;
      break;
    }
  }
  if(dirFirst == dirLast) {
    dir_ = A2STR::SLASH_C;
  } else {
    dir_ = std::string(dirFirst, dirLast);
  }
  return true;
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
  Request origReq;
  origReq.setUri(uri_);
  peerStat_.reset(new PeerStat(0, origReq.getHost(), origReq.getProtocol()));
  return peerStat_;
}

} // namespace aria2
