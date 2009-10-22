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
#include "bittorrent_helper.h"
#include "BtRuntime.h"
#include "util.h"
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

static const BDE& getDictionary(const BDE& dict,
					 const std::string& key)
{
  const BDE& d = dict[key];
  if(d.isDict()) {
    return d;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const BDE& getString(const BDE& dict,
				     const std::string& key)
{
  const BDE& c = dict[key];
  if(c.isString()) {
    return c;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const BDE& getInteger(const BDE& dict,
				      const std::string& key)
{
  const BDE& c = dict[key];
  if(c.isInteger()) {
    return c;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

static const BDE& getString(const BDE& list, size_t index)
{
  const BDE& c = list[index];
  if(c.isString()) {
    return c;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. element[%u] is not String.",
		    index).str());
  }
}

static const BDE& getInteger(const BDE& list, size_t index)
{
  const BDE& c = list[index];
  if(c.isInteger()) {
    return c;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. element[%u] is not Integer.",
		    index).str());
  }
}

static const BDE& getList(const BDE& dict,
				   const std::string& key)
{
  const BDE& l = dict[key];
  if(l.isList()) {
    return l;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. Missing %s", key.c_str()).str());
  }
}

void DHTMessageFactoryImpl::validateID(const BDE& id) const
{
  if(id.s().size() != DHT_ID_LENGTH) {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. Invalid ID length."
		    " Expected:%d, Actual:%d",
		    DHT_ID_LENGTH, id.s().size()).str());
  }
}

void DHTMessageFactoryImpl::validatePort(const BDE& i) const
{
  BDE::Integer port = i.i();
  if(!(0 < port && port < UINT16_MAX)) {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. Invalid port=%s",
		    util::itos(port).c_str()).str());
  }
}

SharedHandle<DHTMessage> DHTMessageFactoryImpl::createQueryMessage
(const BDE& dict,
 const std::string& ipaddr,
 uint16_t port)
{
  const BDE& messageType = getString(dict, DHTQueryMessage::Q);
  const BDE& transactionID = getString(dict, DHTMessage::T);
  const BDE& y = getString(dict, DHTMessage::Y);
  const BDE& aDict = getDictionary(dict, DHTQueryMessage::A);
  if(y.s() != DHTQueryMessage::Q) {
    throw DL_ABORT_EX("Malformed DHT message. y != q");
  }
  const BDE& id = getString(aDict, DHTMessage::ID);
  validateID(id);
  SharedHandle<DHTNode> remoteNode = getRemoteNode(id.uc(), ipaddr, port);
  if(messageType.s() == DHTPingMessage::PING) {
    return createPingMessage(remoteNode, transactionID.s());
  } else if(messageType.s() == DHTFindNodeMessage::FIND_NODE) {
    const BDE& targetNodeID =
      getString(aDict, DHTFindNodeMessage::TARGET_NODE);
    validateID(targetNodeID);
    return createFindNodeMessage(remoteNode, targetNodeID.uc(),
				 transactionID.s());
  } else if(messageType.s() == DHTGetPeersMessage::GET_PEERS) {
    const BDE& infoHash = 
      getString(aDict, DHTGetPeersMessage::INFO_HASH);
    validateID(infoHash);
    return createGetPeersMessage(remoteNode,
				 infoHash.uc(), transactionID.s());
  } else if(messageType.s() == DHTAnnouncePeerMessage::ANNOUNCE_PEER) {
    const BDE& infoHash =
      getString(aDict, DHTAnnouncePeerMessage::INFO_HASH);
    validateID(infoHash);
    const BDE& port = getInteger(aDict, DHTAnnouncePeerMessage::PORT);
    validatePort(port);
    const BDE& token = getString(aDict, DHTAnnouncePeerMessage::TOKEN);
    return createAnnouncePeerMessage(remoteNode, infoHash.uc(),
				     static_cast<uint16_t>(port.i()),
				     token.s(), transactionID.s());
  } else {
    throw DL_ABORT_EX
      (StringFormat("Unsupported message type: %s",
		    messageType.s().c_str()).str());
  }
}

SharedHandle<DHTMessage>
DHTMessageFactoryImpl::createResponseMessage(const std::string& messageType,
					     const BDE& dict,
					     const std::string& ipaddr,
					     uint16_t port)
{
  const BDE& transactionID = getString(dict, DHTMessage::T);
  const BDE& y = getString(dict, DHTMessage::Y);
  if(y.s() == DHTUnknownMessage::E) {
    // for now, just report error message arrived and throw exception.
    const BDE& e = getList(dict, DHTUnknownMessage::E);
    if(e.size() == 2) {
      _logger->info("Received Error DHT message. code=%s, msg=%s",
		    util::itos(getInteger(e, 0).i()).c_str(),
		    util::urlencode(getString(e, 1).s()).c_str());
    } else {
      _logger->debug("e doesn't have 2 elements.");
    }
    throw DL_ABORT_EX("Received Error DHT message.");
  } else if(y.s() != DHTResponseMessage::R) {
    throw DL_ABORT_EX
      (StringFormat("Malformed DHT message. y != r: y=%s",
		    util::urlencode(y.s()).c_str()).str());
  }
  const BDE& rDict = getDictionary(dict, DHTResponseMessage::R);
  const BDE& id = getString(rDict, DHTMessage::ID);
  validateID(id);
  SharedHandle<DHTNode> remoteNode = getRemoteNode(id.uc(), ipaddr, port);

  if(messageType == DHTPingReplyMessage::PING) {
    return createPingReplyMessage(remoteNode, id.uc(), transactionID.s());
  } else if(messageType == DHTFindNodeReplyMessage::FIND_NODE) {
    return createFindNodeReplyMessage(remoteNode, dict, transactionID.s());
  } else if(messageType == DHTGetPeersReplyMessage::GET_PEERS) {
    const BDE& valuesList = rDict[DHTGetPeersReplyMessage::VALUES];
    if(valuesList.isList()) {
      return createGetPeersReplyMessageWithValues(remoteNode, dict,
						  transactionID.s());
    } else {
      const BDE& nodes = rDict[DHTGetPeersReplyMessage::NODES];
      if(nodes.isString()) {
	return createGetPeersReplyMessageWithNodes(remoteNode, dict,
						   transactionID.s());
      } else {
	throw DL_ABORT_EX("Malformed DHT message: missing nodes/values");
      }
    }
  } else if(messageType == DHTAnnouncePeerReplyMessage::ANNOUNCE_PEER) {
    return createAnnouncePeerReplyMessage(remoteNode, transactionID.s());
  } else {
    throw DL_ABORT_EX
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
    throw DL_ABORT_EX("Nodes length is not multiple of 26");
  }
  std::deque<SharedHandle<DHTNode> > nodes;
  for(size_t offset = 0; offset < length; offset += 26) {
    SharedHandle<DHTNode> node(new DHTNode(src+offset));
    std::pair<std::string, uint16_t> addr =
      bittorrent::unpackcompact(src+offset+DHT_ID_LENGTH);
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
 const BDE& dict,
 const std::string& transactionID)
{
  const BDE& nodesData =
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
 const BDE& dict,
 const std::string& transactionID)
{
  const BDE& rDict = getDictionary(dict, DHTResponseMessage::R);
  const BDE& nodesData = getString(rDict,
					    DHTGetPeersReplyMessage::NODES);
  std::deque<SharedHandle<DHTNode> > nodes = extractNodes(nodesData.uc(),
							  nodesData.s().size());
  const BDE& token = getString(rDict, DHTGetPeersReplyMessage::TOKEN);
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
 const BDE& dict,
 const std::string& transactionID)
{
  const BDE& rDict = getDictionary(dict, DHTResponseMessage::R);
  const BDE& valuesList = getList(rDict,
					   DHTGetPeersReplyMessage::VALUES);
  std::deque<SharedHandle<Peer> > peers;
  for(BDE::List::const_iterator i = valuesList.listBegin();
      i != valuesList.listEnd(); ++i) {
    const BDE& data = *i;
    if(data.isString() && data.s().size() == 6) {
      std::pair<std::string, uint16_t> addr =
	bittorrent::unpackcompact(data.uc());
      PeerHandle peer(new Peer(addr.first, addr.second));
      peers.push_back(peer);
    }
  }
  const BDE& token = getString(rDict, DHTGetPeersReplyMessage::TOKEN);
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
