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
#include "DHTMessageFactoryImpl.h"

#include <cstring>
#include <utility>

#include "LogFactory.h"
#include "DlAbortEx.h"
#include "Data.h"
#include "Dictionary.h"
#include "List.h"
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
#include "PeerMessageUtil.h"
#include "BtRuntime.h"
#include "Util.h"
#include "Peer.h"
#include "Logger.h"
#include "StringFormat.h"

namespace aria2 {

DHTMessageFactoryImpl::DHTMessageFactoryImpl():
  _logger(LogFactory::getInstance()) {}

DHTMessageFactoryImpl::~DHTMessageFactoryImpl() {}

SharedHandle<DHTNode>
DHTMessageFactoryImpl::getRemoteNode(const unsigned char* id, const std::string& ipaddr, uint16_t port) const
{
  SharedHandle<DHTNode> node = _routingTable->getNode(id, ipaddr, port);
  if(node.isNull()) {
    node.reset(new DHTNode(id));
    node->setIPAddress(ipaddr);
    node->setPort(port);
  }
  return node;
}

static const Dictionary* getDictionary(const Dictionary* d, const std::string& key)
{
  const Dictionary* c = dynamic_cast<const Dictionary*>(d->get(key));
  if(c) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const Data* getData(const Dictionary* d, const std::string& key)
{
  const Data* c = dynamic_cast<const Data*>(d->get(key));
  if(c) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const Data* getData(const List* l, size_t index)
{
  const Data* c = dynamic_cast<const Data*>(l->getList()[index]);
  if(c) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. element[%u] is not Data.",
		    index).str());
  }
}

static const List* getList(const Dictionary* d, const std::string& key)
{
  const List* l = dynamic_cast<const List*>(d->get(key));
  if(l) {
    return l;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

void DHTMessageFactoryImpl::validateID(const Data* id) const
{
  if(id->getLen() != DHT_ID_LENGTH) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Invalid ID length. Expected:%d, Actual:%d", DHT_ID_LENGTH, id->getLen()).str());
  }
}

void DHTMessageFactoryImpl::validatePort(const Data* i) const
{
  if(!i->isNumber()) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Invalid port=%s",
		    Util::toHex(i->toString()).c_str()).str());
  }
  uint32_t port = i->toInt();
  if(UINT16_MAX < port) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Invalid port=%u", port).str());
  }
}

SharedHandle<DHTMessage> DHTMessageFactoryImpl::createQueryMessage(const Dictionary* d,
							   const std::string& ipaddr,
							   uint16_t port)
{
  const Data* q = getData(d, DHTQueryMessage::Q);
  const Data* t = getData(d, DHTMessage::T);
  const Data* y = getData(d, DHTMessage::Y);
  const Dictionary* a = getDictionary(d, DHTQueryMessage::A);
  if(y->toString() != DHTQueryMessage::Q) {
    throw DlAbortEx("Malformed DHT message. y != q");
  }
  const Data* id = getData(getDictionary(d, DHTQueryMessage::A), DHTMessage::ID);
  validateID(id);
  SharedHandle<DHTNode> remoteNode = getRemoteNode(id->getData(), ipaddr, port);
  std::string messageType = q->toString();
  std::string transactionID = t->toString();
  if(messageType == DHTPingMessage::PING) {
    return createPingMessage(remoteNode, transactionID);
  } else if(messageType == DHTFindNodeMessage::FIND_NODE) {
    const Data* targetNodeID = getData(a, DHTFindNodeMessage::TARGET_NODE);
    validateID(targetNodeID);
    return createFindNodeMessage(remoteNode, targetNodeID->getData(),
				 transactionID);
  } else if(messageType == DHTGetPeersMessage::GET_PEERS) {
    const Data* infoHash = getData(a, DHTGetPeersMessage::INFO_HASH);
    validateID(infoHash);
    return createGetPeersMessage(remoteNode,
				 infoHash->getData(), transactionID);
  } else if(messageType == DHTAnnouncePeerMessage::ANNOUNCE_PEER) {
    const Data* infoHash = getData(a, DHTAnnouncePeerMessage::INFO_HASH);
    validateID(infoHash);
    const Data* port = getData(a, DHTAnnouncePeerMessage::PORT);
    validatePort(port);
    const Data* token = getData(a, DHTAnnouncePeerMessage::TOKEN);
    return createAnnouncePeerMessage(remoteNode, infoHash->getData(),
				     static_cast<uint16_t>(port->toInt()),
				     token->toString(), transactionID);
  } else {
    throw DlAbortEx
      (StringFormat("Unsupported message type: %s", messageType.c_str()).str());
  }
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createResponseMessage(const std::string& messageType,
					     const Dictionary* d,
					     const std::string& ipaddr,
					     uint16_t port)
{
  const Data* t = getData(d, DHTMessage::T);
  const Data* y = getData(d, DHTMessage::Y);
  if(y->toString() == DHTUnknownMessage::E) {
    // for now, just report error message arrived and throw exception.
    const List* e = getList(d, DHTUnknownMessage::E);
    if(e->getList().size() == 2) {
      _logger->info("Received Error DHT message. code=%s, msg=%s",
		    Util::urlencode(getData(e, 0)->toString()).c_str(),
		    Util::urlencode(getData(e, 1)->toString()).c_str());
    } else {
      _logger->debug("e doesn't have 2 elements.");
    }
    throw DlAbortEx("Received Error DHT message.");
  } else if(y->toString() != DHTResponseMessage::R) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. y != r: y=%s",
		    Util::urlencode(y->toString()).c_str()).str());
  }
  const Dictionary* r = getDictionary(d, DHTResponseMessage::R);
  const Data* id = getData(r, DHTMessage::ID);
  validateID(id);
  SharedHandle<DHTNode> remoteNode = getRemoteNode(id->getData(), ipaddr, port);

  std::string transactionID = t->toString();
  if(messageType == DHTPingReplyMessage::PING) {
    return createPingReplyMessage(remoteNode,
				  id->getData(), transactionID);
  } else if(messageType == DHTFindNodeReplyMessage::FIND_NODE) {
    return createFindNodeReplyMessage(remoteNode, d, transactionID);
  } else if(messageType == DHTGetPeersReplyMessage::GET_PEERS) {
    const List* values =
      dynamic_cast<const List*>(r->get(DHTGetPeersReplyMessage::VALUES));
    if(values) {
      return createGetPeersReplyMessageWithValues(remoteNode, d, transactionID);
    } else {
      const Data* nodes = dynamic_cast<const Data*>(r->get(DHTGetPeersReplyMessage::NODES));
      if(nodes) {
	return createGetPeersReplyMessageWithNodes(remoteNode, d, transactionID);
      } else {
	throw DlAbortEx("Malformed DHT message: missing nodes/values");
      }
    }
  } else if(messageType == DHTAnnouncePeerReplyMessage::ANNOUNCE_PEER) {
    return createAnnouncePeerReplyMessage(remoteNode, transactionID);
  } else {
    throw DlAbortEx
      (StringFormat("Unsupported message type: %s", messageType.c_str()).str());
  }
}

void DHTMessageFactoryImpl::setCommonProperty(const SharedHandle<DHTAbstractMessage>& m)
{
  m->setConnection(_connection);
  m->setMessageDispatcher(_dispatcher);
  m->setRoutingTable(_routingTable);
  WeakHandle<DHTMessageFactory> factory(this);
  m->setMessageFactory(factory);
}

SharedHandle<DHTMessage> DHTMessageFactoryImpl::createPingMessage(const SharedHandle<DHTNode>& remoteNode, const std::string& transactionID)
{
  SharedHandle<DHTPingMessage> m(new DHTPingMessage(_localNode, remoteNode, transactionID));
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createPingReplyMessage(const SharedHandle<DHTNode>& remoteNode,
					      const unsigned char* id,
					      const std::string& transactionID)
{
  SharedHandle<DHTPingReplyMessage> m(new DHTPingReplyMessage(_localNode, remoteNode, id, transactionID));
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createFindNodeMessage(const SharedHandle<DHTNode>& remoteNode,
					     const unsigned char* targetNodeID,
					     const std::string& transactionID)
{
  SharedHandle<DHTFindNodeMessage> m(new DHTFindNodeMessage(_localNode, remoteNode, targetNodeID, transactionID));
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createFindNodeReplyMessage(const SharedHandle<DHTNode>& remoteNode,
						  const std::deque<SharedHandle<DHTNode> >& closestKNodes,
						  const std::string& transactionID)
{
  SharedHandle<DHTFindNodeReplyMessage> m(new DHTFindNodeReplyMessage(_localNode, remoteNode, transactionID));
  m->setClosestKNodes(closestKNodes);
  setCommonProperty(m);
  return m;
}

std::deque<SharedHandle<DHTNode> >
DHTMessageFactoryImpl::extractNodes(const unsigned char* src, size_t length)
{
  if(length%26 != 0) {
    throw DlAbortEx("Nodes length is not multiple of 26");
  }
  std::deque<SharedHandle<DHTNode> > nodes;
  for(size_t offset = 0; offset < length; offset += 26) {
    SharedHandle<DHTNode> node(new DHTNode(src+offset));
    std::pair<std::string, uint16_t> addr =
      PeerMessageUtil::unpackcompact(src+offset+DHT_ID_LENGTH);
    if(addr.first.empty()) {
      continue;
    }
    node->setIPAddress(addr.first);
    node->setPort(addr.second);
    nodes.push_back(node);
  }
  return nodes;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createFindNodeReplyMessage(const SharedHandle<DHTNode>& remoteNode,
						  const Dictionary* d,
						  const std::string& transactionID)
{
  const Data* nodesData = getData(getDictionary(d, DHTResponseMessage::R), DHTFindNodeReplyMessage::NODES);
  std::deque<SharedHandle<DHTNode> > nodes = extractNodes(nodesData->getData(), nodesData->getLen());
  return createFindNodeReplyMessage(remoteNode, nodes, transactionID);
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createGetPeersMessage(const SharedHandle<DHTNode>& remoteNode,
					     const unsigned char* infoHash,
					     const std::string& transactionID)
{
  SharedHandle<DHTGetPeersMessage> m(new DHTGetPeersMessage(_localNode,
							    remoteNode,
							    infoHash,
							    transactionID));
  m->setPeerAnnounceStorage(_peerAnnounceStorage);
  m->setTokenTracker(_tokenTracker);
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createGetPeersReplyMessageWithNodes(const SharedHandle<DHTNode>& remoteNode,
							   const Dictionary* d,
							   const std::string& transactionID)
{
  const Dictionary* r = getDictionary(d, DHTResponseMessage::R);
  const Data* nodesData = getData(r, DHTGetPeersReplyMessage::NODES);
  std::deque<SharedHandle<DHTNode> > nodes = extractNodes(nodesData->getData(), nodesData->getLen());
  const Data* token = getData(r, DHTGetPeersReplyMessage::TOKEN);
  return createGetPeersReplyMessage(remoteNode, nodes, token->toString(),
				    transactionID);
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
						  const std::deque<SharedHandle<DHTNode> >& closestKNodes,
						  const std::string& token,
						  const std::string& transactionID)
{
  SharedHandle<DHTGetPeersReplyMessage> m
    (new DHTGetPeersReplyMessage(_localNode, remoteNode, token, transactionID));
  m->setClosestKNodes(closestKNodes);
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createGetPeersReplyMessageWithValues(const SharedHandle<DHTNode>& remoteNode,
							    const Dictionary* d,
							    const std::string& transactionID)
{
  const Dictionary* r = getDictionary(d, DHTResponseMessage::R);
  const List* valuesList = getList(r, DHTGetPeersReplyMessage::VALUES);
  std::deque<SharedHandle<Peer> > peers;
  for(std::deque<MetaEntry*>::const_iterator i = valuesList->getList().begin(); i != valuesList->getList().end(); ++i) {
    const Data* data = dynamic_cast<const Data*>(*i);
    if(data && data->getLen() == 6) {
      std::pair<std::string, uint16_t> addr = PeerMessageUtil::unpackcompact(data->getData());
      PeerHandle peer(new Peer(addr.first, addr.second));
      peers.push_back(peer);
    }
  }
  const Data* token = getData(r, DHTGetPeersReplyMessage::TOKEN);
  return createGetPeersReplyMessage(remoteNode, peers, token->toString(),
				    transactionID);
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
						  const std::deque<SharedHandle<Peer> >& values,
						  const std::string& token,
						  const std::string& transactionID)
{
  SharedHandle<DHTGetPeersReplyMessage> m(new DHTGetPeersReplyMessage(_localNode, remoteNode, token, transactionID));
  m->setValues(values);
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createAnnouncePeerMessage(const SharedHandle<DHTNode>& remoteNode,
						 const unsigned char* infoHash,
						 uint16_t tcpPort,
						 const std::string& token,
						 const std::string& transactionID)
{
  SharedHandle<DHTAnnouncePeerMessage> m
    (new DHTAnnouncePeerMessage(_localNode, remoteNode, infoHash, tcpPort, token, transactionID));
  m->setPeerAnnounceStorage(_peerAnnounceStorage);
  m->setTokenTracker(_tokenTracker);
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createAnnouncePeerReplyMessage(const SharedHandle<DHTNode>& remoteNode,
						      const std::string& transactionID)
{
  SharedHandle<DHTAnnouncePeerReplyMessage> m
    (new DHTAnnouncePeerReplyMessage(_localNode, remoteNode, transactionID));
  setCommonProperty(m);
  return m;
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createUnknownMessage(const unsigned char* data, size_t length,
					    const std::string& ipaddr, uint16_t port)

{
  SharedHandle<DHTUnknownMessage> m
    (new DHTUnknownMessage(_localNode, data, length, ipaddr, port));
  return m;
}

void DHTMessageFactoryImpl::setRoutingTable(const WeakHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

void DHTMessageFactoryImpl::setConnection(const WeakHandle<DHTConnection>& connection)
{
  _connection = connection;
}

void DHTMessageFactoryImpl::setMessageDispatcher(const WeakHandle<DHTMessageDispatcher>& dispatcher)
{
  _dispatcher = dispatcher;
}
  
void DHTMessageFactoryImpl::setPeerAnnounceStorage(const WeakHandle<DHTPeerAnnounceStorage>& storage)
{
  _peerAnnounceStorage = storage;
}

void DHTMessageFactoryImpl::setTokenTracker(const WeakHandle<DHTTokenTracker>& tokenTracker)
{
  _tokenTracker = tokenTracker;
}

void DHTMessageFactoryImpl::setLocalNode(const SharedHandle<DHTNode>& localNode)
{
  _localNode = localNode;
}

} // namespace aria2
