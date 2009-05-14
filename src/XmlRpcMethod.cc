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
#include "XmlRpcResponse.h"
#include "prefs.h"

namespace aria2 {

namespace xmlrpc {

XmlRpcMethod::XmlRpcMethod():
  _optionParser(OptionParser::getInstance()),
  _logger(LogFactory::getInstance()) {}

static BDE createErrorResponse(const Exception& e)
{
  BDE params = BDE::dict();
  params["faultCode"] = BDE(1);
  params["faultString"] = BDE(e.what());
  return params;
}

XmlRpcResponse XmlRpcMethod::execute
(const XmlRpcRequest& req, DownloadEngine* e)
{
  try {
    return XmlRpcResponse(0, process(req, e));
  } catch(RecoverableException& e) {
    _logger->debug(EX_EXCEPTION_CAUGHT, e);
    return XmlRpcResponse(1, createErrorResponse(e));
  }
}

void XmlRpcMethod::gatherRequestOption
(const SharedHandle<Option>& option, const BDE& optionsDict)
{
  for(std::vector<std::string>::const_iterator i = listRequestOptions().begin();
      i != listRequestOptions().end(); ++i) {
    if(optionsDict.containsKey(*i)) {
      const BDE& value = optionsDict[*i];
      SharedHandle<OptionHandler> optionHandler = _optionParser->findByName(*i);
      // header and index-out option can take array as value
      if((*i == PREF_HEADER || *i == PREF_INDEX_OUT) && value.isList()) {
	for(BDE::List::const_iterator argiter = value.listBegin();
	    argiter != value.listEnd(); ++argiter) {
	  if((*argiter).isString()) {
	    optionHandler->parse(*option.get(), (*argiter).s());
	  }
	}
      } else if(value.isString()) {
	optionHandler->parse(*option.get(), value.s());
      }
    }
  }
}

} // namespace xmlrpc

} // namespace aria2
