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
#include "bencode2.h"

#include <sstream>

#include "fmt.h"
#include "DlAbortEx.h"
#include "error_code.h"
#include "BufferedFile.h"
#include "util.h"

namespace aria2 {

namespace bencode2 {

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeiter
(InputIterator first,
 InputIterator last,
 size_t depth);
} // namespace

namespace {
template<typename InputIterator>
std::pair<std::pair<InputIterator, InputIterator>, InputIterator>
decoderawstring(InputIterator first, InputIterator last)
{
  InputIterator i = first;
  int32_t len;
  for(; i != last && *i != ':'; ++i);
  if(i == last || i == first || !util::parseIntNoThrow(len, first, i) ||
     len < 0) {
    throw DL_ABORT_EX2("Bencode decoding failed:"
                       " A positive integer expected but none found.",
                       error_code::BENCODE_PARSE_ERROR);
  }
  ++i;
  if(last-i < len) {
    throw DL_ABORT_EX2
      (fmt("Bencode decoding failed:"
           " Expected %d bytes of data, but only %d read.",
           len, static_cast<int>(last-i)),
       error_code::BENCODE_PARSE_ERROR);
  }
  return std::make_pair(std::make_pair(i, i+len), i+len);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodestring(InputIterator first, InputIterator last)
{
  std::pair<std::pair<InputIterator, InputIterator>, InputIterator> r =
    decoderawstring(first, last);
  return std::make_pair(String::g(r.first.first, r.first.second), r.second);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeinteger(InputIterator first, InputIterator last)
{
  InputIterator i = first;
  for(; i != last && *i != 'e'; ++i);
  Integer::ValueType iv;
  if(i == last || !util::parseLLIntNoThrow(iv, first, i)) {
    throw DL_ABORT_EX2("Bencode decoding failed:"
                       " Integer expected but none found",
                       error_code::BENCODE_PARSE_ERROR);
  }
  return std::make_pair(Integer::g(iv), ++i);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodedict
(InputIterator first,
 InputIterator last,
 size_t depth)
{
  SharedHandle<Dict> dict = Dict::g();
  while(first != last) {
    if(*first == 'e') {
      return std::make_pair(dict, ++first);
    } else {
      std::pair<std::pair<InputIterator, InputIterator>, InputIterator> keyp =
        decoderawstring(first, last);
      std::pair<SharedHandle<ValueBase>, InputIterator> r =
        decodeiter(keyp.second, last, depth);
      dict->put(std::string(keyp.first.first, keyp.first.second), r.first);
      first = r.second;
    }
  }
  throw DL_ABORT_EX2("Bencode decoding failed:"
                     " Unexpected EOF in dict context. 'e' expected.",
                     error_code::BENCODE_PARSE_ERROR);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodelist
(InputIterator first,
 InputIterator last,
 size_t depth)
{
  SharedHandle<List> list = List::g();
  while(first != last) {
    if(*first == 'e') {
      return std::make_pair(list, ++first);
    } else {
      std::pair<SharedHandle<ValueBase>, InputIterator> r =
        decodeiter(first, last, depth);
      list->append(r.first);
      first = r.second;
    }
  }
  throw DL_ABORT_EX2("Bencode decoding failed:"
                     " Unexpected EOF in list context. 'e' expected.",
                     error_code::BENCODE_PARSE_ERROR);
}
} // namespace

namespace {
void checkDepth(size_t depth)
{
  if(depth >= MAX_STRUCTURE_DEPTH) {
    throw DL_ABORT_EX2("Bencode decoding failed: Structure is too deep.",
                       error_code::BENCODE_PARSE_ERROR);
  }
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeiter
(InputIterator first,
 InputIterator last,
 size_t depth)
{
  checkDepth(depth);
  if(first == last) {
    throw DL_ABORT_EX2("Bencode decoding failed:"
                       " Unexpected EOF in term context."
                       " 'd', 'l', 'i' or digit is expected.",
                       error_code::BENCODE_PARSE_ERROR);
  }
  if(*first == 'd') {
    return decodedict(++first, last, depth+1);
  } else if(*first == 'l') {
    return decodelist(++first, last, depth+1);
  } else if(*first == 'i') {
    return decodeinteger(++first, last);
  } else {
    return decodestring(first, last);
  }
}
} // namespace

namespace {
template<typename InputIterator>
SharedHandle<ValueBase> decodegen
(InputIterator first,
 InputIterator last,
 size_t& end)
{
  if(first == last) {
    return SharedHandle<ValueBase>();
  }
  std::pair<SharedHandle<ValueBase>, InputIterator> p =
    decodeiter(first, last, 0);
  end = p.second-first;
  return p.first;
}
} // namespace

SharedHandle<ValueBase> decode
(std::string::const_iterator first,
 std::string::const_iterator last)
{
  size_t end;
  return decodegen(first, last, end);
}

SharedHandle<ValueBase> decode
(std::string::const_iterator first,
 std::string::const_iterator last,
 size_t& end)
{
  return decodegen(first, last, end);
}

SharedHandle<ValueBase> decode
(const unsigned char* first,
 const unsigned char* last)
{
  size_t end;
  return decodegen(first, last, end);
}

SharedHandle<ValueBase> decode
(const unsigned char* first,
 const unsigned char* last,
 size_t& end)
{
  return decodegen(first, last, end);
}

SharedHandle<ValueBase> decodeFromFile(const std::string& filename)
{
  BufferedFile fp(filename, BufferedFile::READ);
  if(fp) {
    std::stringstream ss;
    fp.transfer(ss);
    fp.close();
    const std::string s = ss.str();
    size_t end;
    return decodegen(s.begin(), s.end(), end);
  } else {
    throw DL_ABORT_EX2
      (fmt("Bencode decoding failed: Cannot open file '%s'.",
           filename.c_str()),
       error_code::BENCODE_PARSE_ERROR);
  }
}

std::string encode(const ValueBase* vlb)
{
  class BencodeValueBaseVisitor:public ValueBaseVisitor {
  private:
    std::ostringstream out_;
  public:
    virtual void visit(const String& string)
    {
      const std::string& s = string.s();
      out_ << s.size() << ":";
      out_.write(s.data(), s.size());
    }

    virtual void visit(const Integer& integer)
    {
      out_ << "i" << integer.i() << "e";
    }

    virtual void visit(const Bool& v) {}
    virtual void visit(const Null& v) {}

    virtual void visit(const List& list)
    {
      out_ << "l";
      for(List::ValueType::const_iterator i = list.begin(), eoi = list.end();
          i != eoi; ++i){
        (*i)->accept(*this);
      }
      out_ << "e";
    }

    virtual void visit(const Dict& dict)
    {
      out_ << "d";
      for(Dict::ValueType::const_iterator i = dict.begin(), eoi = dict.end();
          i != eoi; ++i){
        const std::string& key = (*i).first;
        out_ << key.size() << ":";
        out_.write(key.data(), key.size());
        (*i).second->accept(*this);
      }
      out_ << "e";
    }

    std::string getResult() const
    {
      return out_.str();
    }
  };
  BencodeValueBaseVisitor visitor;
  vlb->accept(visitor);
  return visitor.getResult();
}

std::string encode(const SharedHandle<ValueBase>& vlb)
{
  return encode(vlb.get());
}

} // namespace bencode2

} // namespace aria2
