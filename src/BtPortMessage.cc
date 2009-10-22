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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Util.h"
#include "message.h"
#include "Logger.h"
#include "Peer.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "StringFormat.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtPortMessage::NAME("port");

BtPortMessage::BtPortMessage(uint16_t port):
  SimpleBtMessage(ID, NAME), _port(port), _msg(0) {}

BtPortMessage::~BtPortMessage()
{
  delete [] _msg;
}

SharedHandle<BtPortMessage> BtPortMessage::create(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthEqual(3, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  uint16_t port = bittorrent::getShortIntParam(data, 1);
  SharedHandle<BtPortMessage> message(new BtPortMessage(port));
  return message;
}

void BtPortMessage::doReceivedAction()
{
  if(!_taskFactory.isNull() && !_taskQueue.isNull()) {
    // node id is random at this point. When ping reply received, new DHTNode
    // instance created with proper node ID and is added to a routing table.
    SharedHandle<DHTNode> node(new DHTNode());
    node->setIPAddress(peer->ipaddr);
    node->setPort(_port);
    {
      SharedHandle<DHTTask> task = _taskFactory->createPingTask(node);
      _taskQueue->addImmediateTask(task);
    }
    if(_routingTable->countBucket() == 1) {
      // initiate bootstrap
      logger->info("Dispatch node_lookup since too few buckets.");
      _taskQueue->addImmediateTask(_taskFactory->createNodeLookupTask(_localNode->getID()));
    }
  } else {
    logger->info("DHT port message received while localhost didn't declare support it.");
  }
}

const unsigned char* BtPortMessage::getMessage() {
  if(!_msg) {
    /**
     * len --- 5, 4bytes
     * id --- 4, 1byte
     * port --- port number, 2bytes
     * total: 7bytes
     */
    _msg = new unsigned char[MESSAGE_LENGTH];
    bittorrent::createPeerMessageString(_msg, MESSAGE_LENGTH, 3, ID);
    bittorrent::setShortIntParam(&_msg[5], _port);
  }
  return _msg;
}

size_t BtPortMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

std::string BtPortMessage::toString() const {
  return strconcat(NAME, " port=", util::uitos(_port));
}

void BtPortMessage::setLocalNode(const WeakHandle<DHTNode>& localNode)
{
  _localNode = localNode;
}

void BtPortMessage::setRoutingTable(const WeakHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

void BtPortMessage::setTaskQueue(const WeakHandle<DHTTaskQueue>& taskQueue)
{
  _taskQueue = taskQueue;
}

void BtPortMessage::setTaskFactory(const WeakHandle<DHTTaskFactory>& taskFactory)
{
  _taskFactory = taskFactory;
}

} // namespace aria2
