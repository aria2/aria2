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
#include "Option.h"

#include <cstdlib>
#include <cstring>

#include "bitfield.h"

namespace aria2 {

Option::Option()
    : table_(option::countOption()), use_((option::countOption() + 7) / 8)
{
}

Option::~Option() = default;

Option::Option(const Option& option) = default;

Option& Option::operator=(const Option& option)
{
  if (this != &option) {
    table_ = option.table_;
    use_ = option.use_;
    parent_ = option.parent_;
  }
  return *this;
}

namespace {

template <typename V> void setBit(V& b, PrefPtr pref)
{
  b[pref->i / 8] |= 128 >> (pref->i % 8);
}

template <typename V> void unsetBit(V& b, PrefPtr pref)
{
  b[pref->i / 8] &= ~(128 >> (pref->i % 8));
}

} // namespace

void Option::put(PrefPtr pref, const std::string& value)
{
  setBit(use_, pref);
  table_[pref->i] = value;
}

bool Option::defined(PrefPtr pref) const
{
  return bitfield::test(use_, use_.size() * 8, pref->i) ||
         (parent_ && parent_->defined(pref));
}

bool Option::definedLocal(PrefPtr pref) const
{
  return bitfield::test(use_, use_.size() * 8, pref->i);
}

bool Option::blank(PrefPtr pref) const
{
  if (bitfield::test(use_, use_.size() * 8, pref->i)) {
    return table_[pref->i].empty();
  }
  else {
    return !parent_ || parent_->blank(pref);
  }
}

const std::string& Option::get(PrefPtr pref) const
{
  if (bitfield::test(use_, use_.size() * 8, pref->i)) {
    return table_[pref->i];
  }
  else if (parent_) {
    return parent_->get(pref);
  }
  else {
    return A2STR::NIL;
  }
}

int32_t Option::getAsInt(PrefPtr pref) const
{
  const std::string& value = get(pref);
  if (value.empty()) {
    return 0;
  }
  else {
    return strtol(value.c_str(), nullptr, 10);
  }
}

int64_t Option::getAsLLInt(PrefPtr pref) const
{
  const std::string& value = get(pref);
  if (value.empty()) {
    return 0;
  }
  else {
    return strtoll(value.c_str(), nullptr, 10);
  }
}

bool Option::getAsBool(PrefPtr pref) const { return get(pref) == A2_V_TRUE; }

double Option::getAsDouble(PrefPtr pref) const
{
  const std::string& value = get(pref);
  if (value.empty()) {
    return 0.0;
  }
  else {
    return strtod(value.c_str(), nullptr);
  }
}

void Option::removeLocal(PrefPtr pref)
{
  unsetBit(use_, pref);
  table_[pref->i].clear();
}

void Option::remove(PrefPtr pref)
{
  removeLocal(pref);
  if (parent_) {
    parent_->remove(pref);
  }
}

void Option::clear()
{
  std::fill(use_.begin(), use_.end(), 0);
  std::fill(table_.begin(), table_.end(), "");
}

void Option::merge(const Option& option)
{
  size_t bits = option.use_.size() * 8;
  for (size_t i = 1, len = table_.size(); i < len; ++i) {
    if (bitfield::test(option.use_, bits, i)) {
      use_[i / 8] |= 128 >> (i % 8);
      table_[i] = option.table_[i];
    }
  }
}

void Option::setParent(const std::shared_ptr<Option>& parent)
{
  parent_ = parent;
}

const std::shared_ptr<Option>& Option::getParent() const { return parent_; }

bool Option::emptyLocal() const
{
  size_t dst;
  return !bitfield::getFirstSetBitIndex(dst, use_, use_.size() * 8);
}

} // namespace aria2
