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

#include <fstream>
#include <sstream>

#include "fmt.h"
#include "DlAbortEx.h"

namespace aria2 {

namespace bencode2 {

namespace {
SharedHandle<ValueBase> decodeiter(std::istream& ss, size_t depth);
} // namespace

namespace {
void checkdelim(std::istream& ss, const char delim = ':')
{
  char d;
  if(!(ss.get(d) && d == delim)) {
    throw DL_ABORT_EX
      (fmt("Bencode decoding failed: Delimiter '%c' not found.",
           delim));
  }
}
} // namespace

namespace {
std::string decoderawstring(std::istream& ss)
{
  int length;
  ss >> length;
  if(!ss || length < 0) {
    throw DL_ABORT_EX("Bencode decoding failed:"
                      " A positive integer expected but none found.");
  }
  // TODO check length, it must be less than or equal to INT_MAX
  checkdelim(ss);
  char* buf = new char[length];
  ss.read(buf, length);
  std::string str(&buf[0], &buf[length]);
  delete [] buf;
  if(ss.gcount() != static_cast<int>(length)) {
    throw DL_ABORT_EX
      (fmt("Bencode decoding failed:"
           " Expected %lu bytes of data, but only %ld read.",
           static_cast<unsigned long>(length),
           static_cast<long int>(ss.gcount())));
  }
  return str;
}
} // namespace

namespace {
SharedHandle<ValueBase> decodestring(std::istream& ss)
{
  return String::g(decoderawstring(ss));
}
} // namespace

namespace {
SharedHandle<ValueBase> decodeinteger(std::istream& ss)
{
  Integer::ValueType iv;
  ss >> iv;
  if(!ss) {
    throw DL_ABORT_EX("Bencode decoding failed:"
                      " Integer expected but none found");
  }
  checkdelim(ss, 'e');
  return Integer::g(iv);
}
} // namespace

namespace {
SharedHandle<ValueBase> decodedict(std::istream& ss, size_t depth)
{
  SharedHandle<Dict> dict = Dict::g();
  char c;
  while(ss.get(c)) {
    if(c == 'e') {
      return dict;
    } else {
      ss.unget();
      std::string key = decoderawstring(ss);
      dict->put(key, decodeiter(ss, depth));
    }
  }
  throw DL_ABORT_EX("Bencode decoding failed:"
                    " Unexpected EOF in dict context. 'e' expected.");
}
} // namespace

namespace {
SharedHandle<ValueBase> decodelist(std::istream& ss, size_t depth)
{
  SharedHandle<List> list = List::g();
  char c;
  while(ss.get(c)) {
    if(c == 'e') {
      return list;
    } else {
      ss.unget();
      list->append(decodeiter(ss, depth));
    }
  }
  throw DL_ABORT_EX("Bencode decoding failed:"
                    " Unexpected EOF in list context. 'e' expected.");
}
} // namespace

namespace {
void checkDepth(size_t depth)
{
  if(depth >= MAX_STRUCTURE_DEPTH) {
    throw DL_ABORT_EX("Bencode decoding failed: Structure is too deep.");
  }
}
} // namespace

namespace {
SharedHandle<ValueBase> decodeiter(std::istream& ss, size_t depth)
{
  checkDepth(depth);
  char c;
  if(!ss.get(c)) {
    throw DL_ABORT_EX("Bencode decoding failed:"
                      " Unexpected EOF in term context."
                      " 'd', 'l', 'i' or digit is expected.");
  }
  if(c == 'd') {
    return decodedict(ss, depth+1);
  } else if(c == 'l') {
    return decodelist(ss, depth+1);
  } else if(c == 'i') {
    return decodeinteger(ss);
  } else {
    ss.unget();
    return decodestring(ss);
  }
}
} // namespace

SharedHandle<ValueBase> decode(std::istream& in)
{
  return decodeiter(in, 0);
}

SharedHandle<ValueBase> decode(const std::string& s)
{
  size_t end;
  return decode(s, end);
}

SharedHandle<ValueBase> decode(const std::string& s, size_t& end)
{
  if(s.empty()) {
    return SharedHandle<ValueBase>();
  }
  std::istringstream ss(s);

  SharedHandle<ValueBase> vlb = decodeiter(ss, 0);
  end = ss.tellg();
  return vlb;
}

SharedHandle<ValueBase> decode(const unsigned char* data, size_t length)
{
  return decode(std::string(&data[0], &data[length]));
}

SharedHandle<ValueBase> decode(const unsigned char* data, size_t length, size_t& end)
{
  return decode(std::string(&data[0], &data[length]), end);
}

SharedHandle<ValueBase> decodeFromFile(const std::string& filename)
{
  std::ifstream f(filename.c_str(), std::ios::binary);
  if(f) {
    return decode(f);
  } else {
    throw DL_ABORT_EX
      (fmt("Bencode decoding failed: Cannot open file '%s'.",
           filename.c_str()));
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
