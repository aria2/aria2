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
#include "RpcMethod.h"
#include "DownloadEngine.h"
#include "LogFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "Option.h"
#include "array_fun.h"
#include "download_helper.h"
#include "RpcRequest.h"
#include "RpcResponse.h"
#include "prefs.h"
#include "fmt.h"
#include "DlAbortEx.h"
#include "a2functional.h"

namespace aria2 {

namespace rpc {

RpcMethod::RpcMethod()
  : optionParser_(OptionParser::getInstance())
{}

RpcMethod::~RpcMethod() {}

SharedHandle<ValueBase> RpcMethod::createErrorResponse
(const Exception& e, const RpcRequest& req)
{
  SharedHandle<Dict> params = Dict::g();
  params->put((req.jsonRpc ? "code" : "faultCode"), Integer::g(1));
  params->put((req.jsonRpc ? "message" : "faultString"), std::string(e.what()));
  return params;
}

RpcResponse RpcMethod::execute
(const RpcRequest& req, DownloadEngine* e)
{
  try {
    return RpcResponse(0, process(req, e), req.id);
  } catch(RecoverableException& ex) {
    A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, ex);
    return RpcResponse(1, createErrorResponse(ex, req), req.id);
  }
}

namespace {
template<typename InputIterator, typename Pred>
void gatherOption
(InputIterator first, InputIterator last,
 Pred pred,
 Option* option,
 const SharedHandle<OptionParser>& optionParser)
{
  for(; first != last; ++first) {
    const std::string& optionName = (*first).first;
    const Pref* pref = option::k2p(optionName);
    if(!pref) {
      throw DL_ABORT_EX
        (fmt("We don't know how to deal with %s option", optionName.c_str()));
    }
    const SharedHandle<OptionHandler>& handler = optionParser->find(pref);
    if(!handler || !pred(handler)) {
      // Just ignore the unacceptable options in this context.
      continue;
    }
    const String* opval = downcast<String>((*first).second);
    if(opval) {
      handler->parse(*option, opval->s());
    } else if(handler->getCumulative()) {
      // header and index-out option can take array as value
      const List* oplist = downcast<List>((*first).second);
      if(oplist) {
        for(List::ValueType::const_iterator argiter = oplist->begin(),
              eoi = oplist->end(); argiter != eoi; ++argiter) {
          const String* opval = downcast<String>(*argiter);
          if(opval) {
            handler->parse(*option, opval->s());
          }
        }
      }
    }
  }  
}
} // namespace

void RpcMethod::gatherRequestOption(Option* option, const Dict* optionsDict)
{
  if(optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 mem_fun_sh(&OptionHandler::getInitialOption),
                 option, optionParser_);
  }
}

void RpcMethod::gatherChangeableOption(Option* option, const Dict* optionsDict)
{
  if(optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 mem_fun_sh(&OptionHandler::getChangeOption),
                 option, optionParser_);
  }
}

void RpcMethod::gatherChangeableOptionForReserved
(Option* option,
 const Dict* optionsDict)
{
  if(optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 mem_fun_sh(&OptionHandler::getChangeOptionForReserved),
                 option, optionParser_);
  }
}

void RpcMethod::gatherChangeableGlobalOption
(Option* option, const Dict* optionsDict)
{
  if(optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 mem_fun_sh(&OptionHandler::getChangeGlobalOption),
                 option, optionParser_);
  }
}

} // namespace rpc

} // namespace aria2
