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
#ifndef _D_DICTIONARY_H_
#define _D_DICTIONARY_H_

#include "MetaEntry.h"
#include <map>
#include <deque>
#include <string>

using namespace std;

typedef map<string, MetaEntry*> MetaTable;
typedef deque<string> Order;

class Dictionary : public MetaEntry {
private:
  MetaTable table;
  Order order;
  void clearTable();
public:
  Dictionary();
  ~Dictionary();

  const MetaEntry* get(const string& name) const;
  void put(const string& name, MetaEntry* entry);

  void accept(MetaEntryVisitor* v) const;
  const Order& getOrder() const;
  
};

#endif // _D_DICTIONARY_H_
