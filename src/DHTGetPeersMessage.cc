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
#include <array>

#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "DHTPeerAnnounceStorage.h"
#include "Peer.h"
#include "DHTTokenTracker.h"
#include "DHTGetPeersReplyMessage.h"
#include "util.h"
#include "BtRegistry.h"
#include "DownloadContext.h"
#include "Option.h"
#include "SocketCore.h"

namespace aria2 {

const std::string DHTGetPeersMessage::GET_PEERS("get_peers");

const std::string DHTGetPeersMessage::INFO_HASH("info_hash");

DHTGetPeersMessage::DHTGetPeersMessage(
    const std::shared_ptr<DHTNode>& localNode,
    const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* infoHash,
    const std::string& transactionID)
    : DHTQueryMessage{localNode, remoteNode, transactionID},
      peerAnnounceStorage_{nullptr},
      tokenTracker_{nullptr},
      btRegistry_{nullptr},
      family_{AF_INET}
{
  memcpy(infoHash_, infoHash, DHT_ID_LENGTH);
}

void DHTGetPeersMessage::addLocalPeer(std::vector<std::shared_ptr<Peer>>& peers)
{
  if (!btRegistry_) {
    return;
  }

  auto& dctx = btRegistry_->getDownloadContext(
      std::string(infoHash_, infoHash_ + DHT_ID_LENGTH));

  if (!dctx) {
    return;
  }

  auto group = dctx->getOwnerRequestGroup();
  auto& option = group->getOption();
  auto& externalIP = option->get(PREF_BT_EXTERNAL_IP);

  if (externalIP.empty()) {
    return;
  }

  std::array<uint8_t, sizeof(struct in6_addr)> dst;
  if (inetPton(family_, externalIP.c_str(), dst.data()) == -1) {
    return;
  }

  auto tcpPort = btRegistry_->getTcpPort();
  if (std::find_if(std::begin(peers), std::end(peers),
                   [&externalIP, tcpPort](const std::shared_ptr<Peer>& peer) {
                     return peer->getIPAddress() == externalIP &&
                            peer->getPort() == tcpPort;
                   }) != std::end(peers)) {
    return;
  }

  peers.push_back(std::make_shared<Peer>(externalIP, tcpPort));
}

void DHTGetPeersMessage::doReceivedAction()
{
  std::string token = tokenTracker_->generateToken(
      infoHash_, getRemoteNode()->getIPAddress(), getRemoteNode()->getPort());
  std::vector<std::shared_ptr<Peer>> peers;
  peerAnnounceStorage_->getPeers(peers, infoHash_);

  // Check to see localhost has the contents which has same infohash
  addLocalPeer(peers);

  std::vector<std::shared_ptr<DHTNode>> nodes;
  getRoutingTable()->getClosestKNodes(nodes, infoHash_);
  getMessageDispatcher()->addMessageToQueue(
      getMessageFactory()->createGetPeersReplyMessage(
          getRemoteNode(), std::move(nodes), std::move(peers), token,
          getTransactionID()));
}

std::unique_ptr<Dict> DHTGetPeersMessage::getArgument()
{
  auto aDict = Dict::g();
  aDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  aDict->put(INFO_HASH, String::g(infoHash_, DHT_ID_LENGTH));
  return aDict;
}

const std::string& DHTGetPeersMessage::getMessageType() const
{
  return GET_PEERS;
}

void DHTGetPeersMessage::setPeerAnnounceStorage(DHTPeerAnnounceStorage* storage)
{
  peerAnnounceStorage_ = storage;
}

void DHTGetPeersMessage::setTokenTracker(DHTTokenTracker* tokenTracker)
{
  tokenTracker_ = tokenTracker;
}

void DHTGetPeersMessage::setBtRegistry(BtRegistry* btRegistry)
{
  btRegistry_ = btRegistry;
}

void DHTGetPeersMessage::setFamily(int family) { family_ = family; }

std::string DHTGetPeersMessage::toStringOptional() const
{
  return "info_hash=" + util::toHex(infoHash_, INFO_HASH_LENGTH);
}

} // namespace aria2
