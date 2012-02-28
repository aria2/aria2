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
#include "Netrc.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "DlAbortEx.h"
#include "fmt.h"
#include "A2STR.h"
#include "util.h"
#include "BufferedFile.h"
#include "array_fun.h"

namespace aria2 {

Authenticator::Authenticator() {}

Authenticator::Authenticator
(const std::string& machine,
 const std::string& login,
 const std::string& password,
 const std::string& account)
  : machine_(machine),
    login_(login),
    password_(password),
    account_(account)
{}

Authenticator::~Authenticator() {}

bool Authenticator::match(const std::string& hostname) const
{
  return util::noProxyDomainMatch(hostname, machine_);
}

void Authenticator::setMachine(const std::string& machine)
{
  machine_ = machine;
}

void Authenticator::setLogin(const std::string& login)
{
  login_ = login;
}

void Authenticator::setPassword(const std::string& password)
{
  password_ = password;
}

void Authenticator::setAccount(const std::string& account)
{
  account_ = account;
}

DefaultAuthenticator::DefaultAuthenticator() {}

DefaultAuthenticator::DefaultAuthenticator
(const std::string& login,
 const std::string& password,
 const std::string& account)
  : Authenticator(A2STR::NIL, login, password, account)
{}

DefaultAuthenticator::~DefaultAuthenticator() {}

bool DefaultAuthenticator::match(const std::string& hostname) const
{
  return true;
}

Netrc::Netrc() {}

Netrc::~Netrc() {}

void Netrc::addAuthenticator(const SharedHandle<Authenticator>& authenticator)
{
  authenticators_.push_back(authenticator);
}

namespace {
void skipMacdef(BufferedFile& fp)
{
  std::string s;
  while(1) {
    s = fp.getLine();
    if(s.empty() || fp.eof()) {
      break;
    }
    if(!fp) {
      throw DL_ABORT_EX("Netrc:I/O error.");
    }
    if(s[0] == '\n' || s[0] == '\r') {
      break;
    }
  }
}
} // namespace

void Netrc::parse(const std::string& path)
{
  authenticators_.clear();
  BufferedFile fp(path, BufferedFile::READ);
  if(!fp) {
    throw DL_ABORT_EX(fmt("Cannot open file: %s", path.c_str()));
  }
  enum STATE {
    GET_TOKEN,
    SET_MACHINE,
    SET_LOGIN,
    SET_PASSWORD,
    SET_ACCOUNT,
    SET_MACDEF
  };
  SharedHandle<Authenticator> authenticator;
  STATE state = GET_TOKEN;
  while(1) {
    std::string line = fp.getLine();
    if(line.empty()) {
      if(fp.eof()) {
        break;
      } else if(!fp) {
        throw DL_ABORT_EX("Netrc:I/O error.");
      } else {
        continue;
      }
    }
    if(line[0] == '#') {
      continue;
    }
    std::vector<Scip> tokens;
    util::splitIterM(line.begin(), line.end(), std::back_inserter(tokens),
                     " \t", true);
    for(std::vector<Scip>::const_iterator iter = tokens.begin(),
          eoi = tokens.end(); iter != eoi; ++iter) {
      if(state == GET_TOKEN) {
        if(util::streq((*iter).first, (*iter).second, "machine")) {
          storeAuthenticator(authenticator);
          authenticator.reset(new Authenticator());
          state = SET_MACHINE;
        } else if(util::streq((*iter).first, (*iter).second, "default")) {
          storeAuthenticator(authenticator);
          authenticator.reset(new DefaultAuthenticator());
        } else {
          if(!authenticator) {
            throw DL_ABORT_EX
              (fmt("Netrc:parse error. %s encounterd where 'machine'"
                   " or 'default' expected.",
                   std::string((*iter).first, (*iter).second).c_str()));
          }
          if(util::streq((*iter).first, (*iter).second, "login")) {
            state = SET_LOGIN;
          } else if(util::streq((*iter).first, (*iter).second, "password")) {
            state = SET_PASSWORD;
          } else if(util::streq((*iter).first, (*iter).second, "account")) {
            state = SET_ACCOUNT;
          } else if(util::streq((*iter).first, (*iter).second, "macdef")) {
            state = SET_MACDEF;
          }
        }
      } else {
        if(state == SET_MACHINE) {
          authenticator->setMachine((*iter).first, (*iter).second);
        } else if(state == SET_LOGIN) {
          authenticator->setLogin((*iter).first, (*iter).second);
        } else if(state == SET_PASSWORD) {
          authenticator->setPassword((*iter).first, (*iter).second);
        } else if(state == SET_ACCOUNT) {
          authenticator->setAccount((*iter).first, (*iter).second);
        } else if(state == SET_MACDEF) {
          skipMacdef(fp);
        }
        state = GET_TOKEN;
      } 
    }
  }
  if(state != GET_TOKEN) {
    throw DL_ABORT_EX
      ("Netrc:parse error. EOF reached where a token expected.");
  }
  storeAuthenticator(authenticator);
}

void Netrc::storeAuthenticator(const SharedHandle<Authenticator>& authenticator)
{
  if(authenticator) {
    authenticators_.push_back(authenticator);
  }
}

namespace {
class AuthHostMatch {
private:
  std::string hostname;
public:
  AuthHostMatch(const std::string& hostname):hostname(hostname) {}

  bool operator()(const SharedHandle<Authenticator>& authenticator)
  {
    return authenticator->match(hostname);
  }
};
} // namespace

SharedHandle<Authenticator>
Netrc::findAuthenticator(const std::string& hostname) const
{
  SharedHandle<Authenticator> res;
  std::vector<SharedHandle<Authenticator> >::const_iterator itr =
    std::find_if(authenticators_.begin(), authenticators_.end(),
                 AuthHostMatch(hostname));
  if(itr != authenticators_.end()) {
    res = *itr;
  }
  return res;
}

} // namespace aria2
