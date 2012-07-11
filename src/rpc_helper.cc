/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#include "rpc_helper.h"
#include "XmlParser.h"
#include "RpcRequest.h"
#include "XmlRpcRequestParserStateMachine.h"
#include "message.h"
#include "DlAbortEx.h"
#include "DownloadEngine.h"
#include "RpcMethod.h"
#include "RpcResponse.h"
#include "RpcMethodFactory.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

namespace rpc {

#ifdef ENABLE_XML_RPC
RpcRequest xmlParseMemory(const char* xml, size_t size)
{
  XmlRpcRequestParserStateMachine psm;
  if(xml::XmlParser(&psm).parseFinal(xml, size) < 0) {
    throw DL_ABORT_EX(MSG_CANNOT_PARSE_XML_RPC_REQUEST);
  }
  SharedHandle<List> params;
  if(downcast<List>(psm.getCurrentFrameValue())) {
    params = static_pointer_cast<List>(psm.getCurrentFrameValue());
  } else {
    params = List::g();
  }
  return RpcRequest(psm.getMethodName(), params);
}
#endif // ENABLE_XML_RPC

RpcResponse createJsonRpcErrorResponse(int code,
                                       const std::string& msg,
                                       const SharedHandle<ValueBase>& id)
{
  SharedHandle<Dict> params = Dict::g();
  params->put("code", Integer::g(code));
  params->put("message", msg);
  rpc::RpcResponse res(code, params, id);
  return res;
}

RpcResponse processJsonRpcRequest(const Dict* jsondict, DownloadEngine* e)
{
  SharedHandle<ValueBase> id = jsondict->get("id");
  if(!id) {
    return createJsonRpcErrorResponse(-32600, "Invalid Request.", Null::g());
  }
  const String* methodName = downcast<String>(jsondict->get("method"));
  if(!methodName) {
    return createJsonRpcErrorResponse(-32600, "Invalid Request.", id);
  }
  SharedHandle<List> params;
  const SharedHandle<ValueBase>& tempParams = jsondict->get("params");
  if(downcast<List>(tempParams)) {
    params = static_pointer_cast<List>(tempParams);
  } else if(!tempParams) {
    params = List::g();
  } else {
    // TODO No support for Named params
    return createJsonRpcErrorResponse(-32602, "Invalid params.", id);
  }
  rpc::RpcRequest req(methodName->s(), params, id);
  req.jsonRpc = true;
  SharedHandle<rpc::RpcMethod> method;
  try {
    method = rpc::RpcMethodFactory::create(req.methodName);
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return createJsonRpcErrorResponse(-32601, "Method not found.", id);
  }
  A2_LOG_INFO(fmt("Executing RPC method %s", req.methodName.c_str()));
  rpc::RpcResponse res = method->execute(req, e);
  return res;
}

} // namespace rpc

} // namespace aria2
