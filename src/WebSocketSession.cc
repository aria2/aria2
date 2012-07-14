/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "WebSocketSession.h"

#include <cerrno>
#include <cstring>

#include "SocketCore.h"
#include "LogFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "DownloadEngine.h"
#include "rpc_helper.h"
#include "RpcResponse.h"
#include "json.h"
#include "prefs.h"
#include "Option.h"

namespace aria2 {

namespace rpc {

namespace {
ssize_t sendCallback(wslay_event_context_ptr wsctx,
                      const uint8_t* data, size_t len, int flags,
                      void* userData)
{
  WebSocketSession* session = reinterpret_cast<WebSocketSession*>(userData);
  const SharedHandle<SocketCore>& socket = session->getSocket();
  try {
    ssize_t r = socket->writeData(data, len);
    if(r == 0) {
      if(socket->wantRead() || socket->wantWrite()) {
        wslay_event_set_error(wsctx, WSLAY_ERR_WOULDBLOCK);
      } else {
        wslay_event_set_error(wsctx, WSLAY_ERR_CALLBACK_FAILURE);
      }
      r = -1;
    }
    return r;
  } catch(RecoverableException& e) {
    A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, e);
    wslay_event_set_error(wsctx, WSLAY_ERR_CALLBACK_FAILURE);
    return -1;
  }
}
} // namespace

namespace {
ssize_t recvCallback(wslay_event_context_ptr wsctx,
                     uint8_t* buf, size_t len, int flags,
                     void* userData)
{
  WebSocketSession* session = reinterpret_cast<WebSocketSession*>(userData);
  const SharedHandle<SocketCore>& socket = session->getSocket();
  try {
    ssize_t r;
    socket->readData(buf, len);
    if(len == 0) {
      if(socket->wantRead() || socket->wantWrite()) {
        wslay_event_set_error(wsctx, WSLAY_ERR_WOULDBLOCK);
      } else {
        wslay_event_set_error(wsctx, WSLAY_ERR_CALLBACK_FAILURE);
      }
      r = -1;
    } else {
      r = len;
    }
    return r;
  } catch(RecoverableException& e) {
    A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, e);
    wslay_event_set_error(wsctx, WSLAY_ERR_CALLBACK_FAILURE);
    return -1;
  }
}
} // namespace

namespace {
void addResponse(WebSocketSession* wsSession, const RpcResponse& res)
{
  std::string response = toJson(res, "", false);
  wsSession->addTextMessage(response);
}
} // namespace

namespace {
void addResponse(WebSocketSession* wsSession,
                 const std::vector<RpcResponse>& results)
{
  std::string response = toJsonBatch(results, "", false);
  wsSession->addTextMessage(response);
}
} // namespace

namespace {
void onFrameRecvStartCallback
(wslay_event_context_ptr wsctx,
 const struct wslay_event_on_frame_recv_start_arg* arg,
 void* userData)
{
  WebSocketSession* wsSession = reinterpret_cast<WebSocketSession*>(userData);
  wsSession->setIgnorePayload(wslay_is_ctrl_frame(arg->opcode));
}
} // namespace

namespace {
void onFrameRecvChunkCallback
(wslay_event_context_ptr wsctx,
 const struct wslay_event_on_frame_recv_chunk_arg* arg,
 void* userData)
{
  WebSocketSession* wsSession = reinterpret_cast<WebSocketSession*>(userData);
  if(!wsSession->getIgnorePayload()) {
    // The return value is ignored here. It will be evaluated in
    // onMsgRecvCallback.
    wsSession->parseUpdate(arg->data, arg->data_length);
  }
}
} // namespace

namespace {
void onMsgRecvCallback(wslay_event_context_ptr wsctx,
                       const struct wslay_event_on_msg_recv_arg* arg,
                       void* userData)
{
  WebSocketSession* wsSession = reinterpret_cast<WebSocketSession*>(userData);
  if(!wslay_is_ctrl_frame(arg->opcode)) {
    // TODO Only process text frame
    ssize_t error = 0;
    SharedHandle<ValueBase> json = wsSession->parseFinal(0, 0, error);
    if(error < 0) {
      A2_LOG_INFO("Failed to parse JSON-RPC request");
      RpcResponse res
        (createJsonRpcErrorResponse(-32700, "Parse error.", Null::g()));
      addResponse(wsSession, res);
      return;
    }
    const Dict* jsondict = downcast<Dict>(json);
    if(jsondict) {
      RpcResponse res = processJsonRpcRequest(jsondict,
                                              wsSession->getDownloadEngine());
      addResponse(wsSession, res);
    } else {
      const List* jsonlist = downcast<List>(json);
      if(jsonlist) {
        // This is batch call
        std::vector<RpcResponse> results;
        for(List::ValueType::const_iterator i = jsonlist->begin(),
              eoi = jsonlist->end(); i != eoi; ++i) {
          const Dict* jsondict = downcast<Dict>(*i);
          if(jsondict) {
            RpcResponse r = processJsonRpcRequest
              (jsondict, wsSession->getDownloadEngine());
            results.push_back(r);
          }
        }
        addResponse(wsSession, results);
      } else {
        RpcResponse res(createJsonRpcErrorResponse
                        (-32600, "Invalid Request.", Null::g()));
        addResponse(wsSession, res);
      }
    }
  } else {
    RpcResponse res(createJsonRpcErrorResponse
                    (-32600, "Invalid Request.", Null::g()));
    addResponse(wsSession, res);
  }
}
} // namespace

WebSocketSession::WebSocketSession(const SharedHandle<SocketCore>& socket,
                                   DownloadEngine* e)
  : socket_(socket),
    e_(e),
    ignorePayload_(false),
    receivedLength_(0)
{
  wslay_event_callbacks callbacks;
  memset(&callbacks, 0, sizeof(wslay_event_callbacks));
  callbacks.recv_callback = recvCallback;
  callbacks.send_callback = sendCallback;
  callbacks.on_msg_recv_callback = onMsgRecvCallback;
  callbacks.on_frame_recv_start_callback = onFrameRecvStartCallback;
  callbacks.on_frame_recv_chunk_callback = onFrameRecvChunkCallback;

  int r = wslay_event_context_server_init(&wsctx_, &callbacks, this);
  assert(r == 0);
  wslay_event_config_set_no_buffering(wsctx_, 1);
}
    
WebSocketSession::~WebSocketSession()
{
  wslay_event_context_free(wsctx_);
}

bool WebSocketSession::wantRead()
{
  return wslay_event_want_read(wsctx_);
}

bool WebSocketSession::wantWrite()
{
  return wslay_event_want_write(wsctx_);
}

bool WebSocketSession::finish()
{
  return !wantRead() && !wantWrite();
}

int WebSocketSession::onReadEvent()
{
  if(wslay_event_recv(wsctx_) == 0) {
    return 0;
  } else {
    return -1;
  }
}

int WebSocketSession::onWriteEvent()
{
  if(wslay_event_send(wsctx_) == 0) {
    return 0;
  } else {
    return -1;
  }
}

void WebSocketSession::addTextMessage(const std::string& msg)
{
  // TODO Don't add text message if the size of outbound queue in
  // wsctx_ exceeds certain limit.
  wslay_event_msg arg = {
    WSLAY_TEXT_FRAME, reinterpret_cast<const uint8_t*>(msg.c_str()), msg.size()
  };
  wslay_event_queue_msg(wsctx_, &arg);
}

bool WebSocketSession::closeReceived()
{
  return wslay_event_get_close_received(wsctx_);
}

bool WebSocketSession::closeSent()
{
  return wslay_event_get_close_sent(wsctx_);
}

ssize_t WebSocketSession::parseUpdate(const uint8_t* data, size_t len)
{
  // Cap the number of bytes to feed the parser
  size_t maxlen = e_->getOption()->getAsInt(PREF_RPC_MAX_REQUEST_SIZE);
  if(receivedLength_ + len <= maxlen) {
    receivedLength_ += len;
  } else {
    len = 0;
  }
  return parser_.parseUpdate(reinterpret_cast<const char*>(data), len);
}

SharedHandle<ValueBase> WebSocketSession::parseFinal
(const uint8_t* data, size_t len, ssize_t& error)
{
  SharedHandle<ValueBase> res =
    parser_.parseFinal(reinterpret_cast<const char*>(data), len, error);
  receivedLength_ = 0;
  return res;
}

} // namespace rpc

} // namespace aria2
