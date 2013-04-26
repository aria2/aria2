/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "aria2api.h"

#include <functional>

#include "Platform.h"
#include "Context.h"
#include "DownloadEngine.h"
#include "OptionParser.h"
#include "Option.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "OptionHandler.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "MultiUrlRequestInfo.h"
#include "prefs.h"
#include "download_helper.h"
#include "LogFactory.h"

namespace aria2 {

Session::Session(const KeyVals& options)
  : context(new Context(false, 0, 0, options))
{}

Session::~Session()
{}

namespace {
Platform* platform = 0;
} // namespace

int libraryInit()
{
  platform = new Platform();
  return 0;
}

int libraryDeinit()
{
  delete platform;
  return 0;
}

Session* sessionNew(const KeyVals& options)
{
  int rv;
  Session* session = new Session(options);
  if(session->context->reqinfo) {
    rv = session->context->reqinfo->prepare();
    if(rv != 0) {
      delete session;
      session = 0;
    }
  } else {
    delete session;
    session = 0;
  }
  return session;
}

int sessionFinal(Session* session)
{
  error_code::Value rv = session->context->reqinfo->getResult();
  delete session;
  return rv;
}

int run(Session* session, RUN_MODE mode)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  return e->run(mode == RUN_ONCE);
}

namespace {
template<typename InputIterator, typename Pred>
void apiGatherOption
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
    const OptionHandler* handler = optionParser->find(pref);
    if(!handler || !pred(handler)) {
      // Just ignore the unacceptable options in this context.
      continue;
    }
    handler->parse(*option, (*first).second);
  }
}
} // namespace

namespace {
void apiGatherRequestOption(Option* option, const KeyVals& options,
                            const SharedHandle<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fun(&OptionHandler::getInitialOption),
                  option, optionParser);
}
} // namespace

namespace {
void addRequestGroup(const SharedHandle<RequestGroup>& group,
                     const SharedHandle<DownloadEngine>& e,
                     int position)
{
  if(position >= 0) {
    e->getRequestGroupMan()->insertReservedGroup(position, group);
  } else {
    e->getRequestGroupMan()->addReservedGroup(group);
  }
}
} // namespace

int addUri(Session* session,
           A2Gid& gid,
           const std::vector<std::string>& uris,
           const KeyVals& options,
           int position)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForUri(result, requestOption, uris,
                           /* ignoreForceSeq = */ true,
                           /* ignoreLocalPath = */ true);
  if(!result.empty()) {
    gid = result.front()->getGID();
    addRequestGroup(result.front(), e, position);
  }
  return 0;
}

} // namespace aria2
