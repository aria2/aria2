/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "ValueBase.h"

namespace aria2 {

const SharedHandle<ValueBase> ValueBase::none;

String::String(const ValueType& string):str_(string) {}

String::String(const char* cstring):str_(cstring) {}

String::String(const char* data, size_t length):str_(&data[0], &data[length]) {}

String::String(const unsigned char* data, size_t length):
  str_(&data[0], &data[length]) {}

String::String() {}

String::~String() {}

const String::ValueType& String::s() const
{
  return str_;
}

const unsigned char* String::uc() const
{
  return reinterpret_cast<const unsigned char*>(str_.data());
}

SharedHandle<String> String::g(const ValueType& string)
{
  return SharedHandle<String>(new String(string));
}

SharedHandle<String> String::g(const unsigned char* data, size_t length)
{
  return SharedHandle<String>(new String(data, length));
}

void String::accept(ValueBaseVisitor& v) const
{
  v.visit(*this);
}

Integer::Integer(ValueType integer):integer_(integer) {}

Integer::Integer():integer_(0) {}

Integer::~Integer() {}

Integer::ValueType Integer::i() const
{
  return integer_;
}

SharedHandle<Integer> Integer::g(ValueType integer)
{
  return SharedHandle<Integer>(new Integer(integer));
}

void Integer::accept(ValueBaseVisitor& v) const
{
  v.visit(*this);
}

const SharedHandle<Bool> Bool::trueValue_(new Bool(true));
const SharedHandle<Bool> Bool::falseValue_(new Bool(false));

SharedHandle<Bool> Bool::gTrue()
{
  return trueValue_;
}

SharedHandle<Bool> Bool::gFalse()
{
  return falseValue_;
}

bool Bool::val() const
{
  return val_;
}

void Bool::accept(ValueBaseVisitor& v) const
{
  v.visit(*this);
}

Bool::Bool(bool val):val_(val) {}

const SharedHandle<Null> Null::nullValue_(new Null());

SharedHandle<Null> Null::g()
{
  return nullValue_;
}

void Null::accept(ValueBaseVisitor& v) const
{
  v.visit(*this);
}

Null::Null() {}

List::List() {}

List::~List() {}

const SharedHandle<ValueBase>& List::get(size_t index) const
{
  return list_[index];
}

void List::set(size_t index, const SharedHandle<ValueBase>& v)
{
  list_[index] = v;
}

void List::append(const SharedHandle<ValueBase>& v)
{
  list_.push_back(v);
}

void List::append(const String::ValueType& string)
{
  list_.push_back(String::g(string));
}

List& List::operator<<(const SharedHandle<ValueBase>& v)
{
  list_.push_back(v);
  return *this;
}

const SharedHandle<ValueBase>& List::operator[](size_t index) const
{
  return list_[index];
}

List::ValueType::iterator List::begin()
{
  return list_.begin();
}

List::ValueType::iterator List::end()
{
  return list_.end();
}

List::ValueType::const_iterator List::begin() const
{
  return list_.begin();
}

List::ValueType::const_iterator List::end() const
{
  return list_.end();
}

size_t List::size() const
{
  return list_.size();
}

bool List::empty() const
{
  return list_.empty();
}

SharedHandle<List> List::g()
{
  return SharedHandle<List>(new List());
}

void List::accept(ValueBaseVisitor& v) const
{
  v.visit(*this);
}

Dict::Dict() {}

Dict::~Dict() {}

void Dict::put(const std::string& key, const SharedHandle<ValueBase>& vlb)
{
  ValueType::value_type p = std::make_pair(key, vlb);
  std::pair<ValueType::iterator, bool> r = dict_.insert(p);
  if(!r.second) {
    (*r.first).second = vlb;
  }
}

void Dict::put(const std::string& key, const String::ValueType& string)
{
  put(key, String::g(string));
}

const SharedHandle<ValueBase>& Dict::get(const std::string& key) const
{
  ValueType::const_iterator itr = dict_.find(key);
  if(itr == dict_.end()) {
    return ValueBase::none;
  } else {
    return (*itr).second;
  }
}

SharedHandle<ValueBase>& Dict::get(const std::string& key)
{
  ValueType::iterator itr = dict_.find(key);
  if(itr == dict_.end()) {
    return dict_[key];
  } else {
    return (*itr).second;
  }
}

SharedHandle<ValueBase>& Dict::operator[](const std::string& key)
{
  return get(key);
}

const SharedHandle<ValueBase>& Dict::operator[](const std::string& key) const
{
  return get(key);
}

bool Dict::containsKey(const std::string& key) const
{
  return dict_.count(key) == 1;
}

void Dict::removeKey(const std::string& key)
{
  dict_.erase(key);
}

Dict::ValueType::iterator Dict::begin()
{
  return dict_.begin();
}

Dict::ValueType::iterator Dict::end()
{
  return dict_.end();
}

Dict::ValueType::const_iterator Dict::begin() const
{
  return dict_.begin();
}

Dict::ValueType::const_iterator Dict::end() const
{
  return dict_.end();
}

size_t Dict::size() const
{
  return dict_.size();
}

bool Dict::empty() const
{
  return dict_.empty();
}

SharedHandle<Dict> Dict::g()
{
  return SharedHandle<Dict>(new Dict());
}
void Dict::accept(ValueBaseVisitor& v) const
{
  v.visit(*this);
}

} // namespace aria2
