/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#ifndef D_JSON_H
#define D_JSON_H

#include "common.h"
#include "ValueBase.h"

namespace aria2 {

namespace json {

std::string jsonEscape(const std::string& s);

template <typename OutputStream>
OutputStream& encode(OutputStream& out, const ValueBase* vlb)
{
  class JsonValueBaseVisitor : public ValueBaseVisitor {
  public:
    JsonValueBaseVisitor(OutputStream& out) : out_(out) {}

    virtual void visit(const String& string) CXX11_OVERRIDE
    {
      encodeString(string.s());
    }

    virtual void visit(const Integer& integer) CXX11_OVERRIDE
    {
      out_ << integer.i();
    }

    virtual void visit(const Bool& boolValue) CXX11_OVERRIDE
    {
      out_ << (boolValue.val() ? "true" : "false");
    }

    virtual void visit(const Null& nullValue) CXX11_OVERRIDE { out_ << "null"; }

    virtual void visit(const List& list) CXX11_OVERRIDE
    {
      out_ << "[";
      if (!list.empty()) {
        auto i = list.begin();
        (*i)->accept(*this);
        ++i;
        for (auto eoi = list.end(); i != eoi; ++i) {
          out_ << ",";
          (*i)->accept(*this);
        }
      }
      out_ << "]";
    }

    virtual void visit(const Dict& dict) CXX11_OVERRIDE
    {
      out_ << "{";
      if (!dict.empty()) {
        auto i = dict.begin();
        encodeString((*i).first);
        out_ << ":";
        (*i).second->accept(*this);
        ++i;
        for (auto eoi = dict.end(); i != eoi; ++i) {
          out_ << ",";
          encodeString((*i).first);
          out_ << ":";
          (*i).second->accept(*this);
        }
      }
      out_ << "}";
    }

  private:
    void encodeString(const std::string& s)
    {
      out_ << "\"" << jsonEscape(s) << "\"";
    }
    OutputStream& out_;
  };
  JsonValueBaseVisitor visitor(out);
  vlb->accept(visitor);
  return out;
}

// Serializes JSON object or array.
std::string encode(const ValueBase* json);

struct JsonGetParam {
  std::string request;
  std::string callback;
  JsonGetParam(const std::string& request, const std::string& callback);
};

// Decodes JSON-RPC request from GET query parameter query. query must
// starts with "?". This function identifies method name, id,
// parameters and jsonp callback.  For method name, it searches
// "method" query parameter.  For id, it searches "id" query
// parameter. The id is always treated as string.  For parameters, it
// searches "params" query parameter. The params is Base64 encoded
// JSON string normally associated "params" key in POST request.  For
// jsonp callback, it searches "jsoncallback".  For example, calling
// remote method, sum([1,2,3]) with id=300 looks like this:
// ?method=sum&id=300&params=WzEsMiwzXQ%3D%3D
//
// If both method and id are missing, params is treated as batch call.
JsonGetParam decodeGetParams(const std::string& query);

} // namespace json

} // namespace aria2

#endif // D_JSON_H
