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
#ifndef D_GROUP_ID_H
#define D_GROUP_ID_H

#include "common.h"

#include <set>
#include <string>
#include <memory>

namespace aria2 {

typedef uint64_t a2_gid_t;

class GroupId {
public:
  static std::shared_ptr<GroupId> create();
  static std::shared_ptr<GroupId> import(a2_gid_t n);
  static void clear();
  enum { ERR_NOT_UNIQUE = -1, ERR_NOT_FOUND = -2, ERR_INVALID = -3 };
  static int expandUnique(a2_gid_t& n, const char* hex);
  static int toNumericId(a2_gid_t& n, const char* hex);
  static std::string toHex(a2_gid_t n);
  static std::string toAbbrevHex(a2_gid_t n);

  ~GroupId();
  a2_gid_t getNumericId() const { return gid_; }
  std::string toHex() const;
  std::string toAbbrevHex() const;

private:
  static std::set<a2_gid_t> set_;

  GroupId(a2_gid_t gid);

  a2_gid_t gid_;
};

} // namespace aria2

#endif // D_GROUP_ID_H
