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
#ifndef _D_PEER_LIST_PROCESSOR_H_
#define _D_PEER_LIST_PROCESSOR_H_

#include "common.h"
#include "a2netcompat.h"
#include "bencode.h"
#include "Peer.h"

namespace aria2 {

class PeerListProcessor {
public:
  template<typename OutputIterator>
  void extractPeer(const bencode::BDE& peerData, OutputIterator dest)
  {
    if(peerData.isList()) {
      extractPeerFromList(peerData, dest);
    } else if(peerData.isString()) {
      extractPeerFromCompact(peerData, dest);
    }
  }

  template<typename OutputIterator>
  void extractPeerFromList(const bencode::BDE& peerData, OutputIterator dest)
  {
    for(bencode::BDE::List::const_iterator itr = peerData.listBegin();
	itr != peerData.listEnd(); ++itr) {
      const bencode::BDE& peerDict = *itr;
      if(!peerDict.isDict()) {
	continue;
      }
      static const std::string IP = "ip";
      static const std::string PORT = "port";
      const bencode::BDE& ip = peerDict[IP];
      const bencode::BDE& port = peerDict[PORT];
      if(!ip.isString() || !port.isInteger() ||
	 !(0 < port.i() && port.i() < 65536)) {
	continue;
      }
      *dest = SharedHandle<Peer>(new Peer(ip.s(), port.i()));
      ++dest;
    }
  }

  template<typename OutputIterator>
  void extractPeerFromCompact(const bencode::BDE& peerData, OutputIterator dest)
  {
    size_t length = peerData.s().size();
    if(length%6 == 0) {
      for(size_t i = 0; i < length; i += 6) {
	struct in_addr in;
	in.s_addr = *(uint32_t*)(peerData.s().c_str()+i);
	std::string ipaddr = inet_ntoa(in);
	uint16_t port = ntohs(*(uint16_t*)(peerData.s().c_str()+i+4));
	*dest = SharedHandle<Peer>(new Peer(ipaddr, port));
	++dest;
      }
    }
  }
};

} // namespace aria2

#endif // _D_PEER_LIST_PROCESSOR_H_
