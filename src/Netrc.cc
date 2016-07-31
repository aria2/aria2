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

Authenticator::Authenticator(std::string machine, std::string login,
                             std::string password, std::string account)
    : machine_(std::move(machine)),
      login_(std::move(login)),
      password_(std::move(password)),
      account_(std::move(account))
{
}

Authenticator::~Authenticator() = default;

bool Authenticator::match(const std::string& hostname) const
{
  return util::noProxyDomainMatch(hostname, machine_);
}

void Authenticator::setMachine(std::string machine)
{
  machine_ = std::move(machine);
}

void Authenticator::setLogin(std::string login) { login_ = std::move(login); }

void Authenticator::setPassword(std::string password)
{
  password_ = std::move(password);
}

void Authenticator::setAccount(std::string account)
{
  account_ = std::move(account);
}

DefaultAuthenticator::DefaultAuthenticator() {}

DefaultAuthenticator::DefaultAuthenticator(std::string login,
                                           std::string password,
                                           std::string account)
    : Authenticator("", std::move(login), std::move(password),
                    std::move(account))
{
}

DefaultAuthenticator::~DefaultAuthenticator() = default;

bool DefaultAuthenticator::match(const std::string& hostname) const
{
  return true;
}

Netrc::Netrc() {}

Netrc::~Netrc() = default;

void Netrc::addAuthenticator(std::unique_ptr<Authenticator> authenticator)
{
  authenticators_.push_back(std::move(authenticator));
}

namespace {
void skipMacdef(BufferedFile& fp)
{
  std::string s;
  while (1) {
    s = fp.getLine();
    if (s.empty() || fp.eof()) {
      break;
    }
    if (!fp) {
      throw DL_ABORT_EX("Netrc:I/O error.");
    }
    if (s[0] == '\n' || s[0] == '\r') {
      break;
    }
  }
}
} // namespace

void Netrc::parse(const std::string& path)
{
  authenticators_.clear();
  BufferedFile fp(path.c_str(), BufferedFile::READ);
  if (!fp) {
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
  std::unique_ptr<Authenticator> authenticator;
  STATE state = GET_TOKEN;
  while (1) {
    std::string line = fp.getLine();
    if (line.empty()) {
      if (fp.eof()) {
        break;
      }
      else if (!fp) {
        throw DL_ABORT_EX("Netrc:I/O error.");
      }
      else {
        continue;
      }
    }
    if (line[0] == '#') {
      continue;
    }
    std::vector<Scip> tokens;
    util::splitIterM(line.begin(), line.end(), std::back_inserter(tokens),
                     " \t", true);
    for (std::vector<Scip>::const_iterator iter = tokens.begin(),
                                           eoi = tokens.end();
         iter != eoi; ++iter) {
      if (state == GET_TOKEN) {
        if (util::streq((*iter).first, (*iter).second, "machine")) {
          storeAuthenticator(std::move(authenticator));
          authenticator = make_unique<Authenticator>();
          state = SET_MACHINE;
        }
        else if (util::streq((*iter).first, (*iter).second, "default")) {
          storeAuthenticator(std::move(authenticator));
          authenticator = make_unique<DefaultAuthenticator>();
        }
        else {
          if (!authenticator) {
            throw DL_ABORT_EX(
                fmt("Netrc:parse error. %s encounterd where 'machine'"
                    " or 'default' expected.",
                    std::string((*iter).first, (*iter).second).c_str()));
          }
          if (util::streq((*iter).first, (*iter).second, "login")) {
            state = SET_LOGIN;
          }
          else if (util::streq((*iter).first, (*iter).second, "password")) {
            state = SET_PASSWORD;
          }
          else if (util::streq((*iter).first, (*iter).second, "account")) {
            state = SET_ACCOUNT;
          }
          else if (util::streq((*iter).first, (*iter).second, "macdef")) {
            state = SET_MACDEF;
          }
        }
      }
      else {
        if (state == SET_MACHINE) {
          authenticator->setMachine({(*iter).first, (*iter).second});
        }
        else if (state == SET_LOGIN) {
          authenticator->setLogin({(*iter).first, (*iter).second});
        }
        else if (state == SET_PASSWORD) {
          authenticator->setPassword({(*iter).first, (*iter).second});
        }
        else if (state == SET_ACCOUNT) {
          authenticator->setAccount({(*iter).first, (*iter).second});
        }
        else if (state == SET_MACDEF) {
          skipMacdef(fp);
        }
        state = GET_TOKEN;
      }
    }
  }
  if (state != GET_TOKEN) {
    throw DL_ABORT_EX("Netrc:parse error. EOF reached where a token expected.");
  }
  storeAuthenticator(std::move(authenticator));
}

void Netrc::storeAuthenticator(std::unique_ptr<Authenticator> authenticator)
{
  if (authenticator) {
    authenticators_.push_back(std::move(authenticator));
  }
}

namespace {
class AuthHostMatch {
private:
  std::string hostname;

public:
  AuthHostMatch(std::string hostname) : hostname(std::move(hostname)) {}

  bool operator()(const std::unique_ptr<Authenticator>& authenticator)
  {
    return authenticator->match(hostname);
  }
};
} // namespace

const Authenticator* Netrc::findAuthenticator(const std::string& hostname) const
{
  std::unique_ptr<Authenticator> res;
  auto itr = std::find_if(std::begin(authenticators_),
                          std::end(authenticators_), AuthHostMatch(hostname));
  if (itr == std::end(authenticators_)) {
    return nullptr;
  }
  else {
    return (*itr).get();
  }
}

const std::vector<std::unique_ptr<Authenticator>>&
Netrc::getAuthenticators() const
{
  return authenticators_;
}

} // namespace aria2
