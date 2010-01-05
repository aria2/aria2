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
#ifndef _D_DNS_CACHE_H_
#define _D_DNS_CACHE_H_

#include "common.h"

#include <string>
#include <deque>
#include <algorithm>
#include <vector>

#include "A2STR.h"

namespace aria2 {

class DNSCache {
private:
  struct AddrEntry {
    std::string _addr;
    bool _good;

    AddrEntry(const std::string& addr):_addr(addr), _good(true) {}
  };

  struct CacheEntry {
    std::string _hostname;
    uint16_t _port;
    std::vector<AddrEntry> _addrEntries;

    CacheEntry
    (const std::string& hostname, uint16_t port):
      _hostname(hostname), _port(port) {}

    void add(const std::string& addr)
    {
      _addrEntries.push_back(AddrEntry(addr));
    }

    std::vector<AddrEntry>::iterator find(const std::string& addr)
    {
      for(std::vector<AddrEntry>::iterator i = _addrEntries.begin();
          i != _addrEntries.end(); ++i) {
        if((*i)._addr == addr) {
          return i;
        }
      }
      return _addrEntries.end();
    }

    std::vector<AddrEntry>::const_iterator find(const std::string& addr) const
    {
      for(std::vector<AddrEntry>::const_iterator i = _addrEntries.begin();
          i != _addrEntries.end(); ++i) {
        if((*i)._addr == addr) {
          return i;
        }
      }
      return _addrEntries.end();
    }

    bool contains(const std::string& addr) const
    {
      return find(addr) != _addrEntries.end();
    }

    const std::string& getGoodAddr() const
    {
      for(std::vector<AddrEntry>::const_iterator i = _addrEntries.begin();
          i != _addrEntries.end(); ++i) {
        if((*i)._good) {
          return (*i)._addr;
        }
      }
      return A2STR::NIL;
    }

    void markBad(const std::string& addr)
    {
      std::vector<AddrEntry>::iterator i = find(addr);
      if(i != _addrEntries.end()) {
        (*i)._good = false;
      }
    }

    bool operator<(const CacheEntry& e) const
    {
      int r = _hostname.compare(e._hostname);
      if(r != 0) {
        return r < 0;
      }
      return _port < e._port;
    }

    bool operator==(const CacheEntry& e) const
    {
      return _hostname == e._hostname && _port == e._port;
    }
  };

  std::deque<CacheEntry> _entries;

public:
  const std::string& find(const std::string& hostname, uint16_t port) const
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::const_iterator i =
      std::lower_bound(_entries.begin(), _entries.end(), target);
    if(i != _entries.end() && (*i) == target) {
      return (*i).getGoodAddr();
    }
    return A2STR::NIL;
  }

  void put
  (const std::string& hostname, const std::string& ipaddr, uint16_t port)
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::iterator i =
      std::lower_bound(_entries.begin(), _entries.end(), target);
    if(i == _entries.end() || !((*i) == target)) {
      target.add(ipaddr);
      _entries.insert(i, target);
    } else {
      if(!(*i).contains(ipaddr)) {
        (*i).add(ipaddr);
      }
    }
  }

  void markBad
  (const std::string& hostname, const std::string& ipaddr, uint16_t port)
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::iterator i =
      std::lower_bound(_entries.begin(), _entries.end(), target);
    if(i != _entries.end() && (*i) == target) {
      (*i).markBad(ipaddr);
    }
  }

  void remove(const std::string& hostname, uint16_t port)
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::iterator i =
      std::lower_bound(_entries.begin(), _entries.end(), target);
    if(i != _entries.end() && (*i) == target) {
      _entries.erase(i);
    }
  }
};

} // namespace aria2

#endif // _D_DNS_CACHE_H_
