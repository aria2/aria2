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
#include "PeerListProcessor.h"
#include "DlAbortEx.h"
#include "message.h"
#include "StringFormat.h"
#include "bencode.h"
#include "a2functional.h"

namespace aria2 {

const std::string UTPexExtensionMessage::EXTENSION_NAME = "ut_pex";

UTPexExtensionMessage::UTPexExtensionMessage(uint8_t extensionMessageID):
  _extensionMessageID(extensionMessageID),
  _interval(DEFAULT_INTERVAL),
  _maxFreshPeer(DEFAULT_MAX_FRESH_PEER),
  _maxDroppedPeer(DEFAULT_MAX_DROPPED_PEER) {}

UTPexExtensionMessage::~UTPexExtensionMessage() {}

std::string UTPexExtensionMessage::getPayload()
{
  std::pair<std::string, std::string> freshPeerPair =
    createCompactPeerListAndFlag(_freshPeers);
  std::pair<std::string, std::string> droppedPeerPair =
    createCompactPeerListAndFlag(_droppedPeers);

  BDE dict = BDE::dict();
  dict["added"] = freshPeerPair.first;
  dict["added.f"] = freshPeerPair.second;
  dict["dropped"] = droppedPeerPair.first;
  return bencode::encode(dict);
}

std::pair<std::string, std::string>
UTPexExtensionMessage::createCompactPeerListAndFlag
(const std::vector<SharedHandle<Peer> >& peers)
{
  std::string addrstring;
  std::string flagstring;
  for(std::vector<SharedHandle<Peer> >::const_iterator itr = peers.begin(),
        eoi = peers.end(); itr != eoi; ++itr) {
    unsigned char compact[6];
    if(bittorrent::createcompact(compact, (*itr)->ipaddr, (*itr)->port)) {
      addrstring.append(&compact[0], &compact[6]);
      flagstring += (*itr)->isSeeder() ? "2" : "0";
    }
  }
  return std::pair<std::string, std::string>(addrstring, flagstring);
}

std::string UTPexExtensionMessage::toString() const
{
  return strconcat("ut_pex added=", util::uitos(_freshPeers.size()),
                   ", dropped=", util::uitos(_droppedPeers.size()));
}

void UTPexExtensionMessage::doReceivedAction()
{
  _peerStorage->addPeer(_freshPeers);
}

bool UTPexExtensionMessage::addFreshPeer(const SharedHandle<Peer>& peer)
{
  if(!peer->isIncomingPeer() &&
     !peer->getFirstContactTime().elapsed(_interval)) {
    _freshPeers.push_back(peer);
    return true;
  } else {
    return false;
  }
}

bool UTPexExtensionMessage::freshPeersAreFull() const
{
  return _freshPeers.size() >= _maxFreshPeer;
}

bool UTPexExtensionMessage::addDroppedPeer(const SharedHandle<Peer>& peer)
{
  if(!peer->isIncomingPeer() &&
     !peer->getBadConditionStartTime().elapsed(_interval)) {
    _droppedPeers.push_back(peer);
    return true;
  } else {
    return false;
  }
}

bool UTPexExtensionMessage::droppedPeersAreFull() const
{
  return _droppedPeers.size() >= _maxDroppedPeer;
}

void UTPexExtensionMessage::setMaxFreshPeer(size_t maxFreshPeer)
{
  _maxFreshPeer = maxFreshPeer;
}

void UTPexExtensionMessage::setMaxDroppedPeer(size_t maxDroppedPeer)
{
  _maxDroppedPeer = maxDroppedPeer;
}

void UTPexExtensionMessage::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  _peerStorage = peerStorage;
}

UTPexExtensionMessageHandle
UTPexExtensionMessage::create(const unsigned char* data, size_t len)
{
  if(len < 1) {
    throw DL_ABORT_EX(StringFormat(MSG_TOO_SMALL_PAYLOAD_SIZE,
                                   EXTENSION_NAME.c_str(), len).str());
  }
  UTPexExtensionMessageHandle msg(new UTPexExtensionMessage(*data));

  const BDE dict = bencode::decode(data+1, len-1);
  if(dict.isDict()) {
    PeerListProcessor proc;
    const BDE& added = dict["added"];
    if(added.isString()) {
      proc.extractPeerFromCompact(added, std::back_inserter(msg->_freshPeers));
    }
    const BDE& dropped = dict["dropped"];
    if(dropped.isString()) {
      proc.extractPeerFromCompact(dropped,
                                  std::back_inserter(msg->_droppedPeers));
    }
  }
  return msg;
}

} // namespace aria2
