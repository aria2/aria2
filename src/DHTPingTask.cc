/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
#include "DHTPingTask.h"
#include "DHTMessage.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTNode.h"
#include "DHTConstants.h"
#include "DHTPingReplyMessageCallback.h"
#include "DHTQueryMessage.h"
#include "DHTPingMessage.h"

namespace aria2 {

DHTPingTask::DHTPingTask(const std::shared_ptr<DHTNode>& remoteNode,
                         int numMaxRetry)
    : remoteNode_(remoteNode),
      numMaxRetry_(numMaxRetry),
      numRetry_(0),
      pingSuccessful_(false),
      timeout_(DHT_MESSAGE_TIMEOUT)
{
}

DHTPingTask::~DHTPingTask() = default;

void DHTPingTask::addMessage()
{
  getMessageDispatcher()->addMessageToQueue(
      getMessageFactory()->createPingMessage(remoteNode_), timeout_,
      make_unique<DHTPingReplyMessageCallback<DHTPingTask>>(this));
}

void DHTPingTask::startup() { addMessage(); }

void DHTPingTask::onReceived(const DHTPingReplyMessage* message)
{
  pingSuccessful_ = true;
  setFinished(true);
}

void DHTPingTask::onTimeout(const std::shared_ptr<DHTNode>& node)
{
  ++numRetry_;
  if (numRetry_ >= numMaxRetry_) {
    pingSuccessful_ = false;
    setFinished(true);
  }
  else {
    addMessage();
  }
}

bool DHTPingTask::isPingSuccessful() const { return pingSuccessful_; }

} // namespace aria2
