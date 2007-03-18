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
#include "Util.h"
#include "RecoverableException.h"
#include <fstream>

string Netrc::getRequiredNextToken(ifstream& f) const
{
  string token;
  if(f >> token) {
    return token;
  } else {
    throw new RecoverableException("Netrc:parse error. EOF reached where a token expected.");
  }
}

void Netrc::skipMacdef(ifstream& f) const
{
  string line;
  getline(f, line);
  while(getline(f, line)) {
    if(line == "\r" || line == "") {
      break;
    }
  }
}

void Netrc::parse(const string& path)
{
  authenticators.clear();
  ifstream f(path.c_str());
  
  if(!f) {
    throw new RecoverableException("File not found: %s", path.c_str());
  }

  AuthenticatorHandle authenticator = 0;
  string token;
  while(f >> token) {
    if(token == "machine") {
      storeAuthenticator(authenticator);
      authenticator = new Authenticator();
      authenticator->setMachine(getRequiredNextToken(f));
    } else if(token == "default") {
      storeAuthenticator(authenticator);
      authenticator = new DefaultAuthenticator();
    } else {
      if(authenticator.isNull()) {
	throw new RecoverableException("Netrc:parse error. %s encounterd where 'machine' or 'default' expected.");
      }
      if(token == "login") {
	authenticator->setLogin(getRequiredNextToken(f));
      } else if(token == "password") {
	authenticator->setPassword(getRequiredNextToken(f));
      } else if(token == "account") {
	authenticator->setAccount(getRequiredNextToken(f));
      } else if(token == "macdef") {
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
  string hostname;
public:
  AuthHostMatch(const string& hostname):hostname(hostname) {}

  bool operator()(const AuthenticatorHandle& authenticator)
  {
    return authenticator->match(hostname);
  }
};

AuthenticatorHandle Netrc::findAuthenticator(const string& hostname) const
{
  Authenticators::const_iterator itr =
    find_if(authenticators.begin(), authenticators.end(),
	    AuthHostMatch(hostname));
  if(itr == authenticators.end()) {
    return 0;
  } else {
    return *itr;
  }
}
