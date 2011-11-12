/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "uri.h"
#include "A2STR.h"
#include "FeatureConfig.h"
#include "util.h"

namespace aria2 {

namespace uri {

UriStruct::UriStruct()
  : port(0), hasPassword(false), ipv6LiteralAddress(false)
{}

UriStruct::UriStruct(const UriStruct& c)
  : protocol(c.protocol),
    host(c.host),
    port(c.port),
    dir(c.dir),
    file(c.file),
    query(c.query),
    username(c.username),
    password(c.password),
    hasPassword(c.hasPassword),
    ipv6LiteralAddress(c.ipv6LiteralAddress)
{}

UriStruct::~UriStruct() {}

UriStruct& UriStruct::operator=(const UriStruct& c)
{
  if(this != &c) {
    protocol = c.protocol;
    host = c.host;
    port = c.port;
    dir = c.dir;
    file = c.file;
    query = c.query;
    username = c.username;
    password = c.password;
    hasPassword = c.hasPassword;
    ipv6LiteralAddress = c.ipv6LiteralAddress;
  }
  return *this;
}

void UriStruct::swap(UriStruct& other)
{
  using std::swap;
  if(this != &other) {
    swap(protocol, other.protocol);
    swap(host, other.host);
    swap(port, other.port);
    swap(dir, other.dir);
    swap(file, other.file);
    swap(query, other.query);
    swap(username, other.username);
    swap(password, other.password);
    swap(hasPassword, other.hasPassword);
    swap(ipv6LiteralAddress, other.ipv6LiteralAddress);
  }
}

void swap(UriStruct& lhs, UriStruct& rhs)
{
  lhs.swap(rhs);
}

bool parse(UriStruct& result, const std::string& uri)
{
  // http://user:password@aria2.sourceforge.net:80/dir/file?query#fragment
  //        |            ||                    || |    |   |     |
  //        |            ||             hostLast| |    |   |     |
  //        |            ||              portFirst|    |   |     |
  //    authorityFirst   ||             authorityLast  |   |     |
  //                     ||                       |    |   |     |
  //                   userInfoLast               |    |   |     |
  //                      |                       |    |   |     |
  //                     hostPortFirst            |    |   |     |
  //                                              |    |   |     |
  //                                       dirFirst dirLast|     |
  //                                                       |     |
  //                                                queryFirst fragmentFirst

  // find fragment part
  std::string::const_iterator fragmentFirst = uri.begin();
  for(; fragmentFirst != uri.end(); ++fragmentFirst) {
    if(*fragmentFirst == '#') break;
  }
  // find query part
  std::string::const_iterator queryFirst = uri.begin();
  for(; queryFirst != fragmentFirst; ++queryFirst) {
    if(*queryFirst == '?') break;
  }
  result.query.assign(queryFirst, fragmentFirst);
  // find protocol
  std::string::size_type protocolOffset = uri.find("://");
  if(protocolOffset == std::string::npos) return false;
  result.protocol.assign(uri.begin(), uri.begin()+protocolOffset);
  uint16_t defPort;
  if((defPort = FeatureConfig::getInstance()->
      getDefaultPort(result.protocol)) == 0) {
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
  result.username = A2STR::NIL;
  result.password = A2STR::NIL;
  result.hasPassword = false;
  std::string::const_iterator userInfoLast = authorityLast;
  std::string::const_iterator hostPortFirst = authorityFirst;
  for(; userInfoLast != authorityFirst-1; --userInfoLast) {
    if(*userInfoLast == '@') {
      hostPortFirst = userInfoLast;
      ++hostPortFirst;
      std::string::const_iterator userLast = authorityFirst;
      for(; userLast != userInfoLast; ++userLast) {
        if(*userLast == ':') {
          result.password =
            util::percentDecode(userLast+1,userInfoLast);
          result.hasPassword = true;
          break;
        }
      }
      result.username =
        util::percentDecode(authorityFirst, userLast);
      break;
    }
  }
  std::string::const_iterator hostLast = hostPortFirst;
  std::string::const_iterator portFirst = authorityLast;
  result.ipv6LiteralAddress = false;
  if(*hostPortFirst == '[') {
    // Detected IPv6 literal address in square brackets
    for(; hostLast != authorityLast; ++hostLast) {
      if(*hostLast == ']') {
        ++hostLast;
        if(hostLast == authorityLast) {
          result.ipv6LiteralAddress = true;
        } else {
          if(*hostLast == ':') {
            portFirst = hostLast;
            ++portFirst;
            result.ipv6LiteralAddress = true;
          }
        }
        break;
      }
    }
    if(!result.ipv6LiteralAddress) {
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
    result.port = defPort;
  } else {
    uint32_t tempPort;
    if(util::parseUIntNoThrow(tempPort,
                              std::string(portFirst, authorityLast))) {
      if(65535 < tempPort) {
        return false;
      }
      result.port = tempPort;      
    } else {
      return false;
    }
  }
  if(result.ipv6LiteralAddress) {
    result.host.assign(hostPortFirst+1, hostLast-1);
  } else {
    result.host.assign(hostPortFirst, hostLast);
  }
  // find directory and file part
  std::string::const_iterator dirLast = authorityLast;
  for(std::string::const_iterator i = authorityLast;
      i != queryFirst; ++i) {
    if(*i == '/') {
      dirLast = i+1;
    }
  }
  if(dirLast == queryFirst) {
    result.file = A2STR::NIL;
  } else {
    result.file.assign(dirLast, queryFirst);
  }
  // dirFirst == authorityLast
  if(authorityLast == dirLast) {
    result.dir = "/";
  } else {
    result.dir.assign(authorityLast, dirLast);
  }
  return true;
}

std::string construct(const UriStruct& us)
{
  std::string res;
  res += us.protocol;
  res += "://";
  if(!us.username.empty()) {
    res += util::percentEncode(us.username);
    if(us.hasPassword) {
      res += ":";
      res += util::percentEncode(us.password);
    }
    res += "@";
  }
  if(us.ipv6LiteralAddress) {
    res += "[";
    res += us.host;
    res += "]";
  } else {
    res += us.host;
  }
  uint16_t defPort= FeatureConfig::getInstance()->
    getDefaultPort(us.protocol);
  if(us.port != 0 && defPort != us.port) {
    res += fmt(":%u", us.port);
  }
  res += us.dir;
  if(us.dir.empty() || us.dir[us.dir.size()-1] != '/') {
    res += "/";
  }
  res += us.file;
  res += us.query;
  return res;
}

std::string joinUri(const std::string& baseUri, const std::string& uri)
{
  UriStruct us;
  if(parse(us, uri)) {
    return uri;
  } else {
    UriStruct bus;
    if(!parse(bus, baseUri)) {
      return uri;
    }
    std::vector<std::string> parts;
    if(uri.empty() || uri[0] != '/') {
      util::split(bus.dir.begin(), bus.dir.end(), std::back_inserter(parts),
                  '/');
    }
    std::string::const_iterator qend;
    for(qend = uri.begin(); qend != uri.end(); ++qend) {
      if(*qend == '#') {
        break;
      }
    }
    std::string::const_iterator end;
    for(end = uri.begin(); end != qend; ++end) {
      if(*end == '?') {
        break;
      }
    }
    util::split(uri.begin(), end, std::back_inserter(parts), '/');
    bus.dir.clear();
    bus.file.clear();
    bus.query.clear();
    std::string res = construct(bus);
    res += util::joinPath(parts.begin(), parts.end());
    if((uri.begin() == end || *(end-1) == '/') && *(res.end()-1) != '/') {
      res += "/";
    }
    res.append(end, qend);
    return res;
  }
}

} // namespace uri

} // namespace aria2
