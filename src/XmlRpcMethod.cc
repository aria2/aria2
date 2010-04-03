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
#include "StringFormat.h"

namespace aria2 {

namespace xmlrpc {

XmlRpcMethod::XmlRpcMethod():
  _optionParser(OptionParser::getInstance()),
  _logger(LogFactory::getInstance()) {}

BDE XmlRpcMethod::createErrorResponse(const Exception& e)
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
    if(_logger->debug()) {
      _logger->debug(EX_EXCEPTION_CAUGHT, e);
    }
    return XmlRpcResponse(1, createErrorResponse(e));
  }
}

template<typename InputIterator>
static void gatherOption
(InputIterator first, InputIterator last,
 const std::set<std::string>& allowedOptions,
 const SharedHandle<Option>& option,
 const SharedHandle<OptionParser>& optionParser)
{
  for(; first != last; ++first) {
    const std::string& optionName = (*first).first;
    if(allowedOptions.count(optionName) == 0) {
      throw DL_ABORT_EX
        (StringFormat
         ("%s option cannot be used in this context.",
          optionName.c_str()).str());
    } else {
      SharedHandle<OptionHandler> optionHandler =
        optionParser->findByName(optionName);
      if(optionHandler.isNull()) {
        throw DL_ABORT_EX
          (StringFormat
           ("We don't know how to deal with %s option",
            optionName.c_str()).str());
      }
      // header and index-out option can take array as value
      const BDE& value = (*first).second;
      if((optionName == PREF_HEADER || optionName == PREF_INDEX_OUT) &&
         value.isList()){
        for(BDE::List::const_iterator argiter = value.listBegin(),
              eoi = value.listEnd(); argiter != eoi; ++argiter) {
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

void XmlRpcMethod::gatherRequestOption
(const SharedHandle<Option>& option, const BDE& optionsDict)
{
  gatherOption(optionsDict.dictBegin(), optionsDict.dictEnd(),
               listRequestOptions(),
               option, _optionParser);
}

// Copy option in the range [optNameFirst, optNameLast) from src to
// dest.
template<typename InputIterator>
static void applyOption(InputIterator optNameFirst,
                        InputIterator optNameLast,
                        Option* dest,
                        Option* src)
{
  for(; optNameFirst != optNameLast; ++optNameFirst) {
    if(src->defined(*optNameFirst)) {
      dest->put(*optNameFirst, src->get(*optNameFirst));
    }
  }
}

const std::set<std::string>& listChangeableOptions()
{
  static const std::string OPTIONS[] = {
    PREF_BT_MAX_PEERS,
    PREF_BT_REQUEST_PEER_SPEED_LIMIT,
    PREF_MAX_DOWNLOAD_LIMIT,
    PREF_MAX_UPLOAD_LIMIT
  };
  static std::set<std::string> options(vbegin(OPTIONS), vend(OPTIONS));
  return options;
}

void XmlRpcMethod::gatherChangeableOption
(const SharedHandle<Option>& option, const BDE& optionsDict)
{
  gatherOption(optionsDict.dictBegin(), optionsDict.dictEnd(),
               listChangeableOptions(),
               option, _optionParser);
}

void XmlRpcMethod::applyChangeableOption(Option* dest, Option* src) const
{
  applyOption(listChangeableOptions().begin(), listChangeableOptions().end(),
              dest, src);
}

const std::set<std::string>& listChangeableGlobalOptions()
{
  static const std::string OPTIONS[] = {
    PREF_MAX_OVERALL_UPLOAD_LIMIT,
    PREF_MAX_OVERALL_DOWNLOAD_LIMIT,
    PREF_MAX_CONCURRENT_DOWNLOADS,
  };
  static std::set<std::string> options(vbegin(OPTIONS), vend(OPTIONS));
  return options;
}

void XmlRpcMethod::gatherChangeableGlobalOption
(const SharedHandle<Option>& option, const BDE& optionsDict)
{
  gatherOption(optionsDict.dictBegin(), optionsDict.dictEnd(),
               listChangeableGlobalOptions(),
               option, _optionParser);
}

void XmlRpcMethod::applyChangeableGlobalOption(Option* dest, Option* src) const
{
  applyOption(listChangeableGlobalOptions().begin(),
              listChangeableGlobalOptions().end(),
              dest, src);
}

} // namespace xmlrpc

} // namespace aria2
