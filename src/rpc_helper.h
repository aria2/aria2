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
#ifndef D_RPC_HELPER_H
#define D_RPC_HELPER_H

#include "common.h"

#include <cstdlib>
#include <string>
#include <memory>

#include "RpcRequest.h"

namespace aria2 {

class ValueBase;
class Dict;
class DownloadEngine;

namespace rpc {

struct RpcResponse;

#ifdef ENABLE_XML_RPC
RpcRequest xmlParseMemory(const char* xml, size_t size);
#endif // ENABLE_XML_RPC

// Creates error response. The |code| is the JSON-RPC error code.  The
// |msg| is the error message. The |id| is the id of the request .
RpcResponse createJsonRpcErrorResponse(int code, const std::string& msg,
                                       std::unique_ptr<ValueBase> id);

// Processes JSON-RPC request |jsondict| and returns the result.
RpcResponse processJsonRpcRequest(Dict* jsondict, DownloadEngine* e);

} // namespace rpc

} // namespace aria2

#endif // D_RPC_HELPER_H
