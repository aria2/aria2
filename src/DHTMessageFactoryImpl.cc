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
#include "bencode.h"

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

static const bencode::BDE& getDictionary(const bencode::BDE& dict,
					 const std::string& key)
{
  const bencode::BDE& d = dict[key];
  if(d.isDict()) {
    return d;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const bencode::BDE& getString(const bencode::BDE& dict,
				     const std::string& key)
{
  const bencode::BDE& c = dict[key];
  if(c.isString()) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const bencode::BDE& getInteger(const bencode::BDE& dict,
				      const std::string& key)
{
  const bencode::BDE& c = dict[key];
  if(c.isInteger()) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const bencode::BDE& getString(const bencode::BDE& list, size_t index)
{
  const bencode::BDE& c = list[index];
  if(c.isString()) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. element[%u] is not String.",
		    index).str());
  }
}

static const bencode::BDE& getInteger(const bencode::BDE& list, size_t index)
{
  const bencode::BDE& c = list[index];
  if(c.isInteger()) {
    return c;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. element[%u] is not Integer.",
		    index).str());
  }
}

static const bencode::BDE& getList(const bencode::BDE& dict,
				   const std::string& key)
{
  const bencode::BDE& l = dict[key];
  if(l.isList()) {
    return l;
  } else {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

void DHTMessageFactoryImpl::validateID(const bencode::BDE& id) const
{
  if(id.s().size() != DHT_ID_LENGTH) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Invalid ID length."
		    " Expected:%d, Actual:%d",
		    DHT_ID_LENGTH, id.s().size()).str());
  }
}

void DHTMessageFactoryImpl::validatePort(const bencode::BDE& i) const
{
  bencode::BDE::Integer port = i.i();
  if(!(0 < port && port < UINT16_MAX)) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. Invalid port=%s",
		    Util::itos(port).c_str()).str());
  }
}

SharedHandle<DHTMessage> DHTMessageFactoryImpl::createQueryMessage
(const bencode::BDE& dict,
 const std::string& ipaddr,
 uint16_t port)
{
  const bencode::BDE& messageType = getString(dict, DHTQueryMessage::Q);
  const bencode::BDE& transactionID = getString(dict, DHTMessage::T);
  const bencode::BDE& y = getString(dict, DHTMessage::Y);
  const bencode::BDE& aDict = getDictionary(dict, DHTQueryMessage::A);
  if(y.s() != DHTQueryMessage::Q) {
    throw DlAbortEx("Malformed DHT message. y != q");
  }
  const bencode::BDE& id = getString(aDict, DHTMessage::ID);
  validateID(id);
  SharedHandle<DHTNode> remoteNode = getRemoteNode(id.uc(), ipaddr, port);
  if(messageType.s() == DHTPingMessage::PING) {
    return createPingMessage(remoteNode, transactionID.s());
  } else if(messageType.s() == DHTFindNodeMessage::FIND_NODE) {
    const bencode::BDE& targetNodeID =
      getString(aDict, DHTFindNodeMessage::TARGET_NODE);
    validateID(targetNodeID);
    return createFindNodeMessage(remoteNode, targetNodeID.uc(),
				 transactionID.s());
  } else if(messageType.s() == DHTGetPeersMessage::GET_PEERS) {
    const bencode::BDE& infoHash = 
      getString(aDict, DHTGetPeersMessage::INFO_HASH);
    validateID(infoHash);
    return createGetPeersMessage(remoteNode,
				 infoHash.uc(), transactionID.s());
  } else if(messageType.s() == DHTAnnouncePeerMessage::ANNOUNCE_PEER) {
    const bencode::BDE& infoHash =
      getString(aDict, DHTAnnouncePeerMessage::INFO_HASH);
    validateID(infoHash);
    const bencode::BDE& port = getInteger(aDict, DHTAnnouncePeerMessage::PORT);
    validatePort(port);
    const bencode::BDE& token = getString(aDict, DHTAnnouncePeerMessage::TOKEN);
    return createAnnouncePeerMessage(remoteNode, infoHash.uc(),
				     static_cast<uint16_t>(port.i()),
				     token.s(), transactionID.s());
  } else {
    throw DlAbortEx
      (StringFormat("Unsupported message type: %s",
		    messageType.s().c_str()).str());
  }
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createResponseMessage(const std::string& messageType,
					     const bencode::BDE& dict,
					     const std::string& ipaddr,
					     uint16_t port)
{
  const bencode::BDE& transactionID = getString(dict, DHTMessage::T);
  const bencode::BDE& y = getString(dict, DHTMessage::Y);
  if(y.s() == DHTUnknownMessage::E) {
    // for now, just report error message arrived and throw exception.
    const bencode::BDE& e = getList(dict, DHTUnknownMessage::E);
    if(e.size() == 2) {
      _logger->info("Received Error DHT message. code=%s, msg=%s",
		    Util::itos(getInteger(e, 0).i()).c_str(),
		    Util::urlencode(getString(e, 1).s()).c_str());
    } else {
      _logger->debug("e doesn't have 2 elements.");
    }
    throw DlAbortEx("Received Error DHT message.");
  } else if(y.s() != DHTResponseMessage::R) {
    throw DlAbortEx
      (StringFormat("Malformed DHT message. y != r: y=%s",
		    Util::urlencode(y.s()).c_str()).str());
  }
  const bencode::BDE& rDict = getDictionary(dict, DHTResponseMessage::R);
  const bencode::BDE& id = getString(rDict, DHTMessage::ID);
  validateID(id);
  SharedHandle<DHTNode> remoteNode = getRemoteNode(id.uc(), ipaddr, port);

  if(messageType == DHTPingReplyMessage::PING) {
    return createPingReplyMessage(remoteNode, id.uc(), transactionID.s());
  } else if(messageType == DHTFindNodeReplyMessage::FIND_NODE) {
    return createFindNodeReplyMessage(remoteNode, dict, transactionID.s());
  } else if(messageType == DHTGetPeersReplyMessage::GET_PEERS) {
    const bencode::BDE& valuesList = rDict[DHTGetPeersReplyMessage::VALUES];
    if(valuesList.isList()) {
      return createGetPeersReplyMessageWithValues(remoteNode, dict,
						  transactionID.s());
    } else {
      const bencode::BDE& nodes = rDict[DHTGetPeersReplyMessage::NODES];
      if(nodes.isString()) {
	return createGetPeersReplyMessageWithNodes(remoteNode, dict,
						   transactionID.s());
      } else {
	throw DlAbortEx("Malformed DHT message: missing nodes/values");
      }
    }
  } else if(messageType == DHTAnnouncePeerReplyMessage::ANNOUNCE_PEER) {
    return createAnnouncePeerReplyMessage(remoteNode, transactionID.s());
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
DHTMessageFactoryImpl::createFindNodeReplyMessage
(const SharedHandle<DHTNode>& remoteNode,
 const bencode::BDE& dict,
 const std::string& transactionID)
{
  const bencode::BDE& nodesData =
    getString(getDictionary(dict, DHTResponseMessage::R),
	      DHTFindNodeReplyMessage::NODES);
  std::deque<SharedHandle<DHTNode> > nodes = extractNodes(nodesData.uc(),
							  nodesData.s().size());
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
DHTMessageFactoryImpl::createGetPeersReplyMessageWithNodes
(const SharedHandle<DHTNode>& remoteNode,
 const bencode::BDE& dict,
 const std::string& transactionID)
{
  const bencode::BDE& rDict = getDictionary(dict, DHTResponseMessage::R);
  const bencode::BDE& nodesData = getString(rDict,
					    DHTGetPeersReplyMessage::NODES);
  std::deque<SharedHandle<DHTNode> > nodes = extractNodes(nodesData.uc(),
							  nodesData.s().size());
  const bencode::BDE& token = getString(rDict, DHTGetPeersReplyMessage::TOKEN);
  return createGetPeersReplyMessage(remoteNode, nodes, token.s(),
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
DHTMessageFactoryImpl::createGetPeersReplyMessageWithValues
(const SharedHandle<DHTNode>& remoteNode,
 const bencode::BDE& dict,
 const std::string& transactionID)
{
  const bencode::BDE& rDict = getDictionary(dict, DHTResponseMessage::R);
  const bencode::BDE& valuesList = getList(rDict,
					   DHTGetPeersReplyMessage::VALUES);
  std::deque<SharedHandle<Peer> > peers;
  for(bencode::BDE::List::const_iterator i = valuesList.listBegin();
      i != valuesList.listEnd(); ++i) {
    const bencode::BDE& data = *i;
    if(data.isString() && data.s().size() == 6) {
      std::pair<std::string, uint16_t> addr =
	PeerMessageUtil::unpackcompact(data.uc());
      PeerHandle peer(new Peer(addr.first, addr.second));
      peers.push_back(peer);
    }
  }
  const bencode::BDE& token = getString(rDict, DHTGetPeersReplyMessage::TOKEN);
  return createGetPeersReplyMessage(remoteNode, peers, token.s(),
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
