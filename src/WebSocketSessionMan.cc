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
#include "WebSocketSessionMan.h"

#include <cassert>

#include "WebSocketSession.h"
#include "RequestGroup.h"
#include "json.h"
#include "util.h"
#include "WebSocketInteractionCommand.h"
#include "LogFactory.h"

namespace aria2 {

namespace rpc {

WebSocketSessionMan::WebSocketSessionMan() {}

WebSocketSessionMan::~WebSocketSessionMan() = default;

void WebSocketSessionMan::addSession(
    const std::shared_ptr<WebSocketSession>& wsSession)
{
  A2_LOG_DEBUG("WebSocket session added.");
  sessions_.insert(wsSession);
}

void WebSocketSessionMan::removeSession(
    const std::shared_ptr<WebSocketSession>& wsSession)
{
  A2_LOG_DEBUG("WebSocket session removed.");
  sessions_.erase(wsSession);
}

void WebSocketSessionMan::addNotification(const std::string& method,
                                          const RequestGroup* group)
{
  auto dict = Dict::g();
  dict->put("jsonrpc", "2.0");
  dict->put("method", method);
  auto eventSpec = Dict::g();
  eventSpec->put("gid", GroupId::toHex((group->getGID())));
  auto params = List::g();
  params->append(std::move(eventSpec));
  dict->put("params", std::move(params));
  std::string msg = json::encode(dict.get());
  for (auto& session : sessions_) {
    session->addTextMessage(msg, false);
    session->getCommand()->updateWriteCheck();
  }
}

namespace {
// The string constants for download events.
const std::string ON_DOWNLOAD_START = "aria2.onDownloadStart";
const std::string ON_DOWNLOAD_PAUSE = "aria2.onDownloadPause";
const std::string ON_DOWNLOAD_STOP = "aria2.onDownloadStop";
const std::string ON_DOWNLOAD_COMPLETE = "aria2.onDownloadComplete";
const std::string ON_DOWNLOAD_ERROR = "aria2.onDownloadError";
const std::string ON_BT_DOWNLOAD_COMPLETE = "aria2.onBtDownloadComplete";
} // namespace

namespace {
const std::string& getMethodName(DownloadEvent event)
{
  switch (event) {
  case EVENT_ON_DOWNLOAD_START:
    return ON_DOWNLOAD_START;
  case EVENT_ON_DOWNLOAD_PAUSE:
    return ON_DOWNLOAD_PAUSE;
  case EVENT_ON_DOWNLOAD_STOP:
    return ON_DOWNLOAD_STOP;
  case EVENT_ON_DOWNLOAD_COMPLETE:
    return ON_DOWNLOAD_COMPLETE;
  case EVENT_ON_DOWNLOAD_ERROR:
    return ON_DOWNLOAD_ERROR;
  case EVENT_ON_BT_DOWNLOAD_COMPLETE:
    return ON_BT_DOWNLOAD_COMPLETE;
  default:
    // Not reachable
    assert(0);
    // For suppress compiler warning
    return A2STR::NIL;
  }
}
} // namespace

void WebSocketSessionMan::onEvent(DownloadEvent event,
                                  const RequestGroup* group)
{
  addNotification(getMethodName(event), group);
}

} // namespace rpc

} // namespace aria2
