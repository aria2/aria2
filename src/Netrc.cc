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
#include "Netrc.h"
#include "RecoverableException.h"
#include "StringFormat.h"
#include "A2STR.h"
#include <fstream>
#include <algorithm>

namespace aria2 {

const std::string Netrc::MACHINE("machine");

const std::string Netrc::DEFAULT("default");

const std::string Netrc::LOGIN("login");

const std::string Netrc::PASSWORD("password");

const std::string Netrc::ACCOUNT("account");

const std::string Netrc::MACDEF("macdef");

std::string Netrc::getRequiredNextToken(std::ifstream& f) const
{
  std::string token;
  if(f >> token) {
    return token;
  } else {
    throw RecoverableException
      ("Netrc:parse error. EOF reached where a token expected.");
  }
}

void Netrc::skipMacdef(std::ifstream& f) const
{
  std::string line;
  getline(f, line);
  while(getline(f, line)) {
    if(line == A2STR::CR_C || line.empty()) {
      break;
    }
  }
}

void Netrc::parse(const std::string& path)
{
  authenticators.clear();
  std::ifstream f(path.c_str(), std::ios::binary);
  
  if(!f) {
    throw RecoverableException
      (StringFormat("File not found: %s", path.c_str()).str());
  }

  AuthenticatorHandle authenticator;
  std::string token;
  while(f >> token) {
    if(token == Netrc::MACHINE) {
      storeAuthenticator(authenticator);
      authenticator.reset(new Authenticator());
      authenticator->setMachine(getRequiredNextToken(f));
    } else if(token == Netrc::DEFAULT) {
      storeAuthenticator(authenticator);
      authenticator.reset(new DefaultAuthenticator());
    } else {
      if(authenticator.isNull()) {
	throw RecoverableException
	  ("Netrc:parse error. %s encounterd where 'machine' or 'default' expected.");
      }
      if(token == Netrc::LOGIN) {
	authenticator->setLogin(getRequiredNextToken(f));
      } else if(token == Netrc::PASSWORD) {
	authenticator->setPassword(getRequiredNextToken(f));
      } else if(token == Netrc::ACCOUNT) {
	authenticator->setAccount(getRequiredNextToken(f));
      } else if(token == Netrc::MACDEF) {
	getRequiredNextToken(f);
	skipMacdef(f);
      }
    }
  }
  storeAuthenticator(authenticator);
}

void Netrc::storeAuthenticator(const AuthenticatorHandle& authenticator)
{
  if(!authenticator.isNull()) {
    authenticators.push_back(authenticator);
  }
}

class AuthHostMatch {
private:
  std::string hostname;
public:
  AuthHostMatch(const std::string& hostname):hostname(hostname) {}

  bool operator()(const AuthenticatorHandle& authenticator)
  {
    return authenticator->match(hostname);
  }
};

AuthenticatorHandle Netrc::findAuthenticator(const std::string& hostname) const
{
  Authenticators::const_iterator itr =
    std::find_if(authenticators.begin(), authenticators.end(),
		 AuthHostMatch(hostname));
  if(itr == authenticators.end()) {
    return SharedHandle<Authenticator>();
  } else {
    return *itr;
  }
}

} // namespace aria2
