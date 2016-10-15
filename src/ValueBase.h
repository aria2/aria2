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
#ifndef D_VALUE_BASE_H
#define D_VALUE_BASE_H

#include "common.h"

#include <string>
#include <deque>
#include <map>
#include <memory>

#include "a2functional.h"

namespace aria2 {

class ValueBaseVisitor;

class ValueBase {
public:
  virtual ~ValueBase() = default;

  virtual void accept(ValueBaseVisitor& visitor) const = 0;
};

class String;
class Integer;
class Bool;
class Null;
class List;
class Dict;

class ValueBaseVisitor {
public:
  virtual ~ValueBaseVisitor() = default;
  virtual void visit(const String& string) = 0;
  virtual void visit(const Integer& integer) = 0;
  virtual void visit(const Bool& boolValue) = 0;
  virtual void visit(const Null& nullValue) = 0;
  virtual void visit(const List& list) = 0;
  virtual void visit(const Dict& dict) = 0;
};

class String : public ValueBase {
public:
  typedef std::string ValueType;

  String(const ValueType& string);
  String(ValueType&& string);

  explicit String(const char* cstring);

  String(const char* data, size_t length);

  String(const unsigned char* data, size_t length);

  template <typename InputIterator>
  String(InputIterator first, InputIterator last) : str_(first, last)
  {
  }

  String();

  // Don't allow copying
  String(const String&) = delete;
  String& operator=(const String&) = delete;

  const ValueType& s() const;

  ValueType popValue() const;

  // Returns std::string.data() cast to unsigned char*.
  // Use s().size() to get length.
  const unsigned char* uc() const;

  static std::unique_ptr<String> g(const ValueType& string);
  static std::unique_ptr<String> g(ValueType&& string);

  static std::unique_ptr<String> g(const unsigned char* data, size_t length);

  template <typename InputIterator>
  static std::unique_ptr<String> g(InputIterator first, InputIterator last)
  {
    return make_unique<String>(first, last);
  }

  virtual void accept(ValueBaseVisitor& visitor) const CXX11_OVERRIDE;

private:
  ValueType str_;
};

class Integer : public ValueBase {
public:
  typedef int64_t ValueType;

  Integer(ValueType integer);

  Integer();

  // Don't allow copying
  Integer(const Integer&) = delete;
  Integer& operator=(const Integer&) = delete;

  // Returns Integer.
  ValueType i() const;

  static std::unique_ptr<Integer> g(ValueType integer);

  virtual void accept(ValueBaseVisitor& visitor) const CXX11_OVERRIDE;

private:
  ValueType integer_;
};

class Bool : public ValueBase {
public:
  static std::unique_ptr<Bool> gTrue();
  static std::unique_ptr<Bool> gFalse();
  Bool(bool val);
  bool val() const;
  virtual void accept(ValueBaseVisitor& visitor) const CXX11_OVERRIDE;

private:
  // Don't allow copying
  Bool(const Bool&) = delete;
  Bool& operator=(const Bool&) = delete;
  bool val_;
};

class Null : public ValueBase {
public:
  static std::unique_ptr<Null> g();
  Null();
  virtual void accept(ValueBaseVisitor& visitor) const CXX11_OVERRIDE;

private:
  // Don't allow copying
  Null(const Null&);
  Null& operator=(const Null&);
};

class List : public ValueBase {
public:
  typedef std::deque<std::unique_ptr<ValueBase>> ValueType;

  List();

  // Don't allow copying
  List(const List&) = delete;
  List& operator=(const List&) = delete;

  // Appends given v to list.
  void append(std::unique_ptr<ValueBase> v);

  // Appending string is so common that we provide shortcut function.
  void append(String::ValueType string);

  // Alias for append()
  List& operator<<(std::unique_ptr<ValueBase> v);

  // Returns the object at given index.
  ValueBase* get(size_t index) const;

  // Set the object at given index.
  void set(size_t index, std::unique_ptr<ValueBase> v);

  // Returns the const reference of the object at the given index.
  ValueBase* operator[](size_t index) const;

  // Pops the value in the front of the list.
  void pop_front();

  // Pops the value in the back of the list.
  void pop_back();

  // Returns a read/write iterator that points to the first object in
  // list.
  ValueType::iterator begin();

  // Returns a read/write iterator that points to the one past the
  // last object in list.
  ValueType::iterator end();

  // Returns a read/write read-only iterator that points to the first
  // object in list.
  ValueType::const_iterator begin() const;

  // Returns a read/write read-only iterator that points to the one
  // past the last object in list.
  ValueType::const_iterator end() const;

  // Returns a read/write read-only iterator that points to the first
  // object in list.
  ValueType::const_iterator cbegin() const;

  // Returns a read/write read-only iterator that points to the one
  // past the last object in list.
  ValueType::const_iterator cend() const;

  // Returns size of list.
  size_t size() const;

  // Returns true if size of list is 0.
  bool empty() const;

  static std::unique_ptr<List> g();

  virtual void accept(ValueBaseVisitor& visitor) const CXX11_OVERRIDE;

private:
  ValueType list_;
};

class Dict : public ValueBase {
public:
  typedef std::map<std::string, std::unique_ptr<ValueBase>> ValueType;

  Dict();

  // Don't allow copying
  Dict(const Dict&) = delete;
  Dict& operator=(const Dict&) = delete;

  void put(std::string key, std::unique_ptr<ValueBase> vlb);

  // Putting string is so common that we provide shortcut function.
  void put(std::string key, String::ValueType string);

  ValueBase* get(const std::string& key) const;

  // Returns the reference to object associated with given key.  If
  // the key is not found, nullptr is returned.
  ValueBase* operator[](const std::string& key) const;

  // Returns true if the given key is found in dict.
  bool containsKey(const std::string& key) const;

  // Removes specified key from dict.
  void removeKey(const std::string& key);

  // Removes specified key from dict and return its associated value.
  std::unique_ptr<ValueBase> popValue(const std::string& key);

  // Returns a read/write iterator that points to the first pair in
  // the dict.
  ValueType::iterator begin();

  // Returns a read/write read-only iterator that points to one past
  // the last pair in the dict.
  ValueType::iterator end();

  // Returns a read/write read-only iterator that points to the first
  // pair in the dict.
  ValueType::const_iterator begin() const;

  // Returns a read/write read-only iterator that points to one past
  // the last pair in the dict.
  ValueType::const_iterator end() const;

  // Returns a read/write read-only iterator that points to the first
  // pair in the dict.
  ValueType::const_iterator cbegin() const;

  // Returns a read/write read-only iterator that points to one past
  // the last pair in the dict.
  ValueType::const_iterator cend() const;

  // Returns size of Dict.
  size_t size() const;

  // Returns true if size of Dict is 0.
  bool empty() const;

  static std::unique_ptr<Dict> g();

  virtual void accept(ValueBaseVisitor& visitor) const CXX11_OVERRIDE;

private:
  ValueType dict_;
};

class EmptyDowncastValueBaseVisitor : public ValueBaseVisitor {
public:
  EmptyDowncastValueBaseVisitor() {}
  virtual void visit(const String& v) CXX11_OVERRIDE {}
  virtual void visit(const Integer& v) CXX11_OVERRIDE {}
  virtual void visit(const Bool& v) CXX11_OVERRIDE {}
  virtual void visit(const Null& v) CXX11_OVERRIDE {}
  virtual void visit(const List& v) CXX11_OVERRIDE {}
  virtual void visit(const Dict& v) CXX11_OVERRIDE {}
};

template <typename T>
class DowncastValueBaseVisitor : public EmptyDowncastValueBaseVisitor {
public:
  DowncastValueBaseVisitor() : result_{nullptr} {}

  virtual void visit(const T& t) { result_ = &t; }

  const T* getResult() const { return result_; }

  void setResult(const T* r) { result_ = r; }

private:
  const T* result_;
};

template <typename T, typename VPtr> T* downcast(const VPtr& v)
{
  if (v) {
    DowncastValueBaseVisitor<T> visitor;
    v->accept(visitor);
    return const_cast<T*>(visitor.getResult());
  }
  else {
    return 0;
  }
}

} // namespace aria2

#endif // D_VALUE_BASE_H
