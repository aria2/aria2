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
#include "wallclock.h"
#include "util.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "json.h"
#include "DlAbortEx.h"
#include "message.h"
#include "RpcMethod.h"
#include "RpcMethodFactory.h"
#include "RpcRequest.h"
#include "RpcResponse.h"
#include "rpc_helper.h"
#include "JsonDiskWriter.h"
#include "ValueBaseJsonParser.h"
#ifdef ENABLE_XML_RPC
#  include "XmlRpcRequestParserStateMachine.h"
#  include "XmlRpcDiskWriter.h"
#endif // ENABLE_XML_RPC

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

namespace {
std::string getJsonRpcContentType(bool script)
{
  return script ? "text/javascript" : "application/json-rpc";
}
} // namespace

void HttpServerBodyCommand::sendJsonRpcResponse
(const rpc::RpcResponse& res,
 const std::string& callback)
{
  bool gzip = httpServer_->supportsGZip();
  std::string responseData = rpc::toJson(res, callback, gzip);
  if(res.code == 0) {
    httpServer_->feedResponse(responseData,
                              getJsonRpcContentType(!callback.empty()));
  } else {
    httpServer_->disableKeepAlive();
    int httpCode;
    switch(res.code) {
    case -32600:
      httpCode = 400;
      break;
    case -32601:
      httpCode = 404;
      break;
    default:
      httpCode = 500;
    };
    httpServer_->feedResponse(httpCode, A2STR::NIL,
                              responseData,
                              getJsonRpcContentType(!callback.empty()));
  }
  addHttpServerResponseCommand();
}

void HttpServerBodyCommand::sendJsonRpcBatchResponse
(const std::vector<rpc::RpcResponse>& results,
 const std::string& callback)
{
  bool gzip = httpServer_->supportsGZip();
  std::string responseData = rpc::toJsonBatch(results, callback, gzip);
  httpServer_->feedResponse(responseData,
                            getJsonRpcContentType(!callback.empty()));
  addHttpServerResponseCommand();
}

void HttpServerBodyCommand::addHttpServerResponseCommand()
{
  Command* command =
    new HttpServerResponseCommand(getCuid(), httpServer_, e_, socket_);
  e_->addCommand(command);
  e_->setNoWait(true);
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
      timeoutTimer_ = global::wallclock();

      if(httpServer_->receiveBody()) {
        std::string reqPath = httpServer_->getRequestPath();
        reqPath.erase(std::find(reqPath.begin(), reqPath.end(), '#'),
                      reqPath.end());
        std::string query(std::find(reqPath.begin(), reqPath.end(), '?'),
                          reqPath.end());
        reqPath.erase(reqPath.size()-query.size(), query.size());

        if(httpServer_->getMethod() == "OPTIONS") {
          // Response to Preflight Request.
          // See http://www.w3.org/TR/cors/
          const SharedHandle<HttpHeader>& header =
            httpServer_->getRequestHeader();
          std::string accessControlHeaders;
          if(!header->find("origin").empty() &&
             !header->find("access-control-request-method").empty() &&
             !httpServer_->getAllowOrigin().empty()) {
            accessControlHeaders +=
              "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
              "Access-Control-Max-Age: 1728000\r\n";
            const std::string& accReqHeaders =
              header->find("access-control-request-headers");
            if(!accReqHeaders.empty()) {
              // We allow all headers requested.
              accessControlHeaders += "Access-Control-Allow-Headers: ";
              accessControlHeaders += accReqHeaders;
              accessControlHeaders += "\r\n";
            }
          }
          httpServer_->feedResponse(200, accessControlHeaders);
          addHttpServerResponseCommand();
          return true;
        }

        // Do something for requestpath and body
        switch(httpServer_->getRequestType()) {
        case RPC_TYPE_XML: {
#ifdef ENABLE_XML_RPC
          SharedHandle<rpc::XmlRpcDiskWriter> dw =
            static_pointer_cast<rpc::XmlRpcDiskWriter>(httpServer_->getBody());
          int error;
          error = dw->finalize();
          rpc::RpcRequest req;
          if(error == 0) {
            req = dw->getResult();
          }
          dw->reset();
          if(error < 0) {
            A2_LOG_INFO
              (fmt("CUID#%" PRId64 " - Failed to parse XML-RPC request",
                   getCuid()));
            httpServer_->feedResponse(400);
            addHttpServerResponseCommand();
            return true;
          }
          SharedHandle<rpc::RpcMethod> method =
            rpc::RpcMethodFactory::create(req.methodName);
          A2_LOG_INFO(fmt("Executing RPC method %s", req.methodName.c_str()));
          rpc::RpcResponse res = method->execute(req, e_);
          bool gzip = httpServer_->supportsGZip();
          std::string responseData = rpc::toXml(res, gzip);
          httpServer_->feedResponse(responseData, "text/xml");
          addHttpServerResponseCommand();
#else // !ENABLE_XML_RPC
          httpServer_->feedResponse(404);
          addHttpServerResponseCommand();
#endif // !ENABLE_XML_RPC
          return true;
        }
        case RPC_TYPE_JSON:
        case RPC_TYPE_JSONP: {
          std::string callback;
          SharedHandle<ValueBase> json;
          ssize_t error = 0;
          if(httpServer_->getRequestType() == RPC_TYPE_JSONP) {
            json::JsonGetParam param = json::decodeGetParams(query);
            callback = param.callback;
            ssize_t error = 0;
            json = json::ValueBaseJsonParser().parseFinal
              (param.request.c_str(),
               param.request.size(),
               error);
          } else {
            SharedHandle<json::JsonDiskWriter> dw =
              static_pointer_cast<json::JsonDiskWriter>(httpServer_->getBody());
            error = dw->finalize();
            if(error == 0) {
              json = dw->getResult();
            }
            dw->reset();
          }
          if(error < 0) {
            A2_LOG_INFO
              (fmt("CUID#%" PRId64 " - Failed to parse JSON-RPC request",
                   getCuid()));
            rpc::RpcResponse res
              (rpc::createJsonRpcErrorResponse(-32700, "Parse error.",
                                               Null::g()));
            sendJsonRpcResponse(res, callback);
            return true;
          }
          const Dict* jsondict = downcast<Dict>(json);
          if(jsondict) {
            rpc::RpcResponse res = rpc::processJsonRpcRequest(jsondict, e_);
            sendJsonRpcResponse(res, callback);
          } else {
            const List* jsonlist = downcast<List>(json);
            if(jsonlist) {
              // This is batch call
              std::vector<rpc::RpcResponse> results;
              for(List::ValueType::const_iterator i = jsonlist->begin(),
                    eoi = jsonlist->end(); i != eoi; ++i) {
                const Dict* jsondict = downcast<Dict>(*i);
                if(jsondict) {
                  rpc::RpcResponse r =
                    rpc::processJsonRpcRequest(jsondict, e_);
                  results.push_back(r);
                }
              }
              sendJsonRpcBatchResponse(results, callback);
            } else {
              rpc::RpcResponse res
                (rpc::createJsonRpcErrorResponse
                 (-32600, "Invalid Request.", Null::g()));
              sendJsonRpcResponse(res, callback);
            }
          }
          return true;
        }
        default:
          httpServer_->feedResponse(404);
          addHttpServerResponseCommand();
          return true;
        }
      } else {
        e_->addCommand(this);
        return false;
      } 
    } else {
      if(timeoutTimer_.difference(global::wallclock()) >= 30) {
        A2_LOG_INFO("HTTP request body timeout.");
        return true;
      } else {
        e_->addCommand(this);
        return false;
      }
    }
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX
      (fmt("CUID#%" PRId64 " - Error occurred while reading HTTP request body",
           getCuid()),
       e);
    return true;
  }

}

} // namespace aria2
