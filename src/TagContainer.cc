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
#include "TagContainer.h"
#include "TaggedItem.h"
#include "Util.h"

TagContainer::TagContainer() {}

TagContainer::TagContainer(const TaggedItems& items):
  _taggedItems(items) {}

TagContainer::~TagContainer() {}

class SingleTagSearch {
private:
  string _tag;

  TaggedItems _result;
public:
  SingleTagSearch(const string& tag):_tag(tag) {}

  void operator()(const TaggedItemHandle& item)
  {
    if(item->hasTag(_tag)) {
      _result.push_back(item);
    }
  }

  const TaggedItems& getResult() const
  {
    return _result;
  }
};

TaggedItems TagContainer::search(const string& tag) const
{
  return for_each(_taggedItems.begin(), _taggedItems.end(), SingleTagSearch(tag)).getResult();
}

class NameMatchForward {
private:
  string _name;

  TaggedItems _result;
public:
  NameMatchForward(const string& name):_name(name) {}

  void operator()(const TaggedItemHandle& item)
  {
    if(Util::startsWith(item->getName(), _name)) {
      _result.push_back(item);
    }
  }

  const TaggedItems& getResult() const
  {
    return _result;
  }
};

TaggedItems TagContainer::nameMatchForward(const string& name) const
{
  return for_each(_taggedItems.begin(), _taggedItems.end(), NameMatchForward(name)).getResult();
}

const TaggedItems& TagContainer::getAllItems() const
{
  return _taggedItems;
}

void TagContainer::addItem(const TaggedItemHandle& item)
{
  _taggedItems.push_back(item);
}
