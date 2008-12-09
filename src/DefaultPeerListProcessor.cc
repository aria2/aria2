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
#include "DefaultPeerListProcessor.h"
#include "Peer.h"
#include "bencode.h"

namespace aria2 {

const std::string DefaultPeerListProcessor::IP("ip");

const std::string DefaultPeerListProcessor::PORT("port");

DefaultPeerListProcessor::DefaultPeerListProcessor() {}

DefaultPeerListProcessor::~DefaultPeerListProcessor() {}

bool DefaultPeerListProcessor::canHandle(const bencode::BDE& peerData) const
{
  return peerData.isList();
}

void DefaultPeerListProcessor::extractPeer
(std::deque<SharedHandle<Peer> >& peers, const bencode::BDE& peerData)
{
  if(!canHandle(peerData)) {
    return;
  }
  for(bencode::BDE::List::const_iterator itr = peerData.listBegin();
      itr != peerData.listEnd(); ++itr) {
    const bencode::BDE& peerDict = *itr;
    if(!peerDict.isDict()) {
      continue;
    }
    const bencode::BDE& ip = peerDict[IP];
    const bencode::BDE& port = peerDict[PORT];
    if(!ip.isString() || !port.isInteger() ||
       !(0 < port.i() && port.i() < 65536)) {
      continue;
    }
    PeerHandle peer(new Peer(ip.s(), port.i()));
    peers.push_back(peer);
  }
}

} // namespace aria2
