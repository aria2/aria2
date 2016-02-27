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
#include "GroupId.h"

#include <cassert>

#include "util.h"

namespace aria2 {

std::set<a2_gid_t> GroupId::set_;

std::shared_ptr<GroupId> GroupId::create()
{
  a2_gid_t n;
  for (;;) {
    util::generateRandomData(reinterpret_cast<unsigned char*>(&n), sizeof(n));
    if (n != 0 && set_.count(n) == 0) {
      break;
    }
  }
  std::shared_ptr<GroupId> res(new GroupId(n));
  return res;
}

std::shared_ptr<GroupId> GroupId::import(a2_gid_t n)
{
  std::shared_ptr<GroupId> res;
  if (n == 0 || set_.count(n) != 0) {
    return res;
  }
  res.reset(new GroupId(n));
  return res;
}

void GroupId::clear() { set_.clear(); }

int GroupId::expandUnique(a2_gid_t& n, const char* hex)
{
  a2_gid_t p = 0;
  size_t i;
  for (i = 0; hex[i]; ++i) {
    unsigned int c = util::hexCharToUInt(hex[i]);
    if (c == 255) {
      return ERR_INVALID;
    }
    p <<= 4;
    p |= c;
  }
  if (i == 0 || i > sizeof(a2_gid_t) * 2) {
    return ERR_INVALID;
  }
  p <<= 64 - i * 4;
  a2_gid_t mask = UINT64_MAX - ((1LL << (64 - i * 4)) - 1);
  auto itr = set_.lower_bound(p);
  if (itr == set_.end()) {
    return ERR_NOT_FOUND;
  }
  if (p == ((*itr) & mask)) {
    n = *itr;
    ++itr;
    if (itr == set_.end() || p != ((*itr) & mask)) {
      return 0;
    }
    else {
      return ERR_NOT_UNIQUE;
    }
  }
  else {
    return ERR_NOT_FOUND;
  }
}

int GroupId::toNumericId(a2_gid_t& n, const char* hex)
{
  a2_gid_t p = 0;
  size_t i;
  for (i = 0; hex[i]; ++i) {
    unsigned int c = util::hexCharToUInt(hex[i]);
    if (c == 255) {
      return ERR_INVALID;
    }
    p <<= 4;
    p |= c;
  }
  if (p == 0 || i != sizeof(a2_gid_t) * 2) {
    return ERR_INVALID;
  }
  n = p;
  return 0;
}

std::string GroupId::toHex(a2_gid_t gid)
{
  a2_gid_t n = hton64(gid);
  return util::toHex(reinterpret_cast<unsigned char*>(&n), sizeof(n));
}

std::string GroupId::toAbbrevHex(a2_gid_t gid)
{
  const size_t abbrevSize = 6;
  std::string h = toHex(gid);
  assert(h.size() >= abbrevSize);
  return toHex(gid).erase(abbrevSize);
}

std::string GroupId::toHex() const { return toHex(gid_); }

std::string GroupId::toAbbrevHex() const { return toAbbrevHex(gid_); }

GroupId::GroupId(a2_gid_t gid) : gid_(gid) { set_.insert(gid_); }

GroupId::~GroupId() { set_.erase(gid_); }

} // namespace aria2
