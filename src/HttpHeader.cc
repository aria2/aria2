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
#include "HttpHeader.h"

void HttpHeader::put(const string& name, const string& value) {
  multimap<string, string>::value_type vt(name, value);
  table.insert(vt);
}

bool HttpHeader::defined(const string& name) const {
  return table.count(name) == 1;
}

string HttpHeader::getFirst(const string& name) const {
  multimap<string, string>::const_iterator itr = table.find(name);
  if(itr == table.end()) {
    return "";
  } else {
    return (*itr).second;
  }
}

vector<string> HttpHeader::get(const string& name) const {
  vector<string> v;
  for(multimap<string, string>::const_iterator itr = table.find(name); itr != table.end(); itr++) {
    v.push_back((*itr).second);
  }
  return v;
}

int HttpHeader::getFirstAsInt(const string& name) const {
  multimap<string, string>::const_iterator itr = table.find(name);
  if(itr == table.end()) {
    return 0;
  } else {
    return (int)strtol((*itr).second.c_str(), NULL, 10);
  }
}

long long int HttpHeader::getFirstAsLLInt(const string& name) const {
  multimap<string, string>::const_iterator itr = table.find(name);
  if(itr == table.end()) {
    return 0;
  } else {
    return strtoll((*itr).second.c_str(), NULL, 10);
  }
}
