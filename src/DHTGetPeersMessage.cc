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
#include "DHTGetPeersMessage.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "DHTPeerAnnounceStorage.h"
#include "Peer.h"
#include "DHTTokenTracker.h"
#include "util.h"

namespace aria2 {

const std::string DHTGetPeersMessage::GET_PEERS("get_peers");

const std::string DHTGetPeersMessage::INFO_HASH("info_hash");

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
  std::string token = _tokenTracker->generateToken
    (_infoHash, getRemoteNode()->getIPAddress(), getRemoteNode()->getPort());
  // Check to see localhost has the contents which has same infohash
  std::vector<SharedHandle<Peer> > peers;
  _peerAnnounceStorage->getPeers(peers, _infoHash);
  SharedHandle<DHTMessage> reply;
  if(peers.empty()) {
    std::vector<SharedHandle<DHTNode> > nodes;
    getRoutingTable()->getClosestKNodes(nodes, _infoHash);
    reply =
      getMessageFactory()->createGetPeersReplyMessage
      (getRemoteNode(), nodes, token, getTransactionID());
  } else {
    reply =
      getMessageFactory()->createGetPeersReplyMessage
      (getRemoteNode(), peers, token, getTransactionID());
  }
  getMessageDispatcher()->addMessageToQueue(reply);
}

SharedHandle<Dict> DHTGetPeersMessage::getArgument()
{
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  aDict->put(INFO_HASH, String::g(_infoHash, DHT_ID_LENGTH));
  return aDict;
}

const std::string& DHTGetPeersMessage::getMessageType() const
{
  return GET_PEERS;
}

void DHTGetPeersMessage::setPeerAnnounceStorage
(const WeakHandle<DHTPeerAnnounceStorage>& storage)
{
  _peerAnnounceStorage = storage;
}

void DHTGetPeersMessage::setTokenTracker
(const WeakHandle<DHTTokenTracker>& tokenTracker)
{
  _tokenTracker = tokenTracker;
}

std::string DHTGetPeersMessage::toStringOptional() const
{
  return "info_hash="+util::toHex(_infoHash, INFO_HASH_LENGTH);
}

} // namespace aria2
