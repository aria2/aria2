/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "WrDiskCache.h"

#include <cassert>

#include "WrDiskCacheEntry.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

WrDiskCache::WrDiskCache(size_t limit) : limit_(limit), total_(0), clock_(0) {}

WrDiskCache::~WrDiskCache()
{
  if (total_) {
    A2_LOG_WARN(fmt("Write disk cache is not empty size=%lu",
                    static_cast<unsigned long>(total_)));
  }
}

bool WrDiskCache::add(WrDiskCacheEntry* ent)
{
  ent->setSizeKey(ent->getSize());
  ent->setLastUpdate(++clock_);
  std::pair<EntrySet::iterator, bool> rv = set_.insert(ent);
  if (rv.second) {
    total_ += ent->getSize();
    ensureLimit();
    return true;
  }
  else {
    A2_LOG_WARN(fmt("Found duplicate cache entry a.{size=%lu,clock=%" PRId64
                    "} b{size=%lu,clock=%" PRId64 "}",
                    static_cast<unsigned long>((*rv.first)->getSize()),
                    (*rv.first)->getLastUpdate(),
                    static_cast<unsigned long>(ent->getSize()),
                    ent->getLastUpdate()));
    return false;
  }
}

bool WrDiskCache::remove(WrDiskCacheEntry* ent)
{
  if (set_.erase(ent)) {
    A2_LOG_DEBUG(fmt("Removed cache entry size=%lu, clock=%" PRId64,
                     static_cast<unsigned long>(ent->getSize()),
                     ent->getLastUpdate()));
    total_ -= ent->getSize();
    return true;
  }
  else {
    return false;
  }
}

bool WrDiskCache::update(WrDiskCacheEntry* ent, ssize_t delta)
{
  if (!set_.erase(ent)) {
    return false;
  }
  A2_LOG_DEBUG(fmt("Update cache entry size=%lu, delta=%ld, clock=%" PRId64,
                   static_cast<unsigned long>(ent->getSize()),
                   static_cast<long>(delta), ent->getLastUpdate()));

  ent->setSizeKey(ent->getSize());
  ent->setLastUpdate(++clock_);
  set_.insert(ent);

  if (delta < 0) {
    assert(total_ >= static_cast<size_t>(-delta));
  }
  total_ += delta;
  ensureLimit();
  return true;
}

void WrDiskCache::ensureLimit()
{
  while (total_ > limit_) {
    auto i = set_.begin();
    WrDiskCacheEntry* ent = *i;
    A2_LOG_DEBUG(fmt("Force flush cache entry size=%lu, clock=%" PRId64,
                     static_cast<unsigned long>(ent->getSizeKey()),
                     ent->getLastUpdate()));
    total_ -= ent->getSize();
    ent->writeToDisk();
    set_.erase(i);

    ent->setSizeKey(ent->getSize());
    ent->setLastUpdate(++clock_);
    set_.insert(ent);
  }
}

} // namespace aria2
