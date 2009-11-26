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
  _port(0), _tryCount(0),
  _redirectCount(0),
  _supportsPersistentConnection(true),
  _keepAliveHint(false),
  _pipeliningHint(false),
  _maxPipelinedRequest(1),
  _method(METHOD_GET),
  _hasPassword(false),
  _ipv6LiteralAddress(false)
{}

static std::string removeFragment(const std::string& url)
{
  std::string::size_type sharpIndex = url.find("#");
  if(sharpIndex == std::string::npos) {
    return url;
  } else {
    return url.substr(0, sharpIndex);
  }
}

static bool isHexNumber(const char c)
{
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') ||
    ('a' <= c && c <= 'f');
}

static std::string urlencode(const std::string& src)
{
  std::string result = src;
  if(src.empty()) {
    return result;
  }
  result += "  ";
  for(int index = src.size()-1; index >= 0; --index) {
    const unsigned char c = result[index];
    // '/' is not urlencoded because src is expected to be a path.
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

bool Request::setUrl(const std::string& url) {
  _supportsPersistentConnection = true;
  _url = url;
  return parseUrl(urlencode(removeFragment(_url)));
}

bool Request::resetUrl() {
  _previousUrl = _referer;
  _supportsPersistentConnection = true;
  return parseUrl(urlencode(removeFragment(_url)));
}

void Request::setReferer(const std::string& url)
{
  _referer = _previousUrl = urlencode(removeFragment(url));
}

bool Request::redirectUrl(const std::string& url) {
  _previousUrl = A2STR::NIL;
  _supportsPersistentConnection = true;
  ++_redirectCount;
  if(url.find("://") == std::string::npos) {
    // rfc2616 requires absolute URI should be provided by Location header
    // field, but some servers don't obey this rule.
    if(util::startsWith(url, "/")) {
      // abosulute path
      return parseUrl(strconcat(_protocol, "://", _host, url));
    } else {
      // relative path
      return parseUrl(strconcat(_protocol, "://", _host, _dir, "/", url));
    }
  } else {
    return parseUrl(url);
  }
}

bool Request::parseUrl(const std::string& url) {
  _currentUrl = url;
  _host = A2STR::NIL;
  _port = 0;
  _dir = A2STR::NIL;
  _file = A2STR::NIL;
  _query = A2STR::NIL;
  _username = A2STR::NIL;
  _password = A2STR::NIL;
  _hasPassword = false;
  _ipv6LiteralAddress = false;

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
  std::string::const_iterator queryFirst = url.begin();
  for(; queryFirst != url.end(); ++queryFirst) {
    if(*queryFirst == '?') break;
  }
  _query = std::string(queryFirst, url.end());
  // find protocol
  std::string::size_type protocolOffset = url.find("://");
  if(protocolOffset == std::string::npos) return false;
  _protocol = std::string(url.begin(), url.begin()+protocolOffset);
  uint16_t defPort;
  if((defPort = FeatureConfig::getInstance()->getDefaultPort(_protocol)) == 0) {
    return false;
  }
  // find authority
  std::string::const_iterator authorityFirst = url.begin()+protocolOffset+3;
  std::string::const_iterator authorityLast = authorityFirst;
  for(; authorityLast != queryFirst; ++authorityLast) {
    if(*authorityLast == '/') break;
  }
  if(authorityFirst == authorityLast) {
    // No authority found
    return false;
  }
  // find userinfo(username and password) in authority if they exist
  std::string::const_iterator userInfoLast = authorityFirst;
  std::string::const_iterator hostPortFirst = authorityFirst;
  for(; userInfoLast != authorityLast; ++userInfoLast) {
    if(*userInfoLast == '@') {
      hostPortFirst = userInfoLast;
      ++hostPortFirst;
      std::string::const_iterator userLast = authorityFirst;
      for(; userLast != userInfoLast; ++userLast) {
	if(*userLast == ':') {
	  _password = util::urldecode(std::string(userLast+1, userInfoLast));
	  _hasPassword = true;
	  break;
	}
      }
      _username = util::urldecode(std::string(authorityFirst, userLast));
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
	  _ipv6LiteralAddress = true;
	} else {
	  if(*hostLast == ':') {
	    portFirst = hostLast;
	    ++portFirst;
	    _ipv6LiteralAddress = true;
	  }
	}
	break;
      }
    }
    if(!_ipv6LiteralAddress) {
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
    _port = defPort;
  } else {
    uint32_t tempPort;
    if(util::parseUIntNoThrow(tempPort, std::string(portFirst, authorityLast))){
      if(65535 < tempPort) {
	return false;
      }
      _port = tempPort;      
    } else {
      return false;
    }
  }
  if(_ipv6LiteralAddress) {
    _host = std::string(hostPortFirst+1, hostLast-1);
  } else {
    _host = std::string(hostPortFirst, hostLast);
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
    _file = std::string(dirLast+1, queryFirst);
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
    _dir = A2STR::SLASH_C;
  } else {
    _dir = std::string(dirFirst, dirLast);
  }
  return true;
}

void Request::resetRedirectCount()
{
  _redirectCount = 0;
}
  
void Request::setMaxPipelinedRequest(unsigned int num)
{
  _maxPipelinedRequest = num;
}

const SharedHandle<PeerStat>& Request::initPeerStat()
{
  // Use host and protocol in original URI, because URI selector
  // selects URI based on original URI, not redirected one.
  Request origReq;
  origReq.setUrl(_url);
  _peerStat.reset(new PeerStat(0, origReq.getHost(), origReq.getProtocol()));
  return _peerStat;
}

} // namespace aria2
