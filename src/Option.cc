/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "Option.h"
#include "prefs.h"

Option::Option() {}

Option::~Option() {}

void Option::put(const string& name, const string& value) {
  table[name] = value;
}

bool Option::defined(const string& name) const {
  return table.count(name) == 1;
}

string Option::get(const string& name) const {
  map<string, string>::const_iterator itr = table.find(name);
  if(itr == table.end()) {
    return "";
  } else {
    return (*itr).second;
  }
}

int Option::getAsInt(const string& name) const {
  string value = get(name);
  if(value == "") {
    return 0;
  } else {
    return (int)strtol(value.c_str(), NULL, 10);
  }
}

long long int Option::getAsLLInt(const string& name) const {
  string value = get(name);
  if(value == "") {
    return 0;
  } else {
    return strtoll(value.c_str(), NULL, 10);
  }
}

bool Option::getAsBool(const string& name) const {
  string value = get(name);
  if(value == V_TRUE) {
    return true;
  } else {
    return false;
  }
}

double Option::getAsDouble(const string& name) const {
  string value = get(name);
  if(value == "") {
    return 0.0;
  } else {
    return strtod(value.c_str(), 0);
  }
}
