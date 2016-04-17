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

String::String(const ValueType& string) : str_{string} {}
String::String(ValueType&& string) : str_{std::move(string)} {}

String::String(const char* cstring) : str_{cstring} {}

String::String(const char* data, size_t length) : str_{&data[0], &data[length]}
{
}

String::String(const unsigned char* data, size_t length)
    : str_{&data[0], &data[length]}
{
}

String::String() {}

const String::ValueType& String::s() const { return str_; }

String::ValueType String::popValue() const { return std::move(str_); }

const unsigned char* String::uc() const
{
  return reinterpret_cast<const unsigned char*>(str_.data());
}

std::unique_ptr<String> String::g(const ValueType& string)
{
  return make_unique<String>(string);
}

std::unique_ptr<String> String::g(ValueType&& string)
{
  return make_unique<String>(std::move(string));
}

std::unique_ptr<String> String::g(const unsigned char* data, size_t length)
{
  return make_unique<String>(data, length);
}

void String::accept(ValueBaseVisitor& v) const { v.visit(*this); }

Integer::Integer(ValueType integer) : integer_{integer} {}

Integer::Integer() : integer_{0} {}

Integer::ValueType Integer::i() const { return integer_; }

std::unique_ptr<Integer> Integer::g(ValueType integer)
{
  return make_unique<Integer>(integer);
}

void Integer::accept(ValueBaseVisitor& v) const { v.visit(*this); }

Bool::Bool(bool val) : val_{val} {}

std::unique_ptr<Bool> Bool::gTrue() { return make_unique<Bool>(true); }

std::unique_ptr<Bool> Bool::gFalse() { return make_unique<Bool>(false); }

bool Bool::val() const { return val_; }

void Bool::accept(ValueBaseVisitor& v) const { v.visit(*this); }

Null::Null() {}

std::unique_ptr<Null> Null::g() { return make_unique<Null>(); }

void Null::accept(ValueBaseVisitor& v) const { v.visit(*this); }

List::List() {}

ValueBase* List::get(size_t index) const { return list_[index].get(); }

void List::set(size_t index, std::unique_ptr<ValueBase> v)
{
  list_[index] = std::move(v);
}

void List::pop_front() { list_.pop_front(); }

void List::pop_back() { list_.pop_back(); }

void List::append(std::unique_ptr<ValueBase> v)
{
  list_.push_back(std::move(v));
}

void List::append(String::ValueType string)
{
  list_.push_back(String::g(std::move(string)));
}

List& List::operator<<(std::unique_ptr<ValueBase> v)
{
  list_.push_back(std::move(v));
  return *this;
}

ValueBase* List::operator[](size_t index) const { return list_[index].get(); }

List::ValueType::iterator List::begin() { return list_.begin(); }

List::ValueType::iterator List::end() { return list_.end(); }

List::ValueType::const_iterator List::begin() const { return list_.begin(); }

List::ValueType::const_iterator List::end() const { return list_.end(); }

List::ValueType::const_iterator List::cbegin() const { return list_.cbegin(); }

List::ValueType::const_iterator List::cend() const { return list_.cend(); }

size_t List::size() const { return list_.size(); }

bool List::empty() const { return list_.empty(); }

std::unique_ptr<List> List::g() { return make_unique<List>(); }

void List::accept(ValueBaseVisitor& v) const { v.visit(*this); }

Dict::Dict() {}

void Dict::put(std::string key, std::unique_ptr<ValueBase> vlb)
{
  auto p = std::make_pair(std::move(key), std::move(vlb));
  auto r = dict_.insert(std::move(p));
  if (!r.second) {
    (*r.first).second = std::move(p.second);
  }
}

void Dict::put(std::string key, String::ValueType string)
{
  put(std::move(key), String::g(std::move(string)));
}

ValueBase* Dict::get(const std::string& key) const
{
  auto itr = dict_.find(key);
  if (itr == std::end(dict_)) {
    return nullptr;
  }
  else {
    return (*itr).second.get();
  }
}

ValueBase* Dict::operator[](const std::string& key) const { return get(key); }

bool Dict::containsKey(const std::string& key) const
{
  return dict_.count(key);
}

void Dict::removeKey(const std::string& key) { dict_.erase(key); }

std::unique_ptr<ValueBase> Dict::popValue(const std::string& key)
{
  auto i = dict_.find(key);
  if (i == std::end(dict_)) {
    return nullptr;
  }
  else {
    auto res = std::move((*i).second);
    dict_.erase(i);
    return res;
  }
}

Dict::ValueType::iterator Dict::begin() { return dict_.begin(); }

Dict::ValueType::iterator Dict::end() { return dict_.end(); }

Dict::ValueType::const_iterator Dict::begin() const { return dict_.begin(); }

Dict::ValueType::const_iterator Dict::end() const { return dict_.end(); }

Dict::ValueType::const_iterator Dict::cbegin() const { return dict_.cbegin(); }

Dict::ValueType::const_iterator Dict::cend() const { return dict_.cend(); }

size_t Dict::size() const { return dict_.size(); }

bool Dict::empty() const { return dict_.empty(); }

std::unique_ptr<Dict> Dict::g() { return make_unique<Dict>(); }

void Dict::accept(ValueBaseVisitor& v) const { v.visit(*this); }

} // namespace aria2
