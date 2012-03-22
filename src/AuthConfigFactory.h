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
#ifndef D_AUTH_CONFIG_FACTORY_H
#define D_AUTH_CONFIG_FACTORY_H

#include "common.h"

#include <string>
#include <set>

#include "SharedHandle.h"
#include "SingletonHolder.h"
#include "a2functional.h"

namespace aria2 {

class Option;
class Netrc;
class AuthConfig;
class Request;
class AuthResolver;

class AuthConfigFactory {
private:
  SharedHandle<Netrc> netrc_;
  
  SharedHandle<AuthConfig> createAuthConfig(const std::string& user,
                                            const std::string& password) const;

  SharedHandle<AuthResolver> createHttpAuthResolver(const Option* op) const;
  
  SharedHandle<AuthResolver> createFtpAuthResolver(const Option* op) const;
public:
  class BasicCred {
  public:
    std::string user_;
    std::string password_;
    std::string host_;
    uint16_t port_;
    std::string path_;
    bool activated_;

    BasicCred(const std::string& user, const std::string& password,
              const std::string& host, uint16_t port, const std::string& path,
              bool activated = false);

    void activate();

    bool isActivated() const;

    bool operator==(const BasicCred& cred) const;

    bool operator<(const BasicCred& cred) const;
  };

  typedef std::set<SharedHandle<BasicCred>,
                   DerefLess<SharedHandle<BasicCred> > > BasicCredSet;
private:
  BasicCredSet basicCreds_;
public:
  
  AuthConfigFactory();

  ~AuthConfigFactory();

  // Creates AuthConfig object for request. Following option values
  // are used in this method: PREF_HTTP_USER, PREF_HTTP_PASSWD,
  // PREF_FTP_USER, PREF_FTP_PASSWD, PREF_NO_NETRC and
  // PREF_HTTP_AUTH_CHALLENGE.
  SharedHandle<AuthConfig> createAuthConfig
  (const SharedHandle<Request>& request, const Option* op);

  void setNetrc(const SharedHandle<Netrc>& netrc);

  // Find a BasicCred using findBasicCred() and activate it then
  // return true.  If matching BasicCred is not found, AuthConfig
  // object is created using createHttpAuthResolver and op.  If it is
  // null, then returns false. Otherwise new BasicCred is created
  // using this AuthConfig object with given host and path "/" and
  // returns true.
  bool activateBasicCred
  (const std::string& host,
   uint16_t port,
   const std::string& path,
   const Option* op);

  // Find a BasicCred using host, port and path and return the
  // iterator pointing to it. If not found, then return
  // basicCreds_.end().
  BasicCredSet::iterator
  findBasicCred
  (const std::string& host,
   uint16_t port,
   const std::string& path);

  // If the same BasicCred is already added, then it is replaced with
  // given basicCred. Otherwise, insert given basicCred to
  // basicCreds_.
  void updateBasicCred(const SharedHandle<BasicCred>& basicCred);

  static const std::string ANONYMOUS;

  static const std::string ARIA2USER_AT;
};

typedef SharedHandle<AuthConfigFactory> AuthConfigFactoryHandle;

} // namespace aria2

#endif // D_AUTH_CONFIG_FACTORY_H
