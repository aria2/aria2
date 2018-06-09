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
#include "RpcResponse.h"

#include <cassert>
#include <sstream>

#include "util.h"
#include "json.h"
#ifdef HAVE_ZLIB
#  include "GZipEncoder.h"
#endif // HAVE_ZLIB

namespace aria2 {

namespace rpc {

namespace {
template <typename OutputStream>
void encodeValue(const ValueBase* value, OutputStream& o)
{
  class XmlValueBaseVisitor : public ValueBaseVisitor {
  private:
    OutputStream& o_;

  public:
    XmlValueBaseVisitor(OutputStream& o) : o_(o) {}

    virtual ~XmlValueBaseVisitor() = default;

    virtual void visit(const String& v) CXX11_OVERRIDE
    {
      o_ << "<value><string>" << util::htmlEscape(v.s()) << "</string></value>";
    }

    virtual void visit(const Integer& v) CXX11_OVERRIDE
    {
      o_ << "<value><int>" << v.i() << "</int></value>";
    }

    virtual void visit(const Bool& boolValue) CXX11_OVERRIDE {}

    virtual void visit(const Null& nullValue) CXX11_OVERRIDE {}

    virtual void visit(const List& v) CXX11_OVERRIDE
    {
      o_ << "<value><array><data>";
      for (const auto& e : v) {
        e->accept(*this);
      }
      o_ << "</data></array></value>";
    }

    virtual void visit(const Dict& v) CXX11_OVERRIDE
    {
      o_ << "<value><struct>";
      for (const auto& e : v) {
        o_ << "<member><name>" << util::htmlEscape(e.first) << "</name>";
        e.second->accept(*this);
        o_ << "</member>";
      }
      o_ << "</struct></value>";
    }
  };

  XmlValueBaseVisitor visitor(o);
  value->accept(visitor);
}
} // namespace

namespace {
template <typename OutputStream>
std::string encodeAll(OutputStream& o, int code, const ValueBase* param)
{
  o << "<?xml version=\"1.0\"?>"
    << "<methodResponse>";
  if (code == 0) {
    o << "<params>"
      << "<param>";
    encodeValue(param, o);
    o << "</param>"
      << "</params>";
  }
  else {
    o << "<fault>";
    encodeValue(param, o);
    o << "</fault>";
  }
  o << "</methodResponse>";
  return o.str();
}
} // namespace

RpcResponse::RpcResponse(int code, RpcResponse::authorization_t authorized,
                         std::unique_ptr<ValueBase> param,
                         std::unique_ptr<ValueBase> id)
    : param{std::move(param)},
      id{std::move(id)},
      code{code},
      authorized{authorized}
{
}

std::string toXml(const RpcResponse& res, bool gzip)
{
  if (gzip) {
#ifdef HAVE_ZLIB
    GZipEncoder o;
    o.init();
    return encodeAll(o, res.code, res.param.get());
#else  // !HAVE_ZLIB
    abort();
#endif // !HAVE_ZLIB
  }
  else {
    std::stringstream o;
    return encodeAll(o, res.code, res.param.get());
  }
}

namespace {
template <typename OutputStream>
OutputStream& encodeJsonAll(OutputStream& o, int code, const ValueBase* param,
                            const ValueBase* id,
                            const std::string& callback = A2STR::NIL)
{
  if (!callback.empty()) {
    o << callback << "(";
  }
  o << "{\"id\":";
  json::encode(o, id);
  o << ",\"jsonrpc\":\"2.0\",";
  if (code == 0) {
    o << "\"result\":";
  }
  else {
    o << "\"error\":";
  }
  json::encode(o, param);
  o << "}";
  if (!callback.empty()) {
    o << ")";
  }
  return o;
}
} // namespace

std::string toJson(const RpcResponse& res, const std::string& callback,
                   bool gzip)
{
  if (gzip) {
#ifdef HAVE_ZLIB
    GZipEncoder o;
    o.init();
    return encodeJsonAll(o, res.code, res.param.get(), res.id.get(), callback)
        .str();
#else  // !HAVE_ZLIB
    abort();
#endif // !HAVE_ZLIB
  }
  else {
    std::stringstream o;
    return encodeJsonAll(o, res.code, res.param.get(), res.id.get(), callback)
        .str();
  }
}

namespace {
template <typename OutputStream>
OutputStream& encodeJsonBatchAll(OutputStream& o,
                                 const std::vector<RpcResponse>& results,
                                 const std::string& callback)
{
  if (!callback.empty()) {
    o << callback << "(";
  }
  o << "[";
  if (!results.empty()) {
    encodeJsonAll(o, results[0].code, results[0].param.get(),
                  results[0].id.get());

    for (auto i = std::begin(results) + 1, eoi = std::end(results); i != eoi;
         ++i) {
      o << ",";
      encodeJsonAll(o, (*i).code, (*i).param.get(), (*i).id.get());
    }
  }
  o << "]";
  if (!callback.empty()) {
    o << ")";
  }
  return o;
}
} // namespace

std::string toJsonBatch(const std::vector<RpcResponse>& results,
                        const std::string& callback, bool gzip)
{
  if (gzip) {
#ifdef HAVE_ZLIB
    GZipEncoder o;
    o.init();
    return encodeJsonBatchAll(o, results, callback).str();
#else  // !HAVE_ZLIB
    abort();
#endif // !HAVE_ZLIB
  }
  else {
    std::stringstream o;
    return encodeJsonBatchAll(o, results, callback).str();
  }
}

} // namespace rpc

} // namespace aria2
