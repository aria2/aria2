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
#include "RecoverableException.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "a2functional.h"

#define SAFE_CHARS "abcdefghijklmnopqrstuvwxyz"\
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
"0123456789"\
":/?[]@"\
"!$&'()*+,;="\
"-._~"\
"%"\
"#"

namespace aria2 {

const std::string Request::METHOD_GET = "GET";

const std::string Request::METHOD_HEAD = "HEAD";

const std::string Request::PROTO_HTTP("http");

const std::string Request::PROTO_HTTPS("https");

const std::string Request::PROTO_FTP("ftp");

Request::Request():
  port(0), tryCount(0),
  _redirectCount(0),
  _supportsPersistentConnection(true),
  _keepAliveHint(false),
  _pipeliningHint(false),
  _maxPipelinedRequest(1),
  method(METHOD_GET),
  _hasPassword(false),
  _ipv6LiteralAddress(false)
{}

static std::string removeFragment(const std::string url)
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
  this->url = url;
  return parseUrl(urlencode(removeFragment(url)));
}

bool Request::resetUrl() {
  previousUrl = referer;
  _supportsPersistentConnection = true;
  return parseUrl(urlencode(removeFragment(url)));
}

void Request::setReferer(const std::string& url)
{
  referer = previousUrl = urlencode(removeFragment(url));
}

bool Request::redirectUrl(const std::string& url) {
  previousUrl = A2STR::NIL;
  _supportsPersistentConnection = true;
  ++_redirectCount;
  if(url.find("://") == std::string::npos) {
    // rfc2616 requires absolute URI should be provided by Location header
    // field, but some servers don't obey this rule.
    if(util::startsWith(url, "/")) {
      // abosulute path
      return parseUrl(strconcat(protocol, "://", host, url));
    } else {
      // relative path
      return parseUrl(strconcat(protocol, "://", host, dir, "/", url));
    }
  } else {
    return parseUrl(url);
  }
}

bool Request::parseUrl(const std::string& url) {
  currentUrl = url;
  std::string tempUrl = url;
  host = A2STR::NIL;
  port = 0;
  dir = A2STR::NIL;
  file = A2STR::NIL;
  _query = A2STR::NIL;
  _username = A2STR::NIL;
  _password = A2STR::NIL;
  _hasPassword = false;
  _ipv6LiteralAddress = false;
  // find query part
  std::string queryTemp;
  std::string::size_type startQueryIndex = tempUrl.find("?");
  if(startQueryIndex != std::string::npos) {
    queryTemp = tempUrl.substr(startQueryIndex);
    tempUrl.erase(startQueryIndex);
  }
  // find protocol
  std::string::size_type hp = tempUrl.find("://");
  if(hp == std::string::npos) return false;
  protocol = tempUrl.substr(0, hp);
  uint16_t defPort;
  if((defPort = FeatureConfig::getInstance()->getDefaultPort(protocol)) == 0) {
    return false;
  }
  hp += 3;
  // find host part
  if(tempUrl.size() <= hp) return false;
  std::string::size_type hep = tempUrl.find("/", hp);
  if(hep == std::string::npos) {
    hep = tempUrl.size();
  }
  std::string hostPart = tempUrl.substr(hp, hep-hp);
  //   find username and password in host part if they exist
  std::string::size_type atmarkp =  hostPart.find_last_of("@");
  if(atmarkp != std::string::npos) {
    std::string authPart = hostPart.substr(0, atmarkp);
    std::pair<std::string, std::string> userPass =
      util::split(authPart, A2STR::COLON_C);
    _username = util::urldecode(userPass.first);
    _password = util::urldecode(userPass.second);
    if(authPart.find(A2STR::COLON_C) != std::string::npos) {
      _hasPassword = true;
    }
    hostPart.erase(0, atmarkp+1);
  }
  {
    std::string::size_type colonpos;
    // Detect IPv6 literal address in square brackets
    if(util::startsWith(hostPart, "[")) {
      std::string::size_type rbracketpos = hostPart.find("]");
      if(rbracketpos == std::string::npos) {
	return false;
      }
      _ipv6LiteralAddress = true;
      colonpos = hostPart.find(":", rbracketpos+1);
    } else {
      colonpos = hostPart.find_last_of(":");
    }
    if(colonpos == std::string::npos) {
      colonpos = hostPart.size();
      // If port is not specified, then we set it to default port of
      // its protocol..
      port = defPort;
    } else {
      try {
	unsigned int tempPort = util::parseUInt(hostPart.substr(colonpos+1));
	if(65535 < tempPort) {
	  return false;
	}
	port = tempPort;
      } catch(RecoverableException& e) {
	return false;
      }
    }
    if(_ipv6LiteralAddress) {
      host = hostPart.substr(1, colonpos-2);
    } else {
      host = hostPart.substr(0, colonpos);
    }
  }
  // find directory and file part
  std::string::size_type direp = tempUrl.find_last_of("/");
  if(direp == std::string::npos || direp <= hep) {
    dir = A2STR::SLASH_C;
    direp = hep;
  } else {
    std::string rawDir = tempUrl.substr(hep, direp-hep);
    std::string::size_type p = rawDir.find_first_not_of("/");
    if(p != std::string::npos) {
      rawDir.erase(0, p-1);
    }
    p = rawDir.find_last_not_of("/");
    if(p != std::string::npos) {
      rawDir.erase(p+1);
    }
    dir = rawDir;
  }
  if(tempUrl.size() > direp+1) {
    file = tempUrl.substr(direp+1);
  }
  _query = queryTemp;
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
  _peerStat.reset(new PeerStat(0, host, protocol));
  return _peerStat;
}

} // namespace aria2
