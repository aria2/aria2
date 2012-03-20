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
#include "Notifier.h"
#include "RequestGroup.h"
#include "WebSocketSessionMan.h"
#include "LogFactory.h"

namespace aria2 {

Notifier::Notifier(const SharedHandle<rpc::WebSocketSessionMan>& wsSessionMan)
  : wsSessionMan_(wsSessionMan)
{}

Notifier::~Notifier() {}

void Notifier::addWebSocketSession
(const SharedHandle<rpc::WebSocketSession>& wsSession)
{
  A2_LOG_DEBUG("WebSocket session added.");
  wsSessionMan_->addSession(wsSession);
}

void Notifier::removeWebSocketSession
(const SharedHandle<rpc::WebSocketSession>& wsSession)
{
  A2_LOG_DEBUG("WebSocket session removed.");
  wsSessionMan_->removeSession(wsSession);
}

const std::string Notifier::ON_DOWNLOAD_START = "aria2.onDownloadStart";
const std::string Notifier::ON_DOWNLOAD_PAUSE = "aria2.onDownloadPause";
const std::string Notifier::ON_DOWNLOAD_STOP = "aria2.onDownloadStop";
const std::string Notifier::ON_DOWNLOAD_COMPLETE = "aria2.onDownloadComplete";
const std::string Notifier::ON_DOWNLOAD_ERROR = "aria2.onDownloadError";
const std::string Notifier::ON_BT_DOWNLOAD_COMPLETE =
  "aria2.onBtDownloadComplete";

void Notifier::notifyDownloadEvent
(const std::string& event, const RequestGroup* group)
{
  if(wsSessionMan_) {
    wsSessionMan_->addNotification(event, group);
  }
}

} // namespace aria2
