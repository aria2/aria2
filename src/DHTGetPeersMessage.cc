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
#include "DHTGetPeersMessage.h"
#include "DHTNode.h"
#include "Data.h"
#include "Dictionary.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTUtil.h"
#include "Peer.h"
#include "DHTTokenTracker.h"
#include "Util.h"
#include <cstring>

namespace aria2 {

DHTGetPeersMessage::DHTGetPeersMessage(const SharedHandle<DHTNode>& localNode,
				       const SharedHandle<DHTNode>& remoteNode,
				       const unsigned char* infoHash,
				       const std::string& transactionID):
  DHTQueryMessage(localNode, remoteNode, transactionID)
{
  memcpy(_infoHash, infoHash, DHT_ID_LENGTH);
}

DHTGetPeersMessage::~DHTGetPeersMessage() {}

void DHTGetPeersMessage::doReceivedAction()
{
  std::string token = _tokenTracker->generateToken(_infoHash,
					      _remoteNode->getIPAddress(),
					      _remoteNode->getPort());
  // Check to see localhost has the contents which has same infohash
  Peers peers = _peerAnnounceStorage->getPeers(_infoHash);
  SharedHandle<DHTMessage> reply;
  if(peers.empty()) {
    std::deque<SharedHandle<DHTNode> > nodes;
    _routingTable->getClosestKNodes(nodes, _infoHash);
    reply =
      _factory->createGetPeersReplyMessage(_remoteNode, nodes, token,
					   _transactionID);
  } else {
    reply =
      _factory->createGetPeersReplyMessage(_remoteNode, peers, token,
					   _transactionID);
  }
  _dispatcher->addMessageToQueue(reply);
}

Dictionary* DHTGetPeersMessage::getArgument()
{
  Dictionary* a = new Dictionary();
  a->put("id", new Data(_localNode->getID(), DHT_ID_LENGTH));
  a->put("info_hash", new Data(_infoHash, DHT_ID_LENGTH));
  return a;
}

std::string DHTGetPeersMessage::getMessageType() const
{
  return "get_peers";
}

void DHTGetPeersMessage::validate() const {}

void DHTGetPeersMessage::setPeerAnnounceStorage(const WeakHandle<DHTPeerAnnounceStorage>& storage)
{
  _peerAnnounceStorage = storage;
}

void DHTGetPeersMessage::setTokenTracker(const WeakHandle<DHTTokenTracker>& tokenTracker)
{
  _tokenTracker = tokenTracker;
}

std::string DHTGetPeersMessage::toStringOptional() const
{
  return "info_hash="+Util::toHex(_infoHash, INFO_HASH_LENGTH);
}

} // namespace aria2
