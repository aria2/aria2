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
#include "HttpServerBodyCommand.h"
#include "SocketCore.h"
#include "DownloadEngine.h"
#include "HttpServer.h"
#include "HttpHeader.h"
#include "Logger.h"
#include "LogFactory.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "RecoverableException.h"
#include "HttpServerResponseCommand.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "XmlRpcRequestProcessor.h"
#include "XmlRpcRequestParserStateMachine.h"
#include "XmlRpcMethod.h"
#include "XmlRpcMethodFactory.h"
#include "XmlRpcResponse.h"
#include "wallclock.h"
#include "util.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "json.h"
#include "DlAbortEx.h"

namespace aria2 {

HttpServerBodyCommand::HttpServerBodyCommand
(cuid_t cuid,
 const SharedHandle<HttpServer>& httpServer,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& socket)
  : Command(cuid),
    e_(e),
    socket_(socket),
    httpServer_(httpServer)
{
  // To handle Content-Length == 0 case
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  e_->addSocketForReadCheck(socket_, this);
  if(!httpServer_->getSocketRecvBuffer()->bufferEmpty()) {
    e_->setNoWait(true);
  }
}

HttpServerBodyCommand::~HttpServerBodyCommand()
{
  e_->deleteSocketForReadCheck(socket_, this);
}

bool HttpServerBodyCommand::execute()
{
  if(e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  try {
    if(socket_->isReadable(0) ||
       !httpServer_->getSocketRecvBuffer()->bufferEmpty() ||
       httpServer_->getContentLength() == 0) {
      timeoutTimer_ = global::wallclock;

      if(httpServer_->receiveBody()) {
        A2_LOG_DEBUG(fmt("%s", httpServer_->getBody().c_str()));

        std::string reqPath = httpServer_->getRequestPath();
        reqPath.erase(std::find(reqPath.begin(), reqPath.end(), '#'),
                      reqPath.end());
        std::string query(std::find(reqPath.begin(), reqPath.end(), '?'),
                          reqPath.end());
        reqPath.erase(reqPath.size()-query.size(), query.size());
        // Do something for requestpath and body
        if(reqPath == "/rpc") {
          xmlrpc::XmlRpcRequest req =
            xmlrpc::XmlRpcRequestProcessor().parseMemory(httpServer_->getBody());
          
          SharedHandle<xmlrpc::XmlRpcMethod> method =
            xmlrpc::XmlRpcMethodFactory::create(req.methodName);
          xmlrpc::XmlRpcResponse res = method->execute(req, e_);
          bool gzip = httpServer_->supportsGZip();
          std::string responseData = res.toXml(gzip);
          httpServer_->feedResponse(responseData, "text/xml");
          Command* command =
            new HttpServerResponseCommand(getCuid(), httpServer_, e_, socket_);
          e_->addCommand(command);
          e_->setNoWait(true);
          return true;
        } else if(reqPath == "/jsonrpc") {
          // TODO handle query parameter
          std::string callback;// = "callback";

          SharedHandle<ValueBase> json = json::decode(httpServer_->getBody());
          const Dict* jsondict = asDict(json);
          if(!jsondict) {
            // TODO code: -32600, Invalid Request
            throw DL_ABORT_EX("JSON-RPC Invalid Request");
          }
          const String* methodName = asString(jsondict->get("method"));
          if(!methodName) {
            // TODO Batch request does not have method
            throw DL_ABORT_EX("JSON-RPC No Method Found");
          }
          SharedHandle<List> params;
          const SharedHandle<ValueBase>& tempParams = jsondict->get("params");
          if(asList(tempParams)) {
            params = static_pointer_cast<List>(tempParams);
          } else if(!tempParams) {
            params = List::g();
          } else {
            // TODO No support for Named params
            throw DL_ABORT_EX("JSON-RPC Named params are not supported");
          }
          SharedHandle<ValueBase> id = jsondict->get("id");
          if(!id) {
            // TODO Batch request does not have id
            throw DL_ABORT_EX("JSON-RPC NO id found");
          }
          xmlrpc::XmlRpcRequest req(methodName->s(), params, id);

          SharedHandle<xmlrpc::XmlRpcMethod> method =
            xmlrpc::XmlRpcMethodFactory::create(req.methodName);
          method->setJsonRpc(true);
          xmlrpc::XmlRpcResponse res = method->execute(req, e_);
          bool gzip = httpServer_->supportsGZip();
          std::string responseData = res.toJson(callback, gzip);
          httpServer_->feedResponse(responseData, "application/json-rpc");
          Command* command =
            new HttpServerResponseCommand(getCuid(), httpServer_, e_, socket_);
          e_->addCommand(command);
          e_->setNoWait(true);
          return true;
        } else {
          return true;
        }
      } else {
        e_->addCommand(this);
        return false;
      } 
    } else {
      if(timeoutTimer_.difference(global::wallclock) >= 30) {
        A2_LOG_INFO("HTTP request body timeout.");
        return true;
      } else {
        e_->addCommand(this);
        return false;
      }
    }
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX
      (fmt("CUID#%lld - Error occurred while reading HTTP request body",
           getCuid()),
       e);
    return true;
  }

}

} // namespace aria2
