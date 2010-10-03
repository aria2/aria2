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
#ifndef _D_NETRC_H_
#define _D_NETRC_H_

#include "common.h"

#include <string>
#include <vector>
#include <iosfwd>

#include "SharedHandle.h"
#include "A2STR.h"
#include "util.h"

namespace aria2 {

class Authenticatable {
public:
  virtual ~Authenticatable() {}

  virtual bool match(const std::string& hostname) const = 0;
};

class Authenticator : public Authenticatable {
private:
  std::string machine_;
  std::string login_;
  std::string password_;
  std::string account_;
public:
  Authenticator() {}

  Authenticator(const std::string& machine,
                const std::string& login,
                const std::string& password,
                const std::string& account)
    :machine_(machine),
     login_(login),
     password_(password),
     account_(account) {}

  virtual ~Authenticator() {}

  virtual bool match(const std::string& hostname) const
  {
    if(util::isNumericHost(hostname)) {
      return hostname == machine_;
    } else {
      if(util::startsWith(machine_, A2STR::DOT_C)) {
        return util::endsWith(A2STR::DOT_C+hostname, machine_);
      } else {
        return hostname == machine_;
      }
    }
  }

  const std::string& getMachine() const
  {
    return machine_;
  }

  void setMachine(const std::string& machine) { machine_ = machine; }

  const std::string& getLogin() const
  {
    return login_;
  }

  void setLogin(const std::string& login) { login_ = login; }

  const std::string& getPassword() const
  {
    return password_;
  }

  void setPassword(const std::string& password) { password_ = password; }

  const std::string& getAccount() const
  {
    return account_;
  }

  void setAccount(const std::string& account) { account_ = account; }
};

class DefaultAuthenticator : public Authenticator {
public:
  DefaultAuthenticator() {}

  DefaultAuthenticator(const std::string& login,
                       const std::string& password,
                       const std::string& account)
    :Authenticator(A2STR::NIL, login, password, account) {}

  virtual ~DefaultAuthenticator() {}

  virtual bool match(const std::string& hostname) const
  {
    return true;
  }
};

class Netrc {
private:
  std::vector<SharedHandle<Authenticator> > authenticators_;

  void storeAuthenticator(const SharedHandle<Authenticator>& authenticator);

  std::string getRequiredNextToken(std::ifstream& f) const;
  
  void skipMacdef(std::ifstream& f) const;
public:
  Netrc() {}

  void parse(const std::string& path);

  SharedHandle<Authenticator> findAuthenticator
  (const std::string& hostname) const;

  const std::vector<SharedHandle<Authenticator> >& getAuthenticators() const
  {
    return authenticators_;
  }

  void addAuthenticator(const SharedHandle<Authenticator>& authenticator)
  {
    authenticators_.push_back(authenticator);
  }

  static const std::string A2_MACHINE;

  static const std::string A2_DEFAULT;

  static const std::string A2_LOGIN;

  static const std::string A2_PASSWORD;

  static const std::string A2_ACCOUNT;

  static const std::string A2_MACDEF;
};

} // namespace aria2

#endif // _D_NETRC_H_
