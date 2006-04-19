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
#include "Dictionary.h"
#include "MetaEntryVisitor.h"

Dictionary::Dictionary() {}

Dictionary::~Dictionary() {
  clearTable();
}

void Dictionary::clearTable() {
  for(MetaTable::iterator itr = table.begin(); itr != table.end(); itr++) {
    delete itr->second;
  }
}

const MetaEntry* Dictionary::get(const string& name) const {
  MetaTable::const_iterator itr = table.find(name);
  if(itr == table.end()) {
    return NULL;
  } else {
    return itr->second;
  }
}

void Dictionary::put(const string& name, MetaEntry* entry) {
  table[name] = entry;
  order.push_back(name);
}

void Dictionary::accept(MetaEntryVisitor* v) const {
  v->visit(this);
}

const Order& Dictionary::getOrder() const {
  return order;
}
