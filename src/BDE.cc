/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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

#include "BDE.h"

namespace aria2 {

const BDE BDE::none;

BDE::BDE():_type(TYPE_NONE) {}

BDE::BDE(Integer integer):_type(TYPE_INTEGER),
			  _bobject(new BInteger(integer)) {}


BDE::BDE(const std::string& string):_type(TYPE_STRING),
				    _bobject(new BString(std::string(string))) {}

BDE::BDE(const char* cstring):_type(TYPE_STRING),
			      _bobject(new BString(std::string(cstring))) {}

BDE::BDE(const char* data, size_t length):
  _type(TYPE_STRING),
  _bobject(new BString(std::string(&data[0], &data[length]))) {}

BDE::BDE(const unsigned char* data, size_t length):
  _type(TYPE_STRING),
  _bobject(new BString(std::string(&data[0], &data[length]))) {}

BDE BDE::dict()
{
  BDE bde;
  bde._type = TYPE_DICT;
  bde._bobject.reset(new BDict());
  return bde;
}

BDE BDE::list()
{
  BDE bde;
  bde._type = TYPE_LIST;
  bde._bobject.reset(new BList());
  return bde;
}

// Test for Null data
bool BDE::isNone() const
{
  return _type == TYPE_NONE;
}

// Integer Interface

bool BDE::isInteger() const
{
  return _type == TYPE_INTEGER;
}

BDE::Integer BDE::i() const
{
  return _bobject->i();
}

// String Interface

bool BDE::isString() const
{
  return _type == TYPE_STRING;
}

const std::string& BDE::s() const
{
  return _bobject->s();
}

const unsigned char* BDE::uc() const
{
  return _bobject->uc();
}

// Dictionary Interface

bool BDE::isDict() const
{
  return _type == TYPE_DICT;
}

BDE& BDE::operator[](const std::string& key)
{
  return _bobject->operator[](key);
}

const BDE& BDE::operator[](const std::string& key) const
{
  if(_bobject->containsKey(key)) {
    return _bobject->operator[](key);
  } else {
    return none;
  }
}

bool BDE::containsKey(const std::string& key) const
{
  return _bobject->containsKey(key);
}

void BDE::removeKey(const std::string& key)
{
  _bobject->removeKey(key);
}

BDE::Dict::iterator BDE::dictBegin()
{
  return _bobject->dictBegin();
}

BDE::Dict::const_iterator BDE::dictBegin() const
{
  return _bobject->dictBegin();
}

BDE::Dict::iterator BDE::dictEnd()
{
  return _bobject->dictEnd();
}

BDE::Dict::const_iterator BDE::dictEnd() const
{
  return _bobject->dictEnd();
}

// List Interface

bool BDE::isList() const
{
  return _type == TYPE_LIST;
}

void BDE::append(const BDE& bde)
{
  _bobject->append(bde);
}

void BDE::operator<<(const BDE& bde)
{
  _bobject->operator<<(bde);
}

BDE& BDE::operator[](size_t index)
{
  return _bobject->operator[](index);
}

const BDE& BDE::operator[](size_t index) const
{
  return _bobject->operator[](index);
}

BDE::List::iterator BDE::listBegin()
{
  return _bobject->listBegin();
}

BDE::List::const_iterator BDE::listBegin() const
{
  return _bobject->listBegin();
}

BDE::List::iterator BDE::listEnd()
{
  return _bobject->listEnd();
}

BDE::List::const_iterator BDE::listEnd() const
{
  return _bobject->listEnd();
}

// Callable from List and Dict
size_t BDE::size() const
{
  return _bobject->size();
}

// Callable from List and Dict
bool BDE::empty() const
{
  return _bobject->empty();
}

} // namespace aria2
