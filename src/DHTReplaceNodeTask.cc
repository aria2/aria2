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
#include "DHTReplaceNodeTask.h"
#include "DHTBucket.h"
#include "DHTNode.h"
#include "DHTPingReplyMessage.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "Logger.h"
#include "LogFactory.h"
#include "DHTPingReplyMessageCallback.h"
#include "DHTQueryMessage.h"
#include "DHTPingMessage.h"
#include "fmt.h"

namespace aria2 {

DHTReplaceNodeTask::DHTReplaceNodeTask(const std::shared_ptr<DHTBucket>& bucket,
                                       const std::shared_ptr<DHTNode>& newNode)
    : bucket_(bucket),
      newNode_(newNode),
      numRetry_(0),
      timeout_(DHT_MESSAGE_TIMEOUT)
{
}

DHTReplaceNodeTask::~DHTReplaceNodeTask() = default;

void DHTReplaceNodeTask::startup() { sendMessage(); }

void DHTReplaceNodeTask::sendMessage()
{
  std::shared_ptr<DHTNode> questionableNode = bucket_->getLRUQuestionableNode();
  if (!questionableNode) {
    setFinished(true);
  }
  else {
    getMessageDispatcher()->addMessageToQueue(
        getMessageFactory()->createPingMessage(questionableNode), timeout_,
        make_unique<DHTPingReplyMessageCallback<DHTReplaceNodeTask>>(this));
  }
}

void DHTReplaceNodeTask::onReceived(const DHTPingReplyMessage* message)
{
  A2_LOG_INFO(fmt("ReplaceNode: Ping reply received from %s.",
                  message->getRemoteNode()->toString().c_str()));
  setFinished(true);
}

namespace {
const int MAX_RETRY = 2;
} // namespace

void DHTReplaceNodeTask::onTimeout(const std::shared_ptr<DHTNode>& node)
{
  ++numRetry_;
  if (numRetry_ >= MAX_RETRY) {
    A2_LOG_INFO(fmt("ReplaceNode: Ping failed %d times. Replace %s with %s.",
                    numRetry_, node->toString().c_str(),
                    newNode_->toString().c_str()));
    node->markBad();
    bucket_->addNode(newNode_);
    setFinished(true);
  }
  else {
    A2_LOG_INFO(fmt("ReplaceNode: Ping reply timeout from %s. Try once more.",
                    node->toString().c_str()));
    sendMessage();
  }
}

} // namespace aria2
