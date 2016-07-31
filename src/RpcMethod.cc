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
#include "util.h"

namespace aria2 {

namespace rpc {

RpcMethod::RpcMethod() : optionParser_(OptionParser::getInstance()) {}

RpcMethod::~RpcMethod() = default;

std::unique_ptr<ValueBase> RpcMethod::createErrorResponse(const Exception& e,
                                                          const RpcRequest& req)
{
  auto params = Dict::g();
  params->put((req.jsonRpc ? "code" : "faultCode"), Integer::g(1));
  params->put((req.jsonRpc ? "message" : "faultString"), std::string(e.what()));
  return std::move(params);
}

void RpcMethod::authorize(RpcRequest& req, DownloadEngine* e)
{
  std::string token;
  // We always treat first parameter as token if it is string and
  // starts with "token:" and remove it from parameter list, so that
  // we don't have to add conditionals to all RPCMethod
  // implementations.
  if (req.params && !req.params->empty()) {
    auto t = downcast<String>(req.params->get(0));
    if (t) {
      if (util::startsWith(t->s(), "token:")) {
        token = t->s().substr(6);
        req.params->pop_front();
      }
    }
  }
  if (!e || !e->validateToken(token)) {
    throw DL_ABORT_EX("Unauthorized");
  }
}

RpcResponse RpcMethod::execute(RpcRequest req, DownloadEngine* e)
{
  auto authorized = RpcResponse::NOTAUTHORIZED;
  try {
    authorize(req, e);
    authorized = RpcResponse::AUTHORIZED;
    auto r = process(req, e);
    return RpcResponse(0, authorized, std::move(r), std::move(req.id));
  }
  catch (RecoverableException& ex) {
    A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, ex);
    return RpcResponse(1, authorized, createErrorResponse(ex, req),
                       std::move(req.id));
  }
}

namespace {
template <typename InputIterator, typename Pred>
void gatherOption(InputIterator first, InputIterator last, Pred pred,
                  Option* option,
                  const std::shared_ptr<OptionParser>& optionParser)
{
  for (; first != last; ++first) {
    const std::string& optionName = (*first).first;
    PrefPtr pref = option::k2p(optionName);
    const OptionHandler* handler = optionParser->find(pref);
    if (!handler || !pred(handler)) {
      // Just ignore the unacceptable options in this context.
      continue;
    }
    const String* opval = downcast<String>((*first).second);
    if (opval) {
      handler->parse(*option, opval->s());
    }
    else if (handler->getCumulative()) {
      // header and index-out option can take array as value
      const List* oplist = downcast<List>((*first).second);
      if (oplist) {
        for (auto& elem : *oplist) {
          const String* opval = downcast<String>(elem);
          if (opval) {
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
  if (optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 std::mem_fn(&OptionHandler::getInitialOption), option,
                 optionParser_);
  }
}

void RpcMethod::gatherChangeableOption(Option* option, Option* pendingOption,
                                       const Dict* optionsDict)
{
  if (!optionsDict) {
    return;
  }

  auto first = optionsDict->begin();
  auto last = optionsDict->end();

  for (; first != last; ++first) {
    const auto& optionName = (*first).first;
    auto pref = option::k2p(optionName);
    auto handler = optionParser_->find(pref);
    if (!handler) {
      // Just ignore the unacceptable options in this context.
      continue;
    }

    Option* dst = nullptr;
    if (handler->getChangeOption()) {
      dst = option;
    }
    else if (handler->getChangeOptionForReserved()) {
      dst = pendingOption;
    }

    if (!dst) {
      continue;
    }

    const auto opval = downcast<String>((*first).second);
    if (opval) {
      handler->parse(*dst, opval->s());
    }
    else if (handler->getCumulative()) {
      // header and index-out option can take array as value
      const auto oplist = downcast<List>((*first).second);
      if (oplist) {
        for (auto& elem : *oplist) {
          const auto opval = downcast<String>(elem);
          if (opval) {
            handler->parse(*dst, opval->s());
          }
        }
      }
    }
  }
}

void RpcMethod::gatherChangeableOptionForReserved(Option* option,
                                                  const Dict* optionsDict)
{
  if (optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 std::mem_fn(&OptionHandler::getChangeOptionForReserved),
                 option, optionParser_);
  }
}

void RpcMethod::gatherChangeableGlobalOption(Option* option,
                                             const Dict* optionsDict)
{
  if (optionsDict) {
    gatherOption(optionsDict->begin(), optionsDict->end(),
                 std::mem_fn(&OptionHandler::getChangeGlobalOption), option,
                 optionParser_);
  }
}

} // namespace rpc

} // namespace aria2
