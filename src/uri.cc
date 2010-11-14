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

bool parse(UriStruct& result, const std::string& uri)
{
  // http://user:password@aria2.sourceforge.net:80/dir/file?query#fragment
  //        |            ||                    || |   |    |     |
  //        |            ||             hostLast| |   |    |     |
  //        |            ||              portFirst|   |    |     |
  //    authorityFirst   ||             authorityLast |    |     |
  //                     ||                       |   |    |     |
  //                   userInfoLast               |   |    |     |
  //                      |                       |   |    |     |
  //                     hostPortFirst            |   |    |     |
  //                                              |   |    |     |
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
  result.query = std::string(queryFirst, fragmentFirst);
  // find protocol
  std::string::size_type protocolOffset = uri.find("://");
  if(protocolOffset == std::string::npos) return false;
  result.protocol = std::string(uri.begin(), uri.begin()+protocolOffset);
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
            util::percentDecode(std::string(userLast+1,userInfoLast));
          result.hasPassword = true;
          break;
        }
      }
      result.username =
        util::percentDecode(std::string(authorityFirst, userLast));
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
    if(util::parseUIntNoThrow(tempPort, std::string(portFirst, authorityLast))){
      if(65535 < tempPort) {
        return false;
      }
      result.port = tempPort;      
    } else {
      return false;
    }
  }
  if(result.ipv6LiteralAddress) {
    result.host = std::string(hostPortFirst+1, hostLast-1);
  } else {
    result.host = std::string(hostPortFirst, hostLast);
  }
  // find directory and file part
  std::string::const_iterator dirLast = authorityLast;
  for(std::string::const_iterator i = authorityLast;
      i != queryFirst; ++i) {
    if(*i == '/') {
      dirLast = i;
    }
  }
  if(dirLast == queryFirst) {
    result.file = A2STR::NIL;
  } else {
    result.file = std::string(dirLast+1, queryFirst);
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
    result.dir = A2STR::SLASH_C;
  } else {
    result.dir = std::string(dirFirst, dirLast);
  }
  return true;
}

} // namespace uri

} // namespace aria2
