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
#include "DHTRoutingTableSerializer.h"

#include <cerrno>
#include <cstring>
#include <ostream>

#include "DHTNode.h"
#include "DlAbortEx.h"
#include "DHTConstants.h"
#include "bittorrent_helper.h"
#include "Logger.h"
#include "a2netcompat.h"
#include "StringFormat.h"
#include "util.h"

namespace aria2 {

DHTRoutingTableSerializer::DHTRoutingTableSerializer() {}

DHTRoutingTableSerializer::~DHTRoutingTableSerializer() {}

void DHTRoutingTableSerializer::setLocalNode(const SharedHandle<DHTNode>& localNode)
{
  _localNode = localNode;
}

void DHTRoutingTableSerializer::setNodes(const std::deque<SharedHandle<DHTNode> >& nodes)
{
  _nodes = nodes;
}

void DHTRoutingTableSerializer::serialize(std::ostream& o)
{
  char header[8];
  memset(header, 0, sizeof(header));
  // magic
  header[0] = 0xa1;
  header[1] = 0xa2;
  // format ID
  header[2] = 0x02;
  // version
  header[6] = 0;
  header[7] = 0x03;
  
  char zero[16];
  memset(zero, 0, sizeof(zero));

  o.write(header, 8);
  // write save date
  uint64_t ntime = hton64(Time().getTime());
  o.write(reinterpret_cast<const char*>(&ntime), sizeof(ntime));

  // localnode
  // 8bytes reserved
  o.write(zero, 8);
  // 20bytes localnode ID
  o.write(reinterpret_cast<const char*>(_localNode->getID()), DHT_ID_LENGTH);
  // 4bytes reserved
  o.write(zero, 4);

  // number of nodes
  uint32_t numNodes = htonl(_nodes.size());
  o.write(reinterpret_cast<const char*>(&numNodes), sizeof(uint32_t));
  // 4bytes reserved
  o.write(zero, 4);

  // nodes
  for(std::deque<SharedHandle<DHTNode> >::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
    const SharedHandle<DHTNode>& node = *i;
    // Currently, only IPv4 address and IPv4-mapped address are saved.
    // 6bytes: write IP address + port in Compact IP-address/port info form.
    unsigned char compactPeer[6];
    if(!bittorrent::createcompact
       (compactPeer, node->getIPAddress(), node->getPort())) {
      memset(compactPeer, 0, 6);
    }
    // 1byte compact peer format length
    o << static_cast<uint8_t>(sizeof(compactPeer));
    // 7bytes reserved
    o.write(zero, 7);
    // 6 bytes compact peer
    o.write(reinterpret_cast<const char*>(compactPeer), 6);
    // 2bytes reserved
    o.write(zero, 2);
    // 16bytes reserved
    o.write(zero, 16);
    // 20bytes: node ID
    o.write(reinterpret_cast<const char*>(node->getID()), DHT_ID_LENGTH);
    // 4bytes reserved
    o.write(zero, 4);
  }

  o.flush();
  if(!o) {
    throw DL_ABORT_EX
      (StringFormat("Failed to save DHT routing table. cause:%s",
                    strerror(errno)).str());
  }
}

} // namespace aria2
