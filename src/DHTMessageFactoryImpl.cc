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
#include "DHTMessageFactoryImpl.h"

#include <cstring>
#include <utility>

#include "LogFactory.h"
#include "DlAbortEx.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTPingMessage.h"
#include "DHTPingReplyMessage.h"
#include "DHTFindNodeMessage.h"
#include "DHTFindNodeReplyMessage.h"
#include "DHTGetPeersMessage.h"
#include "DHTGetPeersReplyMessage.h"
#include "DHTAnnouncePeerMessage.h"
#include "DHTAnnouncePeerReplyMessage.h"
#include "DHTUnknownMessage.h"
#include "DHTConnection.h"
#include "DHTMessageDispatcher.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTTokenTracker.h"
#include "DHTMessageCallback.h"
#include "bittorrent_helper.h"
#include "BtRuntime.h"
#include "util.h"
#include "Peer.h"
#include "Logger.h"
#include "fmt.h"

namespace aria2 {

DHTMessageFactoryImpl::DHTMessageFactoryImpl(int family)
    : family_{family},
      connection_{nullptr},
      dispatcher_{nullptr},
      routingTable_{nullptr},
      peerAnnounceStorage_{nullptr},
      tokenTracker_{nullptr},
      btRegistry_{nullptr}
{
}

std::shared_ptr<DHTNode> DHTMessageFactoryImpl::getRemoteNode(
    const unsigned char* id, const std::string& ipaddr, uint16_t port) const
{
  auto node = routingTable_->getNode(id, ipaddr, port);
  if (!node) {
    node = std::make_shared<DHTNode>(id);
    node->setIPAddress(ipaddr);
    node->setPort(port);
  }
  return node;
}

namespace {
const Dict* getDictionary(const Dict* dict, const std::string& key)
{
  const Dict* d = downcast<Dict>(dict->get(key));
  if (d) {
    return d;
  }
  else {
    throw DL_ABORT_EX(fmt("Malformed DHT message. Missing %s", key.c_str()));
  }
}
} // namespace

namespace {
const String* getString(const Dict* dict, const std::string& key)
{
  const String* c = downcast<String>(dict->get(key));
  if (c) {
    return c;
  }
  else {
    throw DL_ABORT_EX(fmt("Malformed DHT message. Missing %s", key.c_str()));
  }
}
} // namespace

namespace {
const Integer* getInteger(const Dict* dict, const std::string& key)
{
  const Integer* c = downcast<Integer>(dict->get(key));
  if (c) {
    return c;
  }
  else {
    throw DL_ABORT_EX(fmt("Malformed DHT message. Missing %s", key.c_str()));
  }
}
} // namespace

namespace {
const String* getString(const List* list, size_t index)
{
  const String* c = downcast<String>(list->get(index));
  if (c) {
    return c;
  }
  else {
    throw DL_ABORT_EX(fmt("Malformed DHT message. element[%lu] is not String.",
                          static_cast<unsigned long>(index)));
  }
}
} // namespace

namespace {
const Integer* getInteger(const List* list, size_t index)
{
  const Integer* c = downcast<Integer>(list->get(index));
  if (c) {
    return c;
  }
  else {
    throw DL_ABORT_EX(fmt("Malformed DHT message. element[%lu] is not Integer.",
                          static_cast<unsigned long>(index)));
  }
}
} // namespace

namespace {
const List* getList(const Dict* dict, const std::string& key)
{
  const List* l = downcast<List>(dict->get(key));
  if (l) {
    return l;
  }
  else {
    throw DL_ABORT_EX(fmt("Malformed DHT message. Missing %s", key.c_str()));
  }
}
} // namespace

void DHTMessageFactoryImpl::validateID(const String* id) const
{
  if (id->s().size() != DHT_ID_LENGTH) {
    throw DL_ABORT_EX(fmt("Malformed DHT message. Invalid ID length."
                          " Expected:%lu, Actual:%lu",
                          static_cast<unsigned long>(DHT_ID_LENGTH),
                          static_cast<unsigned long>(id->s().size())));
  }
}

void DHTMessageFactoryImpl::validatePort(const Integer* port) const
{
  if (!(0 < port->i() && port->i() < UINT16_MAX)) {
    throw DL_ABORT_EX(
        fmt("Malformed DHT message. Invalid port=%" PRId64 "", port->i()));
  }
}

namespace {
void setVersion(DHTMessage* msg, const Dict* dict)
{
  const String* v = downcast<String>(dict->get(DHTMessage::V));
  if (v) {
    msg->setVersion(v->s());
  }
  else {
    msg->setVersion(A2STR::NIL);
  }
}
} // namespace

std::unique_ptr<DHTQueryMessage> DHTMessageFactoryImpl::createQueryMessage(
    const Dict* dict, const std::string& ipaddr, uint16_t port)
{
  const String* messageType = getString(dict, DHTQueryMessage::Q);
  const String* transactionID = getString(dict, DHTMessage::T);
  const String* y = getString(dict, DHTMessage::Y);
  const Dict* aDict = getDictionary(dict, DHTQueryMessage::A);
  if (y->s() != DHTQueryMessage::Q) {
    throw DL_ABORT_EX("Malformed DHT message. y != q");
  }
  const String* id = getString(aDict, DHTMessage::ID);
  validateID(id);
  auto remoteNode = getRemoteNode(id->uc(), ipaddr, port);
  auto msg = std::unique_ptr<DHTQueryMessage>{};
  if (messageType->s() == DHTPingMessage::PING) {
    msg = createPingMessage(remoteNode, transactionID->s());
  }
  else if (messageType->s() == DHTFindNodeMessage::FIND_NODE) {
    const String* targetNodeID =
        getString(aDict, DHTFindNodeMessage::TARGET_NODE);
    validateID(targetNodeID);
    msg = createFindNodeMessage(remoteNode, targetNodeID->uc(),
                                transactionID->s());
  }
  else if (messageType->s() == DHTGetPeersMessage::GET_PEERS) {
    const String* infoHash = getString(aDict, DHTGetPeersMessage::INFO_HASH);
    validateID(infoHash);
    msg = createGetPeersMessage(remoteNode, infoHash->uc(), transactionID->s());
  }
  else if (messageType->s() == DHTAnnouncePeerMessage::ANNOUNCE_PEER) {
    const String* infoHash =
        getString(aDict, DHTAnnouncePeerMessage::INFO_HASH);
    validateID(infoHash);
    const Integer* port = getInteger(aDict, DHTAnnouncePeerMessage::PORT);
    validatePort(port);
    const String* token = getString(aDict, DHTAnnouncePeerMessage::TOKEN);
    msg = createAnnouncePeerMessage(remoteNode, infoHash->uc(),
                                    static_cast<uint16_t>(port->i()),
                                    token->s(), transactionID->s());
  }
  else {
    throw DL_ABORT_EX(
        fmt("Unsupported message type: %s", messageType->s().c_str()));
  }
  setVersion(msg.get(), dict);
  return msg;
}

std::unique_ptr<DHTResponseMessage>
DHTMessageFactoryImpl::createResponseMessage(const std::string& messageType,
                                             const Dict* dict,
                                             const std::string& ipaddr,
                                             uint16_t port)
{
  const String* transactionID = getString(dict, DHTMessage::T);
  const String* y = getString(dict, DHTMessage::Y);
  if (y->s() == DHTUnknownMessage::E) {
    // for now, just report error message arrived and throw exception.
    const List* e = getList(dict, DHTUnknownMessage::E);
    if (e->size() == 2) {
      A2_LOG_INFO(fmt("Received Error DHT message. code=%" PRId64 ", msg=%s",
                      getInteger(e, 0)->i(),
                      util::percentEncode(getString(e, 1)->s()).c_str()));
    }
    else {
      A2_LOG_DEBUG("e doesn't have 2 elements.");
    }
    throw DL_ABORT_EX("Received Error DHT message.");
  }
  else if (y->s() != DHTResponseMessage::R) {
    throw DL_ABORT_EX(fmt("Malformed DHT message. y != r: y=%s",
                          util::percentEncode(y->s()).c_str()));
  }
  const Dict* rDict = getDictionary(dict, DHTResponseMessage::R);
  const String* id = getString(rDict, DHTMessage::ID);
  validateID(id);
  auto remoteNode = getRemoteNode(id->uc(), ipaddr, port);
  auto msg = std::unique_ptr<DHTResponseMessage>{};
  if (messageType == DHTPingReplyMessage::PING) {
    msg = createPingReplyMessage(remoteNode, id->uc(), transactionID->s());
  }
  else if (messageType == DHTFindNodeReplyMessage::FIND_NODE) {
    msg = createFindNodeReplyMessage(remoteNode, dict, transactionID->s());
  }
  else if (messageType == DHTGetPeersReplyMessage::GET_PEERS) {
    msg = createGetPeersReplyMessage(remoteNode, dict, transactionID->s());
  }
  else if (messageType == DHTAnnouncePeerReplyMessage::ANNOUNCE_PEER) {
    msg = createAnnouncePeerReplyMessage(remoteNode, transactionID->s());
  }
  else {
    throw DL_ABORT_EX(fmt("Unsupported message type: %s", messageType.c_str()));
  }
  setVersion(msg.get(), dict);
  return msg;
}

namespace {
const std::string& getDefaultVersion()
{
  static std::string version;
  if (version.empty()) {
    uint16_t vnum16 = htons(DHT_VERSION);
    unsigned char buf[] = {'A', '2', 0, 0};
    char* vnump = reinterpret_cast<char*>(&vnum16);
    memcpy(buf + 2, vnump, 2);
    version.assign(&buf[0], &buf[4]);
  }
  return version;
}
} // namespace

void DHTMessageFactoryImpl::setCommonProperty(DHTAbstractMessage* m)
{
  m->setConnection(connection_);
  m->setMessageDispatcher(dispatcher_);
  m->setRoutingTable(routingTable_);
  m->setMessageFactory(this);
  m->setVersion(getDefaultVersion());
}

std::unique_ptr<DHTPingMessage> DHTMessageFactoryImpl::createPingMessage(
    const std::shared_ptr<DHTNode>& remoteNode,
    const std::string& transactionID)
{
  auto m = make_unique<DHTPingMessage>(localNode_, remoteNode, transactionID);
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTPingReplyMessage>
DHTMessageFactoryImpl::createPingReplyMessage(
    const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* id,
    const std::string& transactionID)
{
  auto m = make_unique<DHTPingReplyMessage>(localNode_, remoteNode, id,
                                            transactionID);
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTFindNodeMessage>
DHTMessageFactoryImpl::createFindNodeMessage(
    const std::shared_ptr<DHTNode>& remoteNode,
    const unsigned char* targetNodeID, const std::string& transactionID)
{
  auto m = make_unique<DHTFindNodeMessage>(localNode_, remoteNode, targetNodeID,
                                           transactionID);
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTFindNodeReplyMessage>
DHTMessageFactoryImpl::createFindNodeReplyMessage(
    const std::shared_ptr<DHTNode>& remoteNode,
    std::vector<std::shared_ptr<DHTNode>> closestKNodes,
    const std::string& transactionID)
{
  auto m = make_unique<DHTFindNodeReplyMessage>(family_, localNode_, remoteNode,
                                                transactionID);
  m->setClosestKNodes(std::move(closestKNodes));
  setCommonProperty(m.get());
  return m;
}

void DHTMessageFactoryImpl::extractNodes(
    std::vector<std::shared_ptr<DHTNode>>& nodes, const unsigned char* src,
    size_t length)
{
  int unit = bittorrent::getCompactLength(family_) + 20;
  if (length % unit != 0) {
    throw DL_ABORT_EX(fmt("Nodes length is not multiple of %d", unit));
  }
  for (size_t offset = 0; offset < length; offset += unit) {
    auto node = std::make_shared<DHTNode>(src + offset);
    auto addr =
        bittorrent::unpackcompact(src + offset + DHT_ID_LENGTH, family_);
    if (addr.first.empty()) {
      continue;
    }
    node->setIPAddress(addr.first);
    node->setPort(addr.second);
    nodes.push_back(std::move(node));
  }
}

std::unique_ptr<DHTFindNodeReplyMessage>
DHTMessageFactoryImpl::createFindNodeReplyMessage(
    const std::shared_ptr<DHTNode>& remoteNode, const Dict* dict,
    const std::string& transactionID)
{
  const String* nodesData = downcast<String>(
      getDictionary(dict, DHTResponseMessage::R)
          ->get(family_ == AF_INET ? DHTFindNodeReplyMessage::NODES
                                   : DHTFindNodeReplyMessage::NODES6));
  std::vector<std::shared_ptr<DHTNode>> nodes;
  if (nodesData) {
    extractNodes(nodes, nodesData->uc(), nodesData->s().size());
  }
  return createFindNodeReplyMessage(remoteNode, std::move(nodes),
                                    transactionID);
}

std::unique_ptr<DHTGetPeersMessage>
DHTMessageFactoryImpl::createGetPeersMessage(
    const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* infoHash,
    const std::string& transactionID)
{
  auto m = make_unique<DHTGetPeersMessage>(localNode_, remoteNode, infoHash,
                                           transactionID);
  m->setPeerAnnounceStorage(peerAnnounceStorage_);
  m->setTokenTracker(tokenTracker_);
  m->setBtRegistry(btRegistry_);
  m->setFamily(family_);
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTGetPeersReplyMessage>
DHTMessageFactoryImpl::createGetPeersReplyMessage(
    const std::shared_ptr<DHTNode>& remoteNode, const Dict* dict,
    const std::string& transactionID)
{
  const Dict* rDict = getDictionary(dict, DHTResponseMessage::R);
  const String* nodesData = downcast<String>(
      rDict->get(family_ == AF_INET ? DHTGetPeersReplyMessage::NODES
                                    : DHTGetPeersReplyMessage::NODES6));
  std::vector<std::shared_ptr<DHTNode>> nodes;
  if (nodesData) {
    extractNodes(nodes, nodesData->uc(), nodesData->s().size());
  }
  const List* valuesList =
      downcast<List>(rDict->get(DHTGetPeersReplyMessage::VALUES));
  std::vector<std::shared_ptr<Peer>> peers;
  size_t clen = bittorrent::getCompactLength(family_);
  if (valuesList) {
    for (auto& elem : *valuesList) {
      const String* data = downcast<String>(elem);
      if (data && data->s().size() == clen) {
        auto addr = bittorrent::unpackcompact(data->uc(), family_);
        if (addr.first.empty()) {
          continue;
        }
        peers.push_back(std::make_shared<Peer>(addr.first, addr.second));
      }
    }
  }
  const String* token = getString(rDict, DHTGetPeersReplyMessage::TOKEN);
  return createGetPeersReplyMessage(remoteNode, std::move(nodes),
                                    std::move(peers), token->s(),
                                    transactionID);
}

std::unique_ptr<DHTGetPeersReplyMessage>
DHTMessageFactoryImpl::createGetPeersReplyMessage(
    const std::shared_ptr<DHTNode>& remoteNode,
    std::vector<std::shared_ptr<DHTNode>> closestKNodes,
    std::vector<std::shared_ptr<Peer>> values, const std::string& token,
    const std::string& transactionID)
{
  auto m = make_unique<DHTGetPeersReplyMessage>(family_, localNode_, remoteNode,
                                                token, transactionID);
  m->setClosestKNodes(std::move(closestKNodes));
  m->setValues(std::move(values));
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTAnnouncePeerMessage>
DHTMessageFactoryImpl::createAnnouncePeerMessage(
    const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* infoHash,
    uint16_t tcpPort, const std::string& token,
    const std::string& transactionID)
{
  auto m = make_unique<DHTAnnouncePeerMessage>(localNode_, remoteNode, infoHash,
                                               tcpPort, token, transactionID);
  m->setPeerAnnounceStorage(peerAnnounceStorage_);
  m->setTokenTracker(tokenTracker_);
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTAnnouncePeerReplyMessage>
DHTMessageFactoryImpl::createAnnouncePeerReplyMessage(
    const std::shared_ptr<DHTNode>& remoteNode,
    const std::string& transactionID)
{
  auto m = make_unique<DHTAnnouncePeerReplyMessage>(localNode_, remoteNode,
                                                    transactionID);
  setCommonProperty(m.get());
  return m;
}

std::unique_ptr<DHTUnknownMessage> DHTMessageFactoryImpl::createUnknownMessage(
    const unsigned char* data, size_t length, const std::string& ipaddr,
    uint16_t port)

{
  return make_unique<DHTUnknownMessage>(localNode_, data, length, ipaddr, port);
}

void DHTMessageFactoryImpl::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

void DHTMessageFactoryImpl::setConnection(DHTConnection* connection)
{
  connection_ = connection;
}

void DHTMessageFactoryImpl::setMessageDispatcher(
    DHTMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void DHTMessageFactoryImpl::setPeerAnnounceStorage(
    DHTPeerAnnounceStorage* storage)
{
  peerAnnounceStorage_ = storage;
}

void DHTMessageFactoryImpl::setTokenTracker(DHTTokenTracker* tokenTracker)
{
  tokenTracker_ = tokenTracker;
}

void DHTMessageFactoryImpl::setLocalNode(
    const std::shared_ptr<DHTNode>& localNode)
{
  localNode_ = localNode;
}

void DHTMessageFactoryImpl::setBtRegistry(BtRegistry* btRegistry)
{
  btRegistry_ = btRegistry;
}

} // namespace aria2
