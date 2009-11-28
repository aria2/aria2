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
#include "Option.h"
#include "prefs.h"
#include "A2STR.h"
#include <cstdlib>
#include <cstring>

namespace aria2 {

Option::Option() {}

Option::~Option() {}

void Option::put(const std::string& name, const std::string& value) {
  table[name] = value;
}

bool Option::defined(const std::string& name) const
{
  return table.count(name) == 1;
}

bool Option::blank(const std::string& name) const
{
  std::map<std::string, std::string>::const_iterator i = table.find(name);
  return i == table.end() || (*i).second.empty();
}

const std::string& Option::get(const std::string& name) const {
  std::map<std::string, std::string>::const_iterator itr = table.find(name);
  if(itr == table.end()) {
    return A2STR::NIL;
  } else {
    return (*itr).second;
  }
}

int32_t Option::getAsInt(const std::string& name) const {
  const std::string& value = get(name);
  if(value.empty()) {
    return 0;
  } else {
    return strtol(value.c_str(), 0, 10);
  }
}

int64_t Option::getAsLLInt(const std::string& name) const {
  const std::string& value = get(name);
  if(value.empty()) {
    return 0;
  } else {
    return strtoll(value.c_str(), 0, 10);
  }
}

bool Option::getAsBool(const std::string& name) const {
  return get(name) == V_TRUE;
}

double Option::getAsDouble(const std::string& name) const {
  const std::string& value = get(name);
  if(value.empty()) {
    return 0.0;
  } else {
    return strtod(value.c_str(), 0);
  }
}

void Option::remove(const std::string& name)
{
  std::map<std::string, std::string>::iterator i = table.find(name);
  if(i != table.end()) {
    table.erase(i);
  }
}

void Option::clear()
{
  table.clear();
}

} // namespace aria2
