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
void skipMacdef(FILE* fp)
{
  char buf[4096];
  while(1) {
    if(!fgets(buf, sizeof(buf), fp)) {
      break;
    }
    if(buf[0] == '\n' || buf[0] == '\r') {
      break;
    }
  }
}
} // namespace

void Netrc::parse(const std::string& path)
{
  authenticators_.clear();
  FILE* fp = a2fopen(utf8ToWChar(path).c_str(), "rb");
  if(!fp) {
    throw DL_ABORT_EX(fmt("Cannot open file: %s", utf8ToNative(path).c_str()));
  }
  auto_delete_r<FILE*, int> deleter(fp, fclose);

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
  char buf[4096];
  while(1) {
    if(!fgets(buf, sizeof(buf), fp)) {
      break;
    }
    size_t len = strlen(buf);
    if(buf[len-1] == '\n') {
      buf[len-1] = '\0';
    }
    std::string line(buf);
    if(util::startsWith(line, "#")) {
      continue;
    }
    std::vector<std::string> tokens;
    util::split(line, std::back_inserter(tokens), " \t", true);
    for(std::vector<std::string>::const_iterator iter = tokens.begin(),
          eoi = tokens.end(); iter != eoi; ++iter) {
      const std::string& token = *iter;
      if(state == GET_TOKEN) {
        if(token == "machine") {
          storeAuthenticator(authenticator);
          authenticator.reset(new Authenticator());
          state = SET_MACHINE;
        } else if(token == "default") {
          storeAuthenticator(authenticator);
          authenticator.reset(new DefaultAuthenticator());
        } else {
          if(!authenticator) {
            throw DL_ABORT_EX
              (fmt("Netrc:parse error. %s encounterd where 'machine'"
                   " or 'default' expected.", token.c_str()));
          }
          if(token == "login") {
            state = SET_LOGIN;
          } else if(token == "password") {
            state = SET_PASSWORD;
          } else if(token == "account") {
            state = SET_ACCOUNT;
          } else if(token == "macdef") {
            state = SET_MACDEF;
          }
        }
      } else {
        if(state == SET_MACHINE) {
          authenticator->setMachine(token);
        } else if(state == SET_LOGIN) {
          authenticator->setLogin(token);
        } else if(state == SET_PASSWORD) {
          authenticator->setPassword(token);
        } else if(state == SET_ACCOUNT) {
          authenticator->setAccount(token);
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
