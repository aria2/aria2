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
#include "TaggedItem.h"
#include <numeric>
#include <algorithm>

namespace aria2 {

class Concat {
private:
  std::string _delim;
public:
  Concat(const std::string& delim = ""):_delim(delim) {}

  std::string operator()(const std::string& s1, const std::string& s2) const
  {
    return s1+_delim+s2;
  }
};

std::string TaggedItem::toTagString() const
{
  if(_tags.size()) {
    return std::accumulate(_tags.begin()+1, _tags.end(), _tags.front(), Concat(","));
  } else {
    return "";
  }
}

bool TaggedItem::hasTag(const std::string& tag) const
{
  return std::find(_tags.begin(), _tags.end(), tag) != _tags.end();
}

bool TaggedItem::operator<(const TaggedItem& item) const
{
  return _name < item._name;
}

bool TaggedItem::operator==(const TaggedItem& item) const
{
  return _name == item._name;
}

std::string TaggedItem::toString() const
{
  return toTagString();
}

std::ostream&
operator<<(std::ostream& o, const TaggedItem& item)
{
  o << item.toString();
  return o;
}

std::ostream&
operator<<(std::ostream& o, const SharedHandle<TaggedItem>& item)
{
  o << (*item.get()).toString();
  return o;
}

} // namespace aria2
