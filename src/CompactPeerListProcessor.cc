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
#include "CompactPeerListProcessor.h"
#include "a2netcompat.h"
#include "bencode.h"
#include "Peer.h"

namespace aria2 {

CompactPeerListProcessor::CompactPeerListProcessor() {}

CompactPeerListProcessor::~CompactPeerListProcessor() {}

bool CompactPeerListProcessor::canHandle(const bencode::BDE& peerData) const
{
  return peerData.isString();
}

void CompactPeerListProcessor::extractPeer
(std::deque<SharedHandle<Peer> >& peers, const bencode::BDE& peerData)
{
  if(!canHandle(peerData)) {
    return;
  }
  size_t length = peerData.s().size();
  if(length%6 == 0) {
    for(size_t i = 0; i < length; i += 6) {
      struct in_addr in;
      in.s_addr = *(uint32_t*)(peerData.s().c_str()+i);
      std::string ipaddr = inet_ntoa(in);
      uint16_t port = ntohs(*(uint16_t*)(peerData.s().c_str()+i+4));
      PeerHandle peer(new Peer(ipaddr, port));
      peers.push_back(peer);
    }
  }
}

} // namespace aria2
