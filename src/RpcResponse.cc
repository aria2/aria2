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
# include "GZipEncoder.h"
#endif // HAVE_ZLIB

namespace aria2 {

namespace rpc {

namespace {
template<typename OutputStream>
void encodeValue(const SharedHandle<ValueBase>& value, OutputStream& o)
{
  class XmlValueBaseVisitor:public ValueBaseVisitor {
  private:
    OutputStream& o_;
  public:
    XmlValueBaseVisitor(OutputStream& o):o_(o) {}

    virtual ~XmlValueBaseVisitor() {}

    virtual void visit(const String& v)
    {
      o_ << "<value><string>" << util::htmlEscape(v.s()) << "</string></value>";
    }

    virtual void visit(const Integer& v)
    {
      o_ << "<value><int>" << v.i() << "</int></value>";
    }

    virtual void visit(const Bool& boolValue) {}

    virtual void visit(const Null& nullValue) {}

    virtual void visit(const List& v)
    {
      o_ << "<value><array><data>";
      for(List::ValueType::const_iterator i = v.begin(), eoi = v.end();
          i != eoi; ++i) {
        (*i)->accept(*this);
      }
      o_ << "</data></array></value>";
    }

    virtual void visit(const Dict& v)
    {
      o_ << "<value><struct>";
      for(Dict::ValueType::const_iterator i = v.begin(), eoi = v.end();
          i != eoi; ++i) {
        o_ << "<member><name>" << util::htmlEscape((*i).first) << "</name>";
        (*i).second->accept(*this);
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
template<typename OutputStream>
std::string encodeAll
(OutputStream& o, int code, const SharedHandle<ValueBase>& param)
{
  o << "<?xml version=\"1.0\"?>" << "<methodResponse>";
  if(code == 0) {
    o << "<params>" << "<param>";
    encodeValue(param, o);
    o << "</param>" << "</params>";
  } else {
    o << "<fault>";
    encodeValue(param, o);
    o << "</fault>";
  }
  o << "</methodResponse>";
  return o.str();
}
} // namespace

RpcResponse::RpcResponse
(int code,
 const SharedHandle<ValueBase>& param,
 const SharedHandle<ValueBase>& id)
  : code(code), param(param), id(id)
{}

RpcResponse::RpcResponse(const RpcResponse& c)
  : code(c.code),
    param(c.param),
    id(c.id)
{}

RpcResponse::~RpcResponse() {}

RpcResponse& RpcResponse::operator=(const RpcResponse& c)
{
  if(this != &c) {
    code = c.code;
    param = c.param;
  }
  return *this;
}

std::string toXml(const RpcResponse& res, bool gzip)
{
  if(gzip) {
#ifdef HAVE_ZLIB
    GZipEncoder o;
    o.init();
    return encodeAll(o, res.code, res.param);
#else // !HAVE_ZLIB
    abort();
#endif // !HAVE_ZLIB
  } else {
    std::stringstream o;
    return encodeAll(o, res.code, res.param);
  }
}

namespace {
template<typename OutputStream>
OutputStream& encodeJsonAll
(OutputStream& o,
 int code,
 const SharedHandle<ValueBase>& param,
 const SharedHandle<ValueBase>& id,
 const std::string& callback = A2STR::NIL)
{
  if(!callback.empty()) {
    o << callback << "(";
  }
  SharedHandle<Dict> dict = Dict::g();
  dict->put("jsonrpc", "2.0");
  dict->put("id", id);
  if(code == 0) {
    dict->put("result", param);
  } else {
    dict->put("error", param);
  }
  json::encode(o, dict);
  if(!callback.empty()) {
    o << ")";
  }
  return o;
}
} // namespace

std::string toJson
(const RpcResponse& res, const std::string& callback, bool gzip)
{
  if(gzip) {
#ifdef HAVE_ZLIB
    GZipEncoder o;
    o.init();
    return encodeJsonAll(o, res.code, res.param, res.id, callback).str();
#else // !HAVE_ZLIB
    abort();
#endif // !HAVE_ZLIB
  } else {
    std::stringstream o;
    return encodeJsonAll(o, res.code, res.param, res.id, callback).str();
  }
}

namespace {
template<typename OutputStream>
OutputStream& encodeJsonBatchAll
(OutputStream& o,
 const std::vector<RpcResponse>& results,
 const std::string& callback)
{
  if(!callback.empty()) {
    o << callback << "(";
  }
  o << "[";
  if(!results.empty()) {
    encodeJsonAll(o, results[0].code, results[0].param, results[0].id);
  }
  for(std::vector<RpcResponse>::const_iterator i = results.begin()+1,
        eoi = results.end(); i != eoi; ++i) {
    o << ",";
    encodeJsonAll(o, (*i).code, (*i).param, (*i).id);
  }
  o << "]";
  if(!callback.empty()) {
    o << ")";
  }
  return o;
}
} // namespace

std::string toJsonBatch
(const std::vector<RpcResponse>& results,
 const std::string& callback,
 bool gzip)
{
  if(gzip) {
#ifdef HAVE_ZLIB
    GZipEncoder o;
    o.init();
    return encodeJsonBatchAll(o, results, callback).str();
#else // !HAVE_ZLIB
    abort();
#endif // !HAVE_ZLIB
  } else {
    std::stringstream o;
    return encodeJsonBatchAll(o, results, callback).str();
  }
}

} // namespace rpc

} // namespace aria2
