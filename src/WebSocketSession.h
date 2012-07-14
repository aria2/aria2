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
#ifndef D_WEB_SOCKET_SESSION_H
#define D_WEB_SOCKET_SESSION_H

#include "common.h"

#include <wslay/wslay.h>

#include "SharedHandle.h"
#include "ValueBaseJsonParser.h"

namespace aria2 {

class SocketCore;
class DownloadEngine;

namespace rpc {

class WebSocketInteractionCommand;

class WebSocketSession {
public:
  WebSocketSession(const SharedHandle<SocketCore>& socket,
                   DownloadEngine* e);
  ~WebSocketSession();
  // Returns true if this session object wants to read data from the
  // remote endpoint.
  bool wantRead();
  // Returns true if this session object wants to write data to the
  // remote endpoint.
  bool wantWrite();
  // Returns true if the session ended and the underlying connection
  // can be closed.
  bool finish();
  // Call this function when data is available to read.  This function
  // returns 0 if it succeeds, or -1.
  int onReadEvent();
  // Call this function when data can be sent without blocking.  This
  // function returns 0 if it succeeds, or -1.
  int onWriteEvent();
  // Adds text message |msg|. The message is queued and will be sent
  // in onWriteEvent().
  void addTextMessage(const std::string& msg);
  // Returns true if the close frame is received.
  bool closeReceived();
  // Returns true if the close frame is sent.
  bool closeSent();
  // Parses parital request body. This function returns the number of
  // bytes processed if it succeeds, or negative error code.
  ssize_t parseUpdate(const uint8_t* data, size_t len);
  // Parses final part of request body and returns result.  The
  // |error| will be the number of bytes processed if this function
  // succeeds, or negative error code. Whether success or failure,
  // this function resets parser state and receivedLength_.
  SharedHandle<ValueBase> parseFinal(const uint8_t* data, size_t len,
                                     ssize_t& error);

  const SharedHandle<SocketCore>& getSocket() const
  {
    return socket_;
  }

  DownloadEngine* getDownloadEngine()
  {
    return e_;
  }

  WebSocketInteractionCommand* getCommand()
  {
    return command_;
  }

  void setCommand(WebSocketInteractionCommand* command)
  {
    command_ = command;
  }

  bool getIgnorePayload() const
  {
    return ignorePayload_;
  }

  void setIgnorePayload(bool flag)
  {
    ignorePayload_ = flag;
  }
private:
  SharedHandle<SocketCore> socket_;
  DownloadEngine* e_;
  wslay_event_context_ptr wsctx_;
  bool ignorePayload_;
  int32_t receivedLength_;
  json::ValueBaseJsonParser parser_;
  WebSocketInteractionCommand* command_;
};

} // namespace rpc

} // namespace aria2

#endif // D_WEB_SOCKET_SESSION_H
