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

namespace aria2 {

namespace uri {

struct UriStruct {
  std::string protocol;
  std::string host;
  uint16_t port;
  std::string dir;
  std::string file;
  std::string query;
  std::string username;
  std::string password;
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

std::string construct(const UriStruct& us);

std::string joinUri(const std::string& baseUri, const std::string& uri);

} // namespace uri

} // namespace aria2

#endif // D_URI_H
