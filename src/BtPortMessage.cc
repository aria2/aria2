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
#include "BtPortMessage.h"
#include "bittorrent_helper.h"
#include "DlAbortEx.h"
#include "util.h"
#include "message.h"
#include "Logger.h"
#include "LogFactory.h"
#include "Peer.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "fmt.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtPortMessage::NAME("port");

BtPortMessage::BtPortMessage(uint16_t port)
  : SimpleBtMessage(ID, NAME),
    port_(port),
    localNode_(0),
    routingTable_(0),
    taskQueue_(0),
    taskFactory_(0)
{}

SharedHandle<BtPortMessage> BtPortMessage::create
(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthEqual(3, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  uint16_t port = bittorrent::getShortIntParam(data, 1);
  SharedHandle<BtPortMessage> message(new BtPortMessage(port));
  return message;
}

void BtPortMessage::doReceivedAction()
{
  if(taskFactory_ && taskQueue_) {
    if(port_ == 0) {
      A2_LOG_DEBUG("Ignored port 0.");
      return;
    }
    // node id is random at this point. When ping reply received, new DHTNode
    // instance created with proper node ID and is added to a routing table.
    SharedHandle<DHTNode> node(new DHTNode());
    node->setIPAddress(getPeer()->getIPAddress());
    node->setPort(port_);
    {
      SharedHandle<DHTTask> task = taskFactory_->createPingTask(node);
      taskQueue_->addImmediateTask(task);
    }
    if(routingTable_->getNumBucket() == 1) {
      // initiate bootstrap
      A2_LOG_INFO("Dispatch node_lookup since too few buckets.");
      taskQueue_->addImmediateTask
        (taskFactory_->createNodeLookupTask(localNode_->getID()));
    }
  } else {
    A2_LOG_INFO
      ("DHT port message received while localhost didn't declare support it.");
  }
}

unsigned char* BtPortMessage::createMessage()
{
  /**
   * len --- 5, 4bytes
   * id --- 4, 1byte
   * port --- port number, 2bytes
   * total: 7bytes
   */
  unsigned char* msg = new unsigned char[MESSAGE_LENGTH];
  bittorrent::createPeerMessageString(msg, MESSAGE_LENGTH, 3, ID);
  bittorrent::setShortIntParam(&msg[5], port_);
  return msg;
}

size_t BtPortMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

std::string BtPortMessage::toString() const {
  return fmt("%s port=%u", NAME.c_str(), port_);
}

void BtPortMessage::setLocalNode(DHTNode* localNode)
{
  localNode_ = localNode;
}

void BtPortMessage::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

void BtPortMessage::setTaskQueue(DHTTaskQueue* taskQueue)
{
  taskQueue_ = taskQueue;
}

void BtPortMessage::setTaskFactory(DHTTaskFactory* taskFactory)
{
  taskFactory_ = taskFactory;
}

} // namespace aria2
