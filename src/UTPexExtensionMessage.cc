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
#include "UTPexExtensionMessage.h"
#include "Peer.h"
#include "util.h"
#include "bittorrent_helper.h"
#include "PeerStorage.h"
#include "DlAbortEx.h"
#include "message.h"
#include "fmt.h"
#include "bencode2.h"
#include "a2functional.h"
#include "wallclock.h"

namespace aria2 {

namespace {

// This is the hard limit of the number of "fresh peer" and "dropped
// peer".  This number is treated as the sum of IPv4 and IPv6 peers.
const size_t DEFAULT_MAX_FRESH_PEER = 50;
const size_t DEFAULT_MAX_DROPPED_PEER = 50;

} // namespace

const char UTPexExtensionMessage::EXTENSION_NAME[] = "ut_pex";

constexpr std::chrono::minutes UTPexExtensionMessage::DEFAULT_INTERVAL;

UTPexExtensionMessage::UTPexExtensionMessage(uint8_t extensionMessageID)
    : extensionMessageID_{extensionMessageID},
      peerStorage_{nullptr},
      interval_{DEFAULT_INTERVAL},
      maxFreshPeer_{DEFAULT_MAX_FRESH_PEER},
      maxDroppedPeer_{DEFAULT_MAX_DROPPED_PEER}
{
}

std::string UTPexExtensionMessage::getPayload()
{
  auto freshPeerPair = createCompactPeerListAndFlag(freshPeers_);
  auto droppedPeerPair = createCompactPeerListAndFlag(droppedPeers_);
  Dict dict;
  if (!freshPeerPair.first.first.empty()) {
    dict.put("added", freshPeerPair.first.first);
    dict.put("added.f", freshPeerPair.first.second);
  }
  if (!droppedPeerPair.first.first.empty()) {
    dict.put("dropped", droppedPeerPair.first.first);
  }
  if (!freshPeerPair.second.first.empty()) {
    dict.put("added6", freshPeerPair.second.first);
    dict.put("added6.f", freshPeerPair.second.second);
  }
  if (!droppedPeerPair.second.first.empty()) {
    dict.put("dropped6", droppedPeerPair.second.first);
  }

  return bencode2::encode(&dict);
}

std::pair<std::pair<std::string, std::string>,
          std::pair<std::string, std::string>>
UTPexExtensionMessage::createCompactPeerListAndFlag(
    const std::vector<std::shared_ptr<Peer>>& peers)
{
  std::string addrstring;
  std::string flagstring;
  std::string addrstring6;
  std::string flagstring6;
  for (auto itr = std::begin(peers), eoi = std::end(peers); itr != eoi; ++itr) {
    unsigned char compact[COMPACT_LEN_IPV6];
    int compactlen = bittorrent::packcompact(compact, (*itr)->getIPAddress(),
                                             (*itr)->getPort());
    if (compactlen == COMPACT_LEN_IPV4) {
      addrstring.append(&compact[0], &compact[compactlen]);
      flagstring += (*itr)->isSeeder() ? 0x02u : 0x00u;
    }
    else if (compactlen == COMPACT_LEN_IPV6) {
      addrstring6.append(&compact[0], &compact[compactlen]);
      flagstring6 += (*itr)->isSeeder() ? 0x02u : 0x00u;
    }
  }
  return std::make_pair(
      std::make_pair(std::move(addrstring), std::move(flagstring)),
      std::make_pair(std::move(addrstring6), std::move(flagstring6)));
}

std::string UTPexExtensionMessage::toString() const
{
  return fmt("ut_pex added=%lu, dropped=%lu",
             static_cast<unsigned long>(freshPeers_.size()),
             static_cast<unsigned long>(droppedPeers_.size()));
}

void UTPexExtensionMessage::doReceivedAction()
{
  peerStorage_->addPeer(freshPeers_);
  peerStorage_->addPeer(droppedPeers_);
}

bool UTPexExtensionMessage::addFreshPeer(const std::shared_ptr<Peer>& peer)
{
  if (!peer->isIncomingPeer() &&
      peer->getFirstContactTime().difference(global::wallclock()) < interval_) {
    freshPeers_.push_back(peer);
    return true;
  }
  else {
    return false;
  }
}

const std::vector<std::shared_ptr<Peer>>&
UTPexExtensionMessage::getFreshPeers() const
{
  return freshPeers_;
}

bool UTPexExtensionMessage::freshPeersAreFull() const
{
  return freshPeers_.size() >= maxFreshPeer_;
}

bool UTPexExtensionMessage::addDroppedPeer(const std::shared_ptr<Peer>& peer)
{
  if (!peer->isIncomingPeer() &&
      peer->getDropStartTime().difference(global::wallclock()) < interval_) {
    droppedPeers_.push_back(peer);
    return true;
  }
  else {
    return false;
  }
}

const std::vector<std::shared_ptr<Peer>>&
UTPexExtensionMessage::getDroppedPeers() const
{
  return droppedPeers_;
}

bool UTPexExtensionMessage::droppedPeersAreFull() const
{
  return droppedPeers_.size() >= maxDroppedPeer_;
}

void UTPexExtensionMessage::setMaxFreshPeer(size_t maxFreshPeer)
{
  maxFreshPeer_ = maxFreshPeer;
}

void UTPexExtensionMessage::setMaxDroppedPeer(size_t maxDroppedPeer)
{
  maxDroppedPeer_ = maxDroppedPeer;
}

void UTPexExtensionMessage::setPeerStorage(PeerStorage* peerStorage)
{
  peerStorage_ = peerStorage;
}

std::unique_ptr<UTPexExtensionMessage>
UTPexExtensionMessage::create(const unsigned char* data, size_t len)
{
  if (len < 1) {
    throw DL_ABORT_EX(fmt(MSG_TOO_SMALL_PAYLOAD_SIZE, EXTENSION_NAME,
                          static_cast<unsigned long>(len)));
  }
  auto msg = make_unique<UTPexExtensionMessage>(*data);

  auto decoded = bencode2::decode(data + 1, len - 1);
  const Dict* dict = downcast<Dict>(decoded);
  if (dict) {
    const String* added = downcast<String>(dict->get("added"));
    if (added) {
      bittorrent::extractPeer(added, AF_INET,
                              std::back_inserter(msg->freshPeers_));
    }
    const String* dropped = downcast<String>(dict->get("dropped"));
    if (dropped) {
      bittorrent::extractPeer(dropped, AF_INET,
                              std::back_inserter(msg->droppedPeers_));
    }
    const String* added6 = downcast<String>(dict->get("added6"));
    if (added6) {
      bittorrent::extractPeer(added6, AF_INET6,
                              std::back_inserter(msg->freshPeers_));
    }
    const String* dropped6 = downcast<String>(dict->get("dropped6"));
    if (dropped6) {
      bittorrent::extractPeer(dropped6, AF_INET6,
                              std::back_inserter(msg->droppedPeers_));
    }
  }
  return msg;
}

} // namespace aria2
