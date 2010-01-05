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
#include "bencode.h"

#include <fstream>
#include <sstream>

#include "StringFormat.h"
#include "DlAbortEx.h"

namespace aria2 {

namespace bencode {

static BDE decodeiter(std::istream& ss, size_t depth);

static void checkdelim(std::istream& ss, const char delim = ':')
{
  char d;
  if(!(ss.get(d) && d == delim)) {
    throw DL_ABORT_EX
      (StringFormat("Bencode decoding failed: Delimiter '%c' not found.",
                    delim).str());
  }
}

static std::string decoderawstring(std::istream& ss)
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
      (StringFormat("Bencode decoding failed:"
                    " Expected %lu bytes of data, but only %d read.",
                    static_cast<unsigned long>(length), ss.gcount()).str());
  }
  return str;
}

static BDE decodestring(std::istream& ss)
{
  return BDE(decoderawstring(ss));
}

static BDE decodeinteger(std::istream& ss)
{
  BDE::Integer integer;
  ss >> integer;
  if(!ss) {
    throw DL_ABORT_EX("Bencode decoding failed:"
                      " Integer expected but none found");
  }
  checkdelim(ss, 'e');
  return BDE(integer);
}

static BDE decodedict(std::istream& ss, size_t depth)
{
  BDE dict = BDE::dict();
  char c;
  while(ss.get(c)) {
    if(c == 'e') {
      return dict;
    } else {
      ss.unget();
      std::string key = decoderawstring(ss);
      dict[key] = decodeiter(ss, depth);
    }
  }
  throw DL_ABORT_EX("Bencode decoding failed:"
                    " Unexpected EOF in dict context. 'e' expected.");
}

static BDE decodelist(std::istream& ss, size_t depth)
{
  BDE list = BDE::list();
  char c;
  while(ss.get(c)) {
    if(c == 'e') {
      return list;
    } else {
      ss.unget();
      list << decodeiter(ss, depth);
    }
  }
  throw DL_ABORT_EX("Bencode decoding failed:"
                    " Unexpected EOF in list context. 'e' expected.");
}

static void checkDepth(size_t depth)
{
  if(depth >= MAX_STRUCTURE_DEPTH) {
    throw DL_ABORT_EX("Bencode decoding failed: Structure is too deep.");
  }
}

static BDE decodeiter(std::istream& ss, size_t depth)
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

BDE decode(std::istream& in)
{
  return decodeiter(in, 0);
}

BDE decode(const std::string& s)
{
  size_t end;
  return decode(s, end);
}

BDE decode(const std::string& s, size_t& end)
{
  if(s.empty()) {
    return BDE::none;
  }
  std::istringstream ss(s);

  BDE bde = decodeiter(ss, 0);
  end = ss.tellg();
  return bde;
}

BDE decode(const unsigned char* data, size_t length)
{
  return decode(std::string(&data[0], &data[length]));
}

BDE decode(const unsigned char* data, size_t length, size_t& end)
{
  return decode(std::string(&data[0], &data[length]), end);
}

BDE decodeFromFile(const std::string& filename)
{
  std::ifstream f(filename.c_str(), std::ios::binary);
  if(f) {
    return decode(f);
  } else {
    throw DL_ABORT_EX
      (StringFormat("Bencode decoding failed:"
                    " Cannot open file '%s'.", filename.c_str()).str());
  }
}

static void encodeIter(std::ostream& o, const BDE& bde)
{
  if(bde.isInteger()) {
    o << "i" << bde.i() << "e";
  } else if(bde.isString()) {
    const std::string& s = bde.s();
    o << s.size() << ":";
    o.write(s.data(), s.size());
  } else if(bde.isDict()) {
    o << "d";
    for(BDE::Dict::const_iterator i = bde.dictBegin(); i != bde.dictEnd(); ++i){
      const std::string& key = (*i).first;
      o << key.size() << ":";
      o.write(key.data(), key.size());
      encodeIter(o, (*i).second);
    }
    o << "e";
  } else if(bde.isList()) {
    o << "l";
    for(BDE::List::const_iterator i = bde.listBegin(); i != bde.listEnd(); ++i){
      encodeIter(o, *i);
    }
    o << "e";
  }
}

std::string encode(const BDE& bde)
{
  std::ostringstream ss;
  encodeIter(ss, bde);
  return ss.str();
}

} // namespace bencode

} // namespace aria2
