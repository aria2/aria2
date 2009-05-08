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
#include "XmlRpcMethod.h"

#include <cassert>
#include <sstream>

#include "DownloadEngine.h"
#include "BDE.h"
#include "LogFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "Option.h"
#include "array_fun.h"
#include "download_helper.h"
#include "XmlRpcRequest.h"

namespace aria2 {

namespace xmlrpc {

XmlRpcMethod::XmlRpcMethod():
  _optionParser(OptionParser::getInstance()),
  _logger(LogFactory::getInstance()) {}

static BDE createErrorResponse(const Exception& e)
{
  BDE params = BDE::list();
  params << BDE("ERROR");
  params << BDE(e.what());
  return params;
}

static void encodeValue(const BDE& value, std::ostream& o);

template<typename InputIterator>
static void encodeArray
(InputIterator first, InputIterator last, std::ostream& o)
{
  o << "<array>" << "<data>";
  for(; first != last; ++first) {
    encodeValue(*first, o);
  }
  o << "</data>" << "</array>";
}

template<typename InputIterator>
static void encodeStruct
(InputIterator first, InputIterator last, std::ostream& o)
{
  o << "<struct>";
  for(; first != last; ++first) {
    o << "<member>"
      << "<name>" << (*first).first << "</name>";
    encodeValue((*first).second, o);
    o << "</member>";
  }
  o << "</struct>";
}

static void encodeValue(const BDE& value, std::ostream& o)
{
  o << "<value>";
  if(value.isString()) {
    o << "<string>" << value.s() << "</string>";
  } else if(value.isList()) {
    encodeArray(value.listBegin(), value.listEnd(), o);
  } else if(value.isDict()) {
    encodeStruct(value.dictBegin(), value.dictEnd(), o);
  }
  o << "</value>";
}

template<typename InputIterator>
static void encodeParams
(InputIterator first, InputIterator last, std::ostream& o)
{
  o << "<params>";
  for(; first != last; ++first) {
    o << "<param>";
    encodeValue(*first, o);
    o << "</param>";
  }
  o << "</params>";
}

static std::string encodeXml(const BDE& params)
{
  assert(params.isList());
  std::stringstream o;
  o << "<?xml version=\"1.0\"?>" << "<methodResponse>";
  encodeParams(params.listBegin(), params.listEnd(), o);
  o << "</methodResponse>";
  return o.str();
}

std::string XmlRpcMethod::execute(const XmlRpcRequest& req, DownloadEngine* e)
{
  try {
    BDE retparams = process(req, e);
    return encodeXml(retparams);
  } catch(RecoverableException& e) {
    _logger->debug(EX_EXCEPTION_CAUGHT, e);
    return encodeXml(createErrorResponse(e));
  }
}

void XmlRpcMethod::gatherRequestOption
(Option& requestOption, const Option& option, const BDE& optionsDict)
{
  for(std::vector<std::string>::const_iterator i = listRequestOptions().begin();
      i != listRequestOptions().end(); ++i) {
    if(optionsDict.containsKey(*i)) {
      const BDE& value = optionsDict[*i];
      if(value.isString()) {
	_optionParser->findByName(*i)->parse
	  (requestOption, value.s());
      }
    }
  }
  completeRequestOption(requestOption, option);
}

} // namespace xmlrpc

} // namespace aria2
