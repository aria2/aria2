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
#ifndef _D_AUTH_CONFIG_FACTORY_H_
#define _D_AUTH_CONFIG_FACTORY_H_

#include "common.h"

#include <string>

#include "SharedHandle.h"
#include "SingletonHolder.h"

namespace aria2 {

class Option;
class Netrc;
class AuthConfig;
class Request;
class AuthResolver;

class AuthConfigFactory {
private:
  const Option* _option;

  SharedHandle<Netrc> _netrc;
  
  SharedHandle<AuthConfig> createAuthConfig(const std::string& user,
					    const std::string& password) const;

  SharedHandle<AuthResolver> createHttpAuthResolver() const;
  
  SharedHandle<AuthResolver> createFtpAuthResolver() const;

public:
  
  AuthConfigFactory(const Option* option);

  ~AuthConfigFactory();

  SharedHandle<AuthConfig> createAuthConfig
  (const SharedHandle<Request>& request) const;

  void setNetrc(const SharedHandle<Netrc>& netrc);

  static const std::string ANONYMOUS;

  static const std::string ARIA2USER_AT;
};

typedef SharedHandle<AuthConfigFactory> AuthConfigFactoryHandle;

} // namespace aria2

#endif // _D_AUTH_CONFIG_FACTORY_H_
