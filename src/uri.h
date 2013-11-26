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
#ifndef D_URI_H
#define D_URI_H

#include "common.h"

#include <string>

#include "uri_split.h"

namespace aria2 {

namespace uri {

struct UriStruct {
  std::string protocol;
  std::string host;
  std::string dir;
  std::string file;
  std::string query;
  std::string username;
  std::string password;
  uint16_t port;
  bool hasPassword;
  bool ipv6LiteralAddress;

  UriStruct();
  UriStruct(const UriStruct& c);
  ~UriStruct();

  UriStruct& operator=(const UriStruct& c);
  void swap(UriStruct& other);
};

void swap(UriStruct& lhs, UriStruct& rhs);

// Splits URI uri into components and stores them into result.  On
// success returns true. Otherwise returns false and result is
// undefined.
bool parse(UriStruct& result, const std::string& uri);

// Returns string specified by field in res. The base pointer in res
// is given as base. If the given field is not stored in res, returns
// empty string.
std::string getFieldString(const uri_split_result& res, int field,
                           const char* base);

std::string construct(const UriStruct& us);

std::string joinUri(const std::string& baseUri, const std::string& uri);

std::string joinPath(const std::string& basePath, const std::string& newPath);

// Normalizes path so that: 1) it does not contain successive / and 2)
// resolve path component '.' and '..'. If there is not enough path
// component to resolve '..', those '..' are discarded. The resulting
// path starts / only if path starts with /.
std::string normalizePath(std::string path);

} // namespace uri

} // namespace aria2

#endif // D_URI_H
