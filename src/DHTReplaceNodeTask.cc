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
#include "DHTPingReplyMessageCallback.h"
#include "DHTQueryMessage.h"

namespace aria2 {

DHTReplaceNodeTask::DHTReplaceNodeTask(const SharedHandle<DHTBucket>& bucket,
                                       const SharedHandle<DHTNode>& newNode):
  _bucket(bucket),
  _newNode(newNode),
  _numRetry(0),
  _timeout(DHT_MESSAGE_TIMEOUT)
{}

DHTReplaceNodeTask::~DHTReplaceNodeTask() {}

void DHTReplaceNodeTask::startup()
{
  sendMessage();
}

void DHTReplaceNodeTask::sendMessage()
{
  SharedHandle<DHTNode> questionableNode = _bucket->getLRUQuestionableNode();
  if(questionableNode.isNull()) {
    setFinished(true);
  } else {
    SharedHandle<DHTMessage> m =
      getMessageFactory()->createPingMessage(questionableNode);
    SharedHandle<DHTMessageCallback> callback
      (new DHTPingReplyMessageCallback<DHTReplaceNodeTask>(this));
    getMessageDispatcher()->addMessageToQueue(m, _timeout, callback);
  }
}

void DHTReplaceNodeTask::onReceived(const DHTPingReplyMessage* message)
{
  getLogger()->info("ReplaceNode: Ping reply received from %s.",
                    message->getRemoteNode()->toString().c_str());
  setFinished(true);
}

void DHTReplaceNodeTask::onTimeout(const SharedHandle<DHTNode>& node)
{
  ++_numRetry;
  if(_numRetry >= MAX_RETRY) {
    getLogger()->info("ReplaceNode: Ping failed %u times. Replace %s with %s.",
                      _numRetry, node->toString().c_str(),
                      _newNode->toString().c_str());
    node->markBad();
    _bucket->addNode(_newNode);
    setFinished(true);
  } else {
    getLogger()->info("ReplaceNode: Ping reply timeout from %s. Try once more.",
                      node->toString().c_str());
    sendMessage();
  }
}

} // namespace aria2
