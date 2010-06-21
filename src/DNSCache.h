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
    std::string addr_;
    bool good_;

    AddrEntry(const std::string& addr):addr_(addr), good_(true) {}
  };

  struct CacheEntry {
    std::string hostname_;
    uint16_t port_;
    std::vector<AddrEntry> addrEntries_;

    CacheEntry
    (const std::string& hostname, uint16_t port):
      hostname_(hostname), port_(port) {}

    void add(const std::string& addr)
    {
      addrEntries_.push_back(AddrEntry(addr));
    }

    std::vector<AddrEntry>::iterator find(const std::string& addr)
    {
      for(std::vector<AddrEntry>::iterator i = addrEntries_.begin(),
            eoi = addrEntries_.end(); i != eoi; ++i) {
        if((*i).addr_ == addr) {
          return i;
        }
      }
      return addrEntries_.end();
    }

    std::vector<AddrEntry>::const_iterator find(const std::string& addr) const
    {
      for(std::vector<AddrEntry>::const_iterator i = addrEntries_.begin(),
            eoi = addrEntries_.end(); i != eoi; ++i) {
        if((*i).addr_ == addr) {
          return i;
        }
      }
      return addrEntries_.end();
    }

    bool contains(const std::string& addr) const
    {
      return find(addr) != addrEntries_.end();
    }

    const std::string& getGoodAddr() const
    {
      for(std::vector<AddrEntry>::const_iterator i = addrEntries_.begin(),
            eoi = addrEntries_.end(); i != eoi; ++i) {
        if((*i).good_) {
          return (*i).addr_;
        }
      }
      return A2STR::NIL;
    }

    template<typename OutputIterator>
    void getAllGoodAddrs(OutputIterator out) const
    {
      for(std::vector<AddrEntry>::const_iterator i = addrEntries_.begin(),
            eoi = addrEntries_.end(); i != eoi; ++i) {
        if((*i).good_) {
          *out++ = (*i).addr_;
        }
      }      
    }

    void markBad(const std::string& addr)
    {
      std::vector<AddrEntry>::iterator i = find(addr);
      if(i != addrEntries_.end()) {
        (*i).good_ = false;
      }
    }

    bool operator<(const CacheEntry& e) const
    {
      int r = hostname_.compare(e.hostname_);
      if(r != 0) {
        return r < 0;
      }
      return port_ < e.port_;
    }

    bool operator==(const CacheEntry& e) const
    {
      return hostname_ == e.hostname_ && port_ == e.port_;
    }
  };

  std::deque<CacheEntry> entries_;

public:
  const std::string& find(const std::string& hostname, uint16_t port) const
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::const_iterator i =
      std::lower_bound(entries_.begin(), entries_.end(), target);
    if(i != entries_.end() && (*i) == target) {
      return (*i).getGoodAddr();
    }
    return A2STR::NIL;
  }
  
  template<typename OutputIterator>
  void findAll
  (OutputIterator out, const std::string& hostname, uint16_t port) const
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::const_iterator i =
      std::lower_bound(entries_.begin(), entries_.end(), target);
    if(i != entries_.end() && (*i) == target) {
      (*i).getAllGoodAddrs(out);
    }
  }

  void put
  (const std::string& hostname, const std::string& ipaddr, uint16_t port)
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::iterator i =
      std::lower_bound(entries_.begin(), entries_.end(), target);
    if(i == entries_.end() || !((*i) == target)) {
      target.add(ipaddr);
      entries_.insert(i, target);
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
      std::lower_bound(entries_.begin(), entries_.end(), target);
    if(i != entries_.end() && (*i) == target) {
      (*i).markBad(ipaddr);
    }
  }

  void remove(const std::string& hostname, uint16_t port)
  {
    CacheEntry target(hostname, port);
    std::deque<CacheEntry>::iterator i =
      std::lower_bound(entries_.begin(), entries_.end(), target);
    if(i != entries_.end() && (*i) == target) {
      entries_.erase(i);
    }
  }
};

} // namespace aria2

#endif // _D_DNS_CACHE_H_
