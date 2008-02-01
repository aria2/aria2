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
#include "DHTConnection.h"
#include "DHTMessageDispatcher.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTTokenTracker.h"
#include "PeerMessageUtil.h"
#include "BtRuntime.h"
#include "Util.h"
#include "Peer.h"

DHTMessageFactoryImpl::DHTMessageFactoryImpl():_localNode(0),
					       _logger(LogFactory::getInstance()) {}

DHTMessageFactoryImpl::~DHTMessageFactoryImpl() {}

DHTNodeHandle
DHTMessageFactoryImpl::getRemoteNode(const unsigned char* id, const string& ipaddr, uint16_t port) const
{
  DHTNodeHandle node = _routingTable->getNode(id, ipaddr, port);
  if(node.isNull()) {
    node = new DHTNode(id);
    node->setIPAddress(ipaddr);
    node->setPort(port);
  }
  return node;
}

static const Dictionary* getDictionary(const Dictionary* d, const string& key)
{
  const Dictionary* c = dynamic_cast<const Dictionary*>(d->get(key));
  if(c) {
    return c;
  } else {
    throw new DlAbortEx("Malformed DHT message. Missing %s", key.c_str());
  }
}

static const Data* getData(const Dictionary* d, const string& key)
{
  const Data* c = dynamic_cast<const Data*>(d->get(key));
  if(c) {
    return c;
  } else {
    throw new DlAbortEx("Malformed DHT message. Missing %s", key.c_str());
  }
}

static const List* getList(const Dictionary* d, const string& key)
{
  const List* l = dynamic_cast<const List*>(d->get(key));
  if(l) {
    return l;
  } else {
    throw new DlAbortEx("Malformed DHT message. Missing %s", key.c_str());
  }
}

void DHTMessageFactoryImpl::validateID(const Data* id) const
{
  if(id->getLen() != DHT_ID_LENGTH) {
    throw new DlAbortEx("Malformed DHT message. Invalid ID length. Expected:%d, Actual:%d", DHT_ID_LENGTH, id->getLen());
  }
}

void DHTMessageFactoryImpl::validateIDMatch(const unsigned char* expected, const unsigned char* actual) const
{
  if(memcmp(expected, actual, DHT_ID_LENGTH) != 0) {
    //throw new DlAbortEx("Different ID received.");
  }
}

void DHTMessageFactoryImpl::validatePort(const Data* i) const
{
  if(!i->isNumber()) {
    throw new DlAbortEx("Malformed DHT message. Invalid port=%s",
			Util::toHex(i->toString()).c_str());
  }
  uint32_t port = i->toInt();
  if(UINT16_MAX < port) {
    throw new DlAbortEx("Malformed DHT message. Invalid port=%u", port);
  }
}

DHTMessageHandle DHTMessageFactoryImpl::createQueryMessage(const Dictionary* d,
							   const string& ipaddr,
							   uint16_t port)
{
  const Data* q = getData(d, "q");
  const Data* t = getData(d, "t");
  const Data* y = getData(d, "y");
  const Dictionary* a = getDictionary(d, "a");
  if(y->toString() != "q") {
    throw new DlAbortEx("Malformed DHT message. y != q");
  }
  const Data* id = getData(getDictionary(d, "a"), "id");
  validateID(id);
  DHTNodeHandle remoteNode = getRemoteNode((const unsigned char*)id->toString().c_str(), ipaddr, port);
  string messageType = q->toString();
  string transactionID = t->toString();
  if(messageType == "ping") {
    return createPingMessage(remoteNode, transactionID);
  } else if(messageType == "find_node") {
    const Data* targetNodeID = getData(a, "target");
    validateID(targetNodeID);
    return createFindNodeMessage(remoteNode,
				 (const unsigned char*)targetNodeID->getData(),
				 transactionID);
  } else if(messageType == "get_peers") {
    const Data* infoHash = getData(a, "info_hash");
    validateID(infoHash);
    return createGetPeersMessage(remoteNode,
				 (const unsigned char*)infoHash->getData(),
				 transactionID);
  } else if(messageType == "announce_peer") {
    const Data* infoHash = getData(a, "info_hash");
    validateID(infoHash);
    const Data* port = getData(a, "port");
    validatePort(port);
    const Data* token = getData(a, "token");
    return createAnnouncePeerMessage(remoteNode,
				     (const unsigned char*)infoHash->getData(),
				     (uint16_t)port->toInt(),
				     token->toString(),
				     transactionID);
  } else {
    throw new DlAbortEx("Unsupported message type: %s", messageType.c_str());
  }
}

DHTMessageHandle DHTMessageFactoryImpl::createResponseMessage(const string& messageType,
							      const Dictionary* d,
							      const DHTNodeHandle& remoteNode)
{
  const Data* t = getData(d, "t");
  const Data* y = getData(d, "y");
  const Dictionary* r = getDictionary(d, "r");
  // TODO handle y == "e" case
  if(y->toString() != "r") {
    throw new DlAbortEx("Malformed DHT message. y != r: y=%s",
			Util::urlencode(y->toString()).c_str());
  }
  const Data* id = getData(r, "id");
  validateID(id);
  validateIDMatch(remoteNode->getID(),
		      (const unsigned char*)id->toString().c_str());
  string transactionID = t->toString();
  if(messageType == "ping") {
    return createPingReplyMessage(remoteNode, (const unsigned char*)id->getData(), transactionID);
  } else if(messageType == "find_node") {
    return createFindNodeReplyMessage(remoteNode, d, transactionID);
  } else if(messageType == "get_peers") {
    const List* values = dynamic_cast<const List*>(r->get("values"));
    if(values) {
      return createGetPeersReplyMessageWithValues(remoteNode, d, transactionID);
    } else {
      const Data* nodes = dynamic_cast<const Data*>(r->get("nodes"));
      if(nodes) {
	return createGetPeersReplyMessageWithNodes(remoteNode, d, transactionID);
      } else {
	throw new DlAbortEx("Malformed DHT message: missing nodes/values");
      }
    }
  } else if(messageType == "announce_peer") {
    return createAnnouncePeerReplyMessage(remoteNode, transactionID);
  } else {
    throw new DlAbortEx("Unsupported message type: %s", messageType.c_str());
  }
}

void DHTMessageFactoryImpl::setCommonProperty(const SharedHandle<DHTAbstractMessage>& m)
{
  m->setConnection(_connection);
  m->setMessageDispatcher(_dispatcher);
  m->setRoutingTable(_routingTable);
  m->setMessageFactory(this);
}

DHTMessageHandle DHTMessageFactoryImpl::createPingMessage(const DHTNodeHandle& remoteNode, const string& transactionID)
{
  SharedHandle<DHTPingMessage> m = new DHTPingMessage(_localNode, remoteNode, transactionID);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createPingReplyMessage(const DHTNodeHandle& remoteNode,
					      const unsigned char* id,
					      const string& transactionID)
{
  SharedHandle<DHTPingReplyMessage> m = new DHTPingReplyMessage(_localNode, remoteNode, id, transactionID);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createFindNodeMessage(const DHTNodeHandle& remoteNode,
					     const unsigned char* targetNodeID,
					     const string& transactionID)
{
  SharedHandle<DHTFindNodeMessage> m = new DHTFindNodeMessage(_localNode, remoteNode, targetNodeID, transactionID);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createFindNodeReplyMessage(const DHTNodeHandle& remoteNode,
						  const DHTNodes& closestKNodes,
						  const string& transactionID)
{
  SharedHandle<DHTFindNodeReplyMessage> m = new DHTFindNodeReplyMessage(_localNode, remoteNode, transactionID);
  m->setClosestKNodes(closestKNodes);
  setCommonProperty(m);
  return m;
}

DHTNodes DHTMessageFactoryImpl::extractNodes(const char* src, size_t length)
{
  if(length%26 != 0) {
    throw new DlAbortEx("Nodes length is not multiple of 26");
  }
  DHTNodes nodes;
  for(size_t offset = 0; offset < length; offset += 26) {
    DHTNodeHandle node = new DHTNode(reinterpret_cast<const unsigned char*>(src+offset));
    pair<string, uint16_t> addr =
      PeerMessageUtil::unpackcompact(src+offset+DHT_ID_LENGTH);
    node->setIPAddress(addr.first);
    node->setPort(addr.second);
    nodes.push_back(node);
  }
  return nodes;
}

DHTMessageHandle
DHTMessageFactoryImpl::createFindNodeReplyMessage(const DHTNodeHandle& remoteNode,
						  const Dictionary* d,
						  const string& transactionID)
{
  const Data* nodesData = getData(getDictionary(d, "r"), "nodes");
  DHTNodes nodes = extractNodes(nodesData->getData(), nodesData->getLen());
  return createFindNodeReplyMessage(remoteNode, nodes, transactionID);
}

DHTMessageHandle
DHTMessageFactoryImpl::createGetPeersMessage(const DHTNodeHandle& remoteNode,
					     const unsigned char* infoHash,
					     const string& transactionID)
{
  SharedHandle<DHTGetPeersMessage> m = new DHTGetPeersMessage(_localNode,
							      remoteNode,
							      infoHash,
							      transactionID);
  m->setPeerAnnounceStorage(_peerAnnounceStorage);
  m->setTokenTracker(_tokenTracker);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createGetPeersReplyMessageWithNodes(const DHTNodeHandle& remoteNode,
							   const Dictionary* d,
							   const string& transactionID)
{
  const Dictionary* r = getDictionary(d, "r");
  const Data* nodesData = getData(r, "nodes");
  DHTNodes nodes = extractNodes(nodesData->getData(), nodesData->getLen());
  const Data* token = getData(r, "token");
  return createGetPeersReplyMessage(remoteNode, nodes, token->toString(),
				    transactionID);
}

DHTMessageHandle
DHTMessageFactoryImpl::createGetPeersReplyMessage(const DHTNodeHandle& remoteNode,
						  const DHTNodes& closestKNodes,
						  const string& token,
						  const string& transactionID)
{
  SharedHandle<DHTGetPeersReplyMessage> m = new DHTGetPeersReplyMessage(_localNode, remoteNode, token, transactionID);
  m->setClosestKNodes(closestKNodes);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createGetPeersReplyMessageWithValues(const DHTNodeHandle& remoteNode,
							    const Dictionary* d,
							    const string& transactionID)
{
  const Dictionary* r = getDictionary(d, "r");
  const List* valuesList = getList(r, "values");
  Peers peers;
  for(MetaList::const_iterator i = valuesList->getList().begin(); i != valuesList->getList().end(); ++i) {
    const Data* data = dynamic_cast<const Data*>(*i);
    if(data && data->getLen() == 6) {
      pair<string, uint16_t> addr = PeerMessageUtil::unpackcompact(data->getData());
      PeerHandle peer = new Peer(addr.first, addr.second);
      peers.push_back(peer);
    }
  }
  const Data* token = getData(r, "token");
  return createGetPeersReplyMessage(remoteNode, peers, token->toString(),
				    transactionID);
}

DHTMessageHandle
DHTMessageFactoryImpl::createGetPeersReplyMessage(const DHTNodeHandle& remoteNode,
						  const Peers& values,
						  const string& token,
						  const string& transactionID)
{
  SharedHandle<DHTGetPeersReplyMessage> m = new DHTGetPeersReplyMessage(_localNode, remoteNode, token, transactionID);
  m->setValues(values);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createAnnouncePeerMessage(const DHTNodeHandle& remoteNode,
						 const unsigned char* infoHash,
						 uint16_t tcpPort,
						 const string& token,
						 const string& transactionID)
{
  SharedHandle<DHTAnnouncePeerMessage> m =
    new DHTAnnouncePeerMessage(_localNode, remoteNode, infoHash, tcpPort, token, transactionID);
  m->setPeerAnnounceStorage(_peerAnnounceStorage);
  m->setTokenTracker(_tokenTracker);
  setCommonProperty(m);
  return m;
}

DHTMessageHandle
DHTMessageFactoryImpl::createAnnouncePeerReplyMessage(const DHTNodeHandle& remoteNode,
						      const string& transactionID)
{
  SharedHandle<DHTAnnouncePeerReplyMessage> m =
    new DHTAnnouncePeerReplyMessage(_localNode, remoteNode, transactionID);
  setCommonProperty(m);
  return m;
}

void DHTMessageFactoryImpl::setRoutingTable(const DHTRoutingTableHandle& routingTable)
{
  _routingTable = routingTable;
}

void DHTMessageFactoryImpl::setConnection(const DHTConnectionHandle& connection)
{
  _connection = connection;
}

void DHTMessageFactoryImpl::setMessageDispatcher(const DHTMessageDispatcherHandle& dispatcher)
{
  _dispatcher = dispatcher;
}
  
void DHTMessageFactoryImpl::setPeerAnnounceStorage(const DHTPeerAnnounceStorageHandle& storage)
{
  _peerAnnounceStorage = storage;
}

void DHTMessageFactoryImpl::setTokenTracker(const DHTTokenTrackerHandle& tokenTracker)
{
  _tokenTracker = tokenTracker;
}

void DHTMessageFactoryImpl::setLocalNode(const DHTNodeHandle& localNode)
{
  _localNode = localNode;
}
