/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "DNSCache.h"
#include "A2STR.h"

namespace aria2 {

DNSCache::AddrEntry::AddrEntry(const std::string& addr)
  : addr_(addr), good_(true)
{}

DNSCache::AddrEntry::AddrEntry(const AddrEntry& c)
  : addr_(c.addr_), good_(c.good_)
{}

DNSCache::AddrEntry::~AddrEntry() {}

DNSCache::AddrEntry& DNSCache::AddrEntry::operator=(const AddrEntry& c)
{
  if(this != &c) {
    addr_ = c.addr_;
    good_ = c.good_;
  }
  return *this;
}

DNSCache::CacheEntry::CacheEntry(const std::string& hostname, uint16_t port)
  : hostname_(hostname), port_(port)
{}

DNSCache::CacheEntry::CacheEntry(const CacheEntry& c)
  : hostname_(c.hostname_), port_(c.port_), addrEntries_(c.addrEntries_)
{}

DNSCache::CacheEntry::~CacheEntry() {}

DNSCache::CacheEntry& DNSCache::CacheEntry::operator=(const CacheEntry& c)
{
  if(this != &c) {
    hostname_ = c.hostname_;
    port_ = c.port_;
    addrEntries_ = c.addrEntries_;
  }
  return *this;
}

bool DNSCache::CacheEntry::add(const std::string& addr)
{
  for(std::vector<AddrEntry>::const_iterator i = addrEntries_.begin(),
        eoi = addrEntries_.end(); i != eoi; ++i) {
    if((*i).addr_ == addr) {
      return false;
    }
  }
  addrEntries_.push_back(AddrEntry(addr));
  return true;
}

std::vector<DNSCache::AddrEntry>::iterator DNSCache::CacheEntry::find
(const std::string& addr)
{
  for(std::vector<AddrEntry>::iterator i = addrEntries_.begin(),
        eoi = addrEntries_.end(); i != eoi; ++i) {
    if((*i).addr_ == addr) {
      return i;
    }
  }
  return addrEntries_.end();
}

std::vector<DNSCache::AddrEntry>::const_iterator DNSCache::CacheEntry::find
(const std::string& addr) const
{
  for(std::vector<AddrEntry>::const_iterator i = addrEntries_.begin(),
        eoi = addrEntries_.end(); i != eoi; ++i) {
    if((*i).addr_ == addr) {
      return i;
    }
  }
  return addrEntries_.end();
}

bool DNSCache::CacheEntry::contains(const std::string& addr) const
{
  return find(addr) != addrEntries_.end();
}

const std::string& DNSCache::CacheEntry::getGoodAddr() const
{
  for(std::vector<AddrEntry>::const_iterator i = addrEntries_.begin(),
        eoi = addrEntries_.end(); i != eoi; ++i) {
    if((*i).good_) {
      return (*i).addr_;
    }
  }
  return A2STR::NIL;
}

void DNSCache::CacheEntry::markBad(const std::string& addr)
{
  std::vector<AddrEntry>::iterator i = find(addr);
  if(i != addrEntries_.end()) {
    (*i).good_ = false;
  }
}

bool DNSCache::CacheEntry::operator<(const CacheEntry& e) const
{
  int r = hostname_.compare(e.hostname_);
  if(r != 0) {
    return r < 0;
  }
  return port_ < e.port_;
}

bool DNSCache::CacheEntry::operator==(const CacheEntry& e) const
{
  return hostname_ == e.hostname_ && port_ == e.port_;
}

DNSCache::DNSCache() {}

DNSCache::DNSCache(const DNSCache& c):entries_(c.entries_) {}

DNSCache::~DNSCache() {}

DNSCache& DNSCache::operator=(const DNSCache& c)
{
  if(this != &c) {
    entries_ = c.entries_;
  }
  return *this;
}

const std::string& DNSCache::find
(const std::string& hostname, uint16_t port) const
{
  SharedHandle<CacheEntry> target(new CacheEntry(hostname, port));
  CacheEntrySet::iterator i = entries_.find(target);
  if(i == entries_.end()) {
    return A2STR::NIL;
  } else {
    return (*i)->getGoodAddr();
  }
}

void DNSCache::put
(const std::string& hostname, const std::string& ipaddr, uint16_t port)
{
  SharedHandle<CacheEntry> target(new CacheEntry(hostname, port));
  CacheEntrySet::iterator i = entries_.lower_bound(target);
  if(i != entries_.end() && *(*i) == *target) {
    (*i)->add(ipaddr);
  } else {
    target->add(ipaddr);
    entries_.insert(i, target);
  }
}

void DNSCache::markBad
(const std::string& hostname, const std::string& ipaddr, uint16_t port)
{
  SharedHandle<CacheEntry> target(new CacheEntry(hostname, port));
  CacheEntrySet::iterator i = entries_.find(target);
  if(i != entries_.end()) {
    (*i)->markBad(ipaddr);
  }
}

void DNSCache::remove(const std::string& hostname, uint16_t port)
{
  SharedHandle<CacheEntry> target(new CacheEntry(hostname, port));
  entries_.erase(target);
}

} // namespace aria2
