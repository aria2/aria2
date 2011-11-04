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
#ifndef D_NETRC_H
#define D_NETRC_H

#include "common.h"

#include <string>
#include <vector>

#include "SharedHandle.h"

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
  Authenticator();

  Authenticator(const std::string& machine,
                const std::string& login,
                const std::string& password,
                const std::string& account);

  virtual ~Authenticator();

  virtual bool match(const std::string& hostname) const;

  const std::string& getMachine() const
  {
    return machine_;
  }

  void setMachine(const std::string& machine);

  template<typename InputIterator>
  void setMachine(InputIterator first, InputIterator last)
  {
    machine_.assign(first, last);
  }

  const std::string& getLogin() const
  {
    return login_;
  }

  void setLogin(const std::string& login);

  template<typename InputIterator>
  void setLogin(InputIterator first, InputIterator last)
  {
    login_.assign(first, last);
  }

  const std::string& getPassword() const
  {
    return password_;
  }

  void setPassword(const std::string& password);

  template<typename InputIterator>
  void setPassword(InputIterator first, InputIterator last)
  {
    password_.assign(first, last);
  }

  const std::string& getAccount() const
  {
    return account_;
  }

  void setAccount(const std::string& account);

  template<typename InputIterator>
  void setAccount(InputIterator first, InputIterator last)
  {
    account_.assign(first, last);
  }
};

class DefaultAuthenticator : public Authenticator {
public:
  DefaultAuthenticator();

  DefaultAuthenticator(const std::string& login,
                       const std::string& password,
                       const std::string& account);

  virtual ~DefaultAuthenticator();

  virtual bool match(const std::string& hostname) const;
};

class Netrc {
private:
  std::vector<SharedHandle<Authenticator> > authenticators_;

  void storeAuthenticator(const SharedHandle<Authenticator>& authenticator);
public:
  Netrc();

  ~Netrc();

  void parse(const std::string& path);

  SharedHandle<Authenticator> findAuthenticator
  (const std::string& hostname) const;

  const std::vector<SharedHandle<Authenticator> >& getAuthenticators() const
  {
    return authenticators_;
  }

  void addAuthenticator(const SharedHandle<Authenticator>& authenticator);
};

} // namespace aria2

#endif // D_NETRC_H
