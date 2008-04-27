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
#include "DHTRoutingTableDeserializer.h"
#include "DHTNode.h"
#include "DHTConstants.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"
#include "Logger.h"
#include "a2netcompat.h"
#include "StringFormat.h"
#include <cerrno>
#include <cstring>
#include <istream>
#include <utility>

namespace aria2 {

DHTRoutingTableDeserializer::DHTRoutingTableDeserializer() {}

DHTRoutingTableDeserializer::~DHTRoutingTableDeserializer() {}

SharedHandle<DHTNode> DHTRoutingTableDeserializer::getLocalNode() const
{
  return _localNode;
}

const std::deque<SharedHandle<DHTNode> >& DHTRoutingTableDeserializer::getNodes() const
{
  return _nodes;
}

void DHTRoutingTableDeserializer::deserialize(std::istream& in)
{
  try {
    char header[8];
    memset(header, 0, sizeof(header));
    // magic
    header[0] = 0xa1;
    header[1] = 0xa2;
    // format ID
    header[2] = 0x02;
    // version
    header[6] = 0;
    header[7] = 0x02;

    char zero[8];
    memset(zero, 0, sizeof(zero));

    char buf[26];
    // header
    in.read(buf, 8);
    if(memcmp(header, buf, 8) != 0) {
      throw DlAbortEx
	(StringFormat("Failed to load DHT routing table. cause:%s",
		      "bad header").str());
    }
    // time
    in.read(buf, 4);
    _serializedTime.setTimeInSec(ntohl(*reinterpret_cast<uint32_t*>(buf)));
    // 4bytes reserved
    in.read(buf, 4);
  
    // localnode
    // 8bytes reserved
    in.read(buf, 8);
    // localnode ID
    in.read(buf, DHT_ID_LENGTH);
    SharedHandle<DHTNode> localNode(new DHTNode(reinterpret_cast<const unsigned char*>(buf)));
    // 4bytes reserved
    in.read(buf, 4);

    // number of nodes
    in.read(buf, 4);
    uint32_t numNodes = ntohl(*reinterpret_cast<uint32_t*>(buf));
    // 4bytes reserved
    in.read(buf, 4);

    // nodes
    for(size_t i = 0; i < numNodes; ++i) {
      // Currently, only IPv4 addresses are supported.
      // 1byte compact peer info length
      uint8_t peerInfoLen;
      in >> peerInfoLen;
      if(peerInfoLen != 6) {
	// skip this entry
	in.read(buf, 42+7+6);
	continue;
      }
      // 7bytes reserved
      in.read(buf, 7);
      // 6bytes compact peer info
      in.read(buf, 6);
      if(memcmp(zero, buf, 6) == 0) {
	// skip this entry
	in.read(buf, 42);
	continue;
      }
      std::pair<std::string, uint16_t> peer =
	PeerMessageUtil::unpackcompact(reinterpret_cast<const unsigned char*>(buf));
      if(peer.first.empty()) {
	// skip this entry
	in.read(buf, 42);
	continue;
      }
      // 2bytes reserved
      in.read(buf, 2);
      // 16byte reserved
      in.read(buf, 16);
      // localnode ID
      in.read(buf, DHT_ID_LENGTH);

      SharedHandle<DHTNode> node(new DHTNode(reinterpret_cast<const unsigned char*>(buf)));
      node->setIPAddress(peer.first);
      node->setPort(peer.second);
      // 4bytes reserved
      in.read(buf, 4);

      _nodes.push_back(node);
    }
    _localNode = localNode;
  } catch(std::ios::failure const& exception) {
    _nodes.clear();
    throw DlAbortEx
      (StringFormat("Failed to load DHT routing table. cause:%s",
		    strerror(errno)).str());
  }
}

} // namespace aria2
