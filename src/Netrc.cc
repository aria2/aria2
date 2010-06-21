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

#include <fstream>
#include <algorithm>

#include "DlAbortEx.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "util.h"

namespace aria2 {

const std::string Netrc::MACHINE("machine");

const std::string Netrc::DEFAULT("default");

const std::string Netrc::LOGIN("login");

const std::string Netrc::PASSWORD("password");

const std::string Netrc::ACCOUNT("account");

const std::string Netrc::MACDEF("macdef");

void Netrc::skipMacdef(std::ifstream& f) const
{
  std::string line;
  while(getline(f, line)) {
    if(line == A2STR::CR_C || line.empty()) {
      break;
    }
  }
}

void Netrc::parse(const std::string& path)
{
  authenticators_.clear();
  std::ifstream f(path.c_str(), std::ios::binary);
  
  if(!f) {
    throw DL_ABORT_EX
      (StringFormat("File not found: %s", path.c_str()).str());
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
  std::string line;
  STATE state = GET_TOKEN;
  while(getline(f, line)) {
    if(util::startsWith(line, "#")) {
      continue;
    }
    std::vector<std::string> tokens;
    util::split(line, std::back_inserter(tokens), " \t", true);
    for(std::vector<std::string>::const_iterator iter = tokens.begin(),
          eoi = tokens.end(); iter != eoi; ++iter) {
      const std::string& token = *iter;
      if(state == GET_TOKEN) {
        if(token == Netrc::MACHINE) {
          storeAuthenticator(authenticator);
          authenticator.reset(new Authenticator());
          state = SET_MACHINE;
        } else if(token == Netrc::DEFAULT) {
          storeAuthenticator(authenticator);
          authenticator.reset(new DefaultAuthenticator());
        } else {
          if(authenticator.isNull()) {
            throw DL_ABORT_EX
              (StringFormat("Netrc:parse error. %s encounterd where 'machine'"
                            " or 'default' expected.", token.c_str()).str());
          }
          if(token == Netrc::LOGIN) {
            state = SET_LOGIN;
          } else if(token == Netrc::PASSWORD) {
            state = SET_PASSWORD;
          } else if(token == Netrc::ACCOUNT) {
            state = SET_ACCOUNT;
          } else if(token == Netrc::MACDEF) {
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
          skipMacdef(f);
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
  if(!authenticator.isNull()) {
    authenticators_.push_back(authenticator);
  }
}

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
