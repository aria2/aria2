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
#include "XmlRpcMethodImpl.h"

#include <cassert>

#include "Logger.h"
#include "BDE.h"
#include "DlAbortEx.h"
#include "Option.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "download_helper.h"
#include "Util.h"
#include "RequestGroupMan.h"
#include "StringFormat.h"
#include "XmlRpcRequest.h"

namespace aria2 {

namespace xmlrpc {

BDE AddURIXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isList() || params[0].empty()) {
    throw DlAbortEx("URI is not provided.");
  }
  std::deque<std::string> uris;
  for(BDE::List::const_iterator i = params[0].listBegin();
      i != params[0].listEnd(); ++i) {
    if((*i).isString()) {
      uris.push_back((*i).s());
    }
  }

  Option requestOption;
  if(params.size() > 1 && params[1].isDict()) {
    gatherRequestOption(requestOption, *e->option, params[1]);
  }
  std::deque<SharedHandle<RequestGroup> > result;
  createRequestGroupForUri(result, *e->option, uris, requestOption,
			   /* ignoreForceSeq = */ true,
			   /* ignoreNonURI = */ true);

  if(!result.empty()) {
    e->_requestGroupMan->addReservedGroup(result.front());
    BDE resParams = BDE::list();
    resParams << BDE("OK");
    resParams << BDE(Util::itos(result.front()->getGID()));
    return resParams;
  } else {
    throw DlAbortEx("No URI to download.");
  }
}

BDE RemoveXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DlAbortEx("GID is not provided.");
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);

  if(group.isNull()) {
    throw DlAbortEx
      (StringFormat("Active Download not found for GID#%d", gid).str());
  }

  group->setHaltRequested(true);

  BDE resParams = BDE::list();
  resParams << BDE("OK");
  resParams << BDE(Util::itos(gid));
  return resParams;
}

BDE FailXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  throw DlAbortEx
    (StringFormat("Method %s was not recognized.",
		  req._methodName.c_str()).str());
}

} // namespace xmlrpc

} // namespace aria2
