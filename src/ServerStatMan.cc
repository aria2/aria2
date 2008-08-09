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
#include "ServerStatMan.h"
#include "ServerStat.h"
#include "Util.h"
#include "RecoverableException.h"
#include <algorithm>
#include <ostream>
#include <iterator>
#include <map>

namespace aria2 {

ServerStatMan::ServerStatMan() {}

ServerStatMan::~ServerStatMan() {}

SharedHandle<ServerStat> ServerStatMan::find(const std::string& hostname,
					     const std::string& protocol) const
{
  SharedHandle<ServerStat> ss(new ServerStat(hostname, protocol));
  std::deque<SharedHandle<ServerStat> >::const_iterator i =
    std::lower_bound(_serverStats.begin(), _serverStats.end(), ss);
  if(i != _serverStats.end() &&
     (*i)->getHostname() == hostname && (*i)->getProtocol() == protocol) {
    return *i;
  } else {
    return SharedHandle<ServerStat>();
  }
}

bool ServerStatMan::add(const SharedHandle<ServerStat>& serverStat)
{
  std::deque<SharedHandle<ServerStat> >::iterator i =
    std::lower_bound(_serverStats.begin(), _serverStats.end(), serverStat);

  if(i != _serverStats.end() && (*i) == serverStat) {
    return false;
  } else {
    _serverStats.insert(i, serverStat);
    return true;
  } 
}

void ServerStatMan::save(std::ostream& out) const
{
  std::copy(_serverStats.begin(), _serverStats.end(),
	    std::ostream_iterator<SharedHandle<ServerStat> >(out, "\n"));
}

void ServerStatMan::load(std::istream& in)
{
  static const std::string S_HOST = "host";
  static const std::string S_PROTOCOL = "protocol";
  static const std::string S_DL_SPEED = "dl_speed";
  static const std::string S_LAST_UPDATED = "last_updated";
  static const std::string S_STATUS = "status";

  std::string line;
  while(getline(in, line)) {
    Util::trimSelf(line);
    if(line.empty()) {
      continue;
    }
    std::deque<std::string> items;
    Util::slice(items, line, ',');
    std::map<std::string, std::string> m;
    for(std::deque<std::string>::const_iterator i = items.begin();
	i != items.end(); ++i) {
      std::pair<std::string, std::string> p = Util::split(*i, "=");
      Util::trimSelf(p.first);
      Util::trimSelf(p.second);
      m[p.first] = p.second;
    }
    if(m[S_HOST].empty() || m[S_PROTOCOL].empty()) {
      continue;
    }
    SharedHandle<ServerStat> sstat(new ServerStat(m[S_HOST], m[S_PROTOCOL]));
    try {
      sstat->setDownloadSpeed(Util::parseUInt(m[S_DL_SPEED]));
      sstat->setLastUpdated(Time(Util::parseInt(m[S_LAST_UPDATED])));
      sstat->setStatus(m[S_STATUS]);
      add(sstat);
    } catch(RecoverableException* e) {
      continue;
    }
  }
}

class FindStaleServerStat {
private:
  time_t _timeout;
public:
  FindStaleServerStat(time_t timeout):_timeout(timeout) {}

  bool operator()(const SharedHandle<ServerStat>& ss) const
  {
    return ss->getLastUpdated().elapsed(_timeout);
  }
};

void ServerStatMan::removeStaleServerStat(time_t timeout)
{
  _serverStats.erase(std::remove_if(_serverStats.begin(), _serverStats.end(),
				    FindStaleServerStat(timeout)),
		     _serverStats.end());
}

} // namespace aria2
