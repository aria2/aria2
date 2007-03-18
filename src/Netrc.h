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
#ifndef _D_NETRC_H_
#define _D_NETRC_H_

#include "common.h"

class Authenticatable {
public:
  virtual ~Authenticatable() {}

  virtual bool match(const string& hostname) const = 0;
};

typedef SharedHandle<Authenticatable> AuthenticatableHandle;

class Authenticator : public Authenticatable {
private:
  string machine;
  string login;
  string password;
  string account;
public:
  Authenticator() {}

  Authenticator(const string& machine,
		const string& login,
		const string& password,
		const string& account)
    :machine(machine),
     login(login),
     password(password),
     account(account) {}

  virtual ~Authenticator() {}

  virtual bool match(const string& hostname) const
  {
    return hostname == machine;
  }

  const string& getMachine() const
  {
    return machine;
  }

  void setMachine(const string& machine) { this->machine = machine; }

  const string& getLogin() const
  {
    return login;
  }

  void setLogin(const string& login) { this->login = login; }

  const string& getPassword() const
  {
    return password;
  }

  void setPassword(const string& password) { this->password = password; }

  const string& getAccount() const
  {
    return account;
  }

  void setAccount(const string& account) { this->account = account; }
};

typedef SharedHandle<Authenticator> AuthenticatorHandle;
typedef deque<AuthenticatorHandle> Authenticators;

class DefaultAuthenticator : public Authenticator {
public:
  DefaultAuthenticator() {}

  DefaultAuthenticator(const string& login,
		       const string& password,
		       const string& account)
    :Authenticator("", login, password, account) {}

  virtual ~DefaultAuthenticator() {}

  virtual bool match(const string& hostname) const
  {
    return true;
  }
};

typedef SharedHandle<DefaultAuthenticator> DefaultAuthenticatorHandle;

class Netrc {
private:
  Authenticators authenticators;

  void storeAuthenticator(const AuthenticatorHandle& authenticator);
public:
  Netrc() {}

  void parse(const string& path);

  AuthenticatorHandle findAuthenticator(const string& hostname) const;

  const Authenticators& getAuthenticators() const
  {
    return authenticators;
  }

  void addAuthenticator(const AuthenticatorHandle& authenticator)
  {
    authenticators.push_back(authenticator);
  }
};

typedef SharedHandle<Netrc> NetrcHandle;
typedef SingletonHolder<NetrcHandle> NetrcSingletonHolder;

#endif // _D_NETRC_H_
