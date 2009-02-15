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
#ifndef _D_BENCODE_H_
#define _D_BENCODE_H_

#include "common.h"

#include <string>
#include <map>
#include <deque>
#include <iosfwd>

#include "SharedHandle.h"
#include "A2STR.h"
#include "RecoverableException.h"

namespace aria2 {

namespace bencode {

class BDE;

class BDE {
public:
  typedef std::map<std::string, BDE> Dict;
  typedef std::deque<BDE> List;
  typedef int64_t Integer;
private:
  enum TYPE{
    TYPE_NONE,
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_DICT,
    TYPE_LIST,
  };

  class BObject {
  public:
    virtual ~BObject() {}

    ////////////////////////////////////////////////////////////////////////////
    // Integer Interface

    // Returns Integer.
    virtual Integer i() const
    {
      throw RecoverableException("Not Integer");
    }

    ////////////////////////////////////////////////////////////////////////////
    // String Interface

    // Returns std::string.
    virtual const std::string& s() const
    {
      throw RecoverableException("Not String");
    } 

    // Returns std::string.data() casted to unsigned char*.
    // Use s().size() to get length.
    virtual const unsigned char* uc() const
    {
      throw RecoverableException("Not String");
    }

    ////////////////////////////////////////////////////////////////////////////
    // Dictionary Interface

    // Returns the reference to BDE object associated with given key.
    // If the key is not found, new pair with that key is created
    // using default values, which is then returned. In other words,
    // this is the same behavior of std::map's operator[].
    virtual BDE& operator[](const std::string& key)
    {
      throw RecoverableException("Not Dict");
    }

    // Returns true if the given key is found in dict.
    virtual bool containsKey(const std::string& key) const
    {
      throw RecoverableException("Not Dict");
    }

    // Removes specified key from dict.
    virtual void removeKey(const std::string& key)
    {
      throw RecoverableException("Not Dict");
    }

    // Returns a read/write iterator that points to the first pair in
    // the dict.
    virtual Dict::iterator dictBegin()
    {
      throw RecoverableException("Not Dict");
    }

    // Returns a read/write read-only iterator that points to one past
    // the last pair in the dict.
    virtual Dict::iterator dictEnd()
    {
      throw RecoverableException("Not Dict");
    }

    ////////////////////////////////////////////////////////////////////////////
    // List Interface
    
    // Appends given bde to list.
    virtual void append(const BDE& bde)
    {
      throw RecoverableException("Not List");
    }

    // Alias for append()
    virtual void operator<<(const BDE& bde)
    {
      throw RecoverableException("Not List");
    }

    // Returns the reference of the object at the given index.
    virtual BDE& operator[](size_t index)
    {
      throw RecoverableException("Not List");
    }

    // Returns a read/write iterator that points to the first object
    // in list.
    virtual List::iterator listBegin()
    {
      throw RecoverableException("Not List");
    }

    // Returns a read/write iterator that points to the one past the
    // last object in list.
    virtual List::iterator listEnd()
    {
      throw RecoverableException("Not List");
    }
    
    // Returns size of list or dict.
    virtual size_t size() const
    {
      throw RecoverableException("Neither Dict nor List");
    }

    // Returns true if size of list or dict is 0.
    virtual bool empty() const
    {
      throw RecoverableException("Neither Dict nor List");
    }
  };

  class BInteger : public BObject {
  private:
    Integer _integer;
  public:
    BInteger(Integer i):_integer(i) {}

    virtual BDE::Integer i() const
    {
      return _integer;
    }
  };

  class BString : public BObject {
  private:
    std::string _string;
  public:
    BString(const std::string& string):_string(string) {}

    virtual const std::string& s() const
    {
      return _string;
    }

    virtual const unsigned char* uc() const
    {
      return reinterpret_cast<const unsigned char*>(_string.data());
    }
  };

  class BDict : public BObject {
  private:
    Dict _dict;
  public:

    virtual BDE& operator[](const std::string& key)
    {
      return _dict[key];
    }

    virtual bool containsKey(const std::string& key) const
    {
      return _dict.find(key) != _dict.end();
    }

    virtual void removeKey(const std::string& key)
    {
      _dict.erase(key);
    }

    virtual BDE::Dict::iterator dictBegin()
    {
      return _dict.begin();
    }

    virtual BDE::Dict::iterator dictEnd()
    {
      return _dict.end();
    }

    virtual size_t size() const
    {
      return _dict.size();
    }

    virtual bool empty() const
    {
      return _dict.empty();
    }
  };

  class BList : public BObject {
  private:
    List _list;
  public:
    virtual void append(const BDE& bde)
    {
      _list.push_back(bde);
    }

    virtual void operator<<(const BDE& bde)
    {
      _list.push_back(bde);
    }

    virtual BDE& operator[](size_t index)
    {
      return _list[index];
    }

    virtual BDE::List::iterator listBegin()
    {
      return _list.begin();
    }

    virtual BDE::List::iterator listEnd()
    {
      return _list.end();
    }

    virtual size_t size() const
    {
      return _list.size();
    }

    virtual bool empty() const
    {
      return _list.empty();
    }
  };

  TYPE _type;

  SharedHandle<BObject> _bobject;
public:
  BDE();

  static BDE dict();

  static BDE list();

  static const BDE none;

  // Test for Null data
  // Return true if the type of this object is None.
  bool isNone() const;

  //////////////////////////////////////////////////////////////////////////////
  // Integer Interface

  BDE(Integer integer);

  // Returns true if the type of this object is Integer.
  bool isInteger() const;

  // Returns Integer. Requires this object to be Integer.
  Integer i() const;

  //////////////////////////////////////////////////////////////////////////////
  // String Interface

  BDE(const std::string& string);

  // Made explicit to avoid ambiguity with BDE(Integer).
  explicit BDE(const char* cstring);

  BDE(const char* data, size_t length);

  BDE(const unsigned char* data, size_t length);

  // Returns true if the type of this object is String.
  bool isString() const;

  // Returns std::string. Requires this object to be String
  const std::string& s() const;

  // Returns std::string.data() casted to unsigned char*.
  // Use s().size() to get length.
  const unsigned char* uc() const;

  //////////////////////////////////////////////////////////////////////////////
  // Dictionary Interface

  // Returns true if the type of this object is Dict.
  bool isDict() const;

  // Returns the reference to BDE object associated with given key.
  // If the key is not found, new pair with that key is created using default
  // values, which is then returned. In other words, this is the same behavior
  // of std::map's operator[].
  // Requires this object to be Dict.
  BDE& operator[](const std::string& key);

  // Returns the const reference to BDE ojbect associated with given key.
  // If the key is not found, BDE::none is returned.
  // Requires this object to be Dict.
  const BDE& operator[](const std::string& key) const;

  // Returns true if the given key is found in dict.
  // Requires this object to be Dict.
  bool containsKey(const std::string& key) const;

  // Removes specified key from dict.
  // Requires this object to be Dict.
  void removeKey(const std::string& key);

  // Returns a read/write iterator that points to the first pair in the dict.
  // Requires this object to be Dict.
  Dict::iterator dictBegin();

  // Returns a read/write read-only iterator that points to the first pair in
  // the dict.
  // Requires this object to be Dict.
  Dict::const_iterator dictBegin() const;

  // Returns a read/write read-only iterator that points to one past the last
  // pair in the dict.
  // Requires this object to be Dict.
  Dict::iterator dictEnd();

  // Returns a read/write read-only iterator that points to one past the last
  // pair in the dict.
  // Requires this object to be Dict.
  Dict::const_iterator dictEnd() const;

  //////////////////////////////////////////////////////////////////////////////
  // List Interface

  // Returns true if the type of this object is List.
  bool isList() const;

  // Appends given bde to list. Required the type of this object to be List.
  void append(const BDE& bde);

  // Alias for append()
  void operator<<(const BDE& bde);

  // Returns the reference of the object at the given index. Required this
  // object to be List.
  BDE& operator[](size_t index);

  // Returns the const reference of the object at the given index.
  // Required this object to be List.
  const BDE& operator[](size_t index) const;

  // Returns a read/write iterator that points to the first object in list.
  // Required this object to be List.
  List::iterator listBegin();

  // Returns a read/write read-only iterator that points to the first object
  // in list. Required this object to be List.
  List::const_iterator listBegin() const;

  // Returns a read/write iterator that points to the one past the last object
  // in list. Required this object to be List.
  List::iterator listEnd();

  // Returns a read/write read-only iterator that points to the one past the
  // last object in list. Required this object to be List.
  List::const_iterator listEnd() const;

  // For List type: Returns size of list.
  // For Dict type: Returns size of dict.
  size_t size() const;

  // For List type: Returns true if size of list is 0.
  // For Dict type: Returns true if size of dict is 0.
  bool empty() const;
};

BDE decode(std::istream& in);

// Decode the data in s.
BDE decode(const std::string& s);

BDE decode(const unsigned char* data, size_t length);

BDE decodeFromFile(const std::string& filename);

std::string encode(const BDE& bde);

} // namespace bencode

} // namespace aria2

#endif // _D_BENCODE_H_
