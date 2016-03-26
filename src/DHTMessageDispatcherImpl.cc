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
#include "DHTMessageDispatcherImpl.h"
#include "DHTMessage.h"
#include "DHTMessageCallback.h"
#include "DHTMessageEntry.h"
#include "DHTMessageTracker.h"
#include "RecoverableException.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DHTConstants.h"
#include "fmt.h"
#include "DHTNode.h"
#include "a2functional.h"

namespace aria2 {

DHTMessageDispatcherImpl::DHTMessageDispatcherImpl(
    const std::shared_ptr<DHTMessageTracker>& tracker)
    : tracker_{tracker}, timeout_{DHT_MESSAGE_TIMEOUT}
{
}

void DHTMessageDispatcherImpl::addMessageToQueue(
    std::unique_ptr<DHTMessage> message, std::chrono::seconds timeout,
    std::unique_ptr<DHTMessageCallback> callback)
{
  messageQueue_.push_back(make_unique<DHTMessageEntry>(
      std::move(message), std::move(timeout), std::move(callback)));
}

void DHTMessageDispatcherImpl::addMessageToQueue(
    std::unique_ptr<DHTMessage> message,
    std::unique_ptr<DHTMessageCallback> callback)
{
  addMessageToQueue(std::move(message), timeout_, std::move(callback));
}

bool DHTMessageDispatcherImpl::sendMessage(DHTMessageEntry* entry)
{
  try {
    if (entry->message->send()) {
      if (!entry->message->isReply()) {
        tracker_->addMessage(entry->message.get(), entry->timeout,
                             std::move(entry->callback));
      }
      A2_LOG_INFO(fmt("Message sent: %s", entry->message->toString().c_str()));
    }
    else {
      return false;
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(
        fmt("Failed to send message: %s", entry->message->toString().c_str()),
        e);
    // Add message to DHTMessageTracker with timeout 0 to treat it as
    // time out. Without this, we have untracked message and some of
    // DHTTask(such as DHTAbstractNodeLookupTask) don't finish
    // forever.
    if (!entry->message->isReply()) {
      tracker_->addMessage(entry->message.get(), 0_s,
                           std::move(entry->callback));
    }
  }
  return true;
}

void DHTMessageDispatcherImpl::sendMessages()
{
  auto itr = std::begin(messageQueue_);
  for (; itr != std::end(messageQueue_); ++itr) {
    if (!sendMessage((*itr).get())) {
      break;
    }
  }
  messageQueue_.erase(std::begin(messageQueue_), itr);
  A2_LOG_DEBUG(fmt("%lu dht messages remaining in the queue.",
                   static_cast<unsigned long>(messageQueue_.size())));
}

size_t DHTMessageDispatcherImpl::countMessageInQueue() const
{
  return messageQueue_.size();
}

} // namespace aria2
