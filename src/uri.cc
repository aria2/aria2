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
  uri_split_result res;
  int rv;
  const char* p = uri.c_str();
  rv = uri_split(&res, p);
  if(rv == 0) {
    result.protocol.assign(p + res.fields[USR_SCHEME].off,
                           res.fields[USR_SCHEME].len);
    result.host.assign(p + res.fields[USR_HOST].off, res.fields[USR_HOST].len);
    if(res.port == 0) {
      uint16_t defPort;
      if((defPort = getDefaultPort(result.protocol)) == 0) {
        return false;
      }
      result.port = defPort;
    } else {
      result.port = res.port;
    }
    if(res.field_set & (1 << USR_PATH)) {
      if(res.field_set & (1 << USR_BASENAME)) {
        result.dir.assign(p + res.fields[USR_PATH].off,
                          res.fields[USR_PATH].len -
                          res.fields[USR_BASENAME].len);
        result.file.assign(p + res.fields[USR_BASENAME].off,
                           res.fields[USR_BASENAME].len);
      } else {
        result.dir.assign(p + res.fields[USR_PATH].off,
                          res.fields[USR_PATH].len);
        result.file = A2STR::NIL;
      }
    } else {
      result.dir = "/";
      result.file = A2STR::NIL;
    }
    if(res.field_set & (1 << USR_QUERY)) {
      result.query = "?";
      result.query.append(p + res.fields[USR_QUERY].off,
                          res.fields[USR_QUERY].len);
    } else {
      result.query = A2STR::NIL;
    }
    if(res.field_set & (1 << USR_USER)) {
      result.username.assign(p + res.fields[USR_USER].off,
                             res.fields[USR_USER].len);
      result.username = util::percentDecode(result.username.begin(),
                                            result.username.end());
    } else {
      result.username = A2STR::NIL;
    }
    if(res.field_set & (1 << USR_PASSWD)) {
      result.hasPassword = true;
      result.password.assign(p + res.fields[USR_PASSWD].off,
                             res.fields[USR_PASSWD].len);
      result.password = util::percentDecode(result.password.begin(),
                                            result.password.end());
    } else {
      result.hasPassword = false;
      result.password = A2STR::NIL;
    }
    result.ipv6LiteralAddress = res.flags & USF_IPV6ADDR;
    return true;
  } else {
    return false;
  }
}

std::string getFieldString(const uri_split_result& res, int field,
                           const char* base)
{
  if(res.field_set & (1 << field)) {
    return std::string(base + res.fields[field].off, res.fields[field].len);
  } else {
    return "";
  }
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
  uint16_t defPort= getDefaultPort(us.protocol);
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
