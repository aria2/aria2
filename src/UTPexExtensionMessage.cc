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
#include "StringFormat.h"
#include "bencode2.h"
#include "a2functional.h"
#include "wallclock.h"

namespace aria2 {

const std::string UTPexExtensionMessage::EXTENSION_NAME = "ut_pex";

UTPexExtensionMessage::UTPexExtensionMessage(uint8_t extensionMessageID):
  extensionMessageID_(extensionMessageID),
  interval_(DEFAULT_INTERVAL),
  maxFreshPeer_(DEFAULT_MAX_FRESH_PEER),
  maxDroppedPeer_(DEFAULT_MAX_DROPPED_PEER) {}

UTPexExtensionMessage::~UTPexExtensionMessage() {}

std::string UTPexExtensionMessage::getPayload()
{
  std::pair<std::string, std::string> freshPeerPair =
    createCompactPeerListAndFlag(freshPeers_);
  std::pair<std::string, std::string> droppedPeerPair =
    createCompactPeerListAndFlag(droppedPeers_);

  Dict dict;
  dict.put("added", freshPeerPair.first);
  dict.put("added.f", freshPeerPair.second);
  dict.put("dropped", droppedPeerPair.first);
  return bencode2::encode(&dict);
}

std::pair<std::string, std::string>
UTPexExtensionMessage::createCompactPeerListAndFlag
(const std::vector<SharedHandle<Peer> >& peers)
{
  std::string addrstring;
  std::string flagstring;
  for(std::vector<SharedHandle<Peer> >::const_iterator itr = peers.begin(),
        eoi = peers.end(); itr != eoi; ++itr) {
    unsigned char compact[COMPACT_LEN_IPV6];
    int compactlen = bittorrent::packcompact
      (compact, (*itr)->getIPAddress(), (*itr)->getPort());
    if(compactlen == COMPACT_LEN_IPV4) {
      addrstring.append(&compact[0], &compact[compactlen]);
      flagstring += (*itr)->isSeeder() ? 0x02 : 0x00;
    }
  }
  return std::pair<std::string, std::string>(addrstring, flagstring);
}

std::string UTPexExtensionMessage::toString() const
{
  return strconcat("ut_pex added=", util::uitos(freshPeers_.size()),
                   ", dropped=", util::uitos(droppedPeers_.size()));
}

void UTPexExtensionMessage::doReceivedAction()
{
  peerStorage_->addPeer(freshPeers_);
}

bool UTPexExtensionMessage::addFreshPeer(const SharedHandle<Peer>& peer)
{
  if(!peer->isIncomingPeer() &&
     peer->getFirstContactTime().difference(global::wallclock) < interval_) {
    freshPeers_.push_back(peer);
    return true;
  } else {
    return false;
  }
}

bool UTPexExtensionMessage::freshPeersAreFull() const
{
  return freshPeers_.size() >= maxFreshPeer_;
}

bool UTPexExtensionMessage::addDroppedPeer(const SharedHandle<Peer>& peer)
{
  if(!peer->isIncomingPeer() &&
     peer->getBadConditionStartTime().
     difference(global::wallclock) < interval_) {
    droppedPeers_.push_back(peer);
    return true;
  } else {
    return false;
  }
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

void UTPexExtensionMessage::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

UTPexExtensionMessageHandle
UTPexExtensionMessage::create(const unsigned char* data, size_t len)
{
  if(len < 1) {
    throw DL_ABORT_EX(StringFormat(MSG_TOO_SMALL_PAYLOAD_SIZE,
                                   EXTENSION_NAME.c_str(), len).str());
  }
  UTPexExtensionMessageHandle msg(new UTPexExtensionMessage(*data));

  SharedHandle<ValueBase> decoded = bencode2::decode(data+1, len-1);
  const Dict* dict = asDict(decoded);
  if(dict) {
    const String* added = asString(dict->get("added"));
    if(added) {
      bittorrent::extractPeer
        (added, AF_INET,  std::back_inserter(msg->freshPeers_));
    }
    const String* dropped = asString(dict->get("dropped"));
    if(dropped) {
      bittorrent::extractPeer
        (dropped, AF_INET, std::back_inserter(msg->droppedPeers_));
    }
  }
  return msg;
}

} // namespace aria2
