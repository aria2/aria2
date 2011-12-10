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

#include <cstring>
#include <cstdio>

#include "DHTNode.h"
#include "DlAbortEx.h"
#include "DHTConstants.h"
#include "bittorrent_helper.h"
#include "Logger.h"
#include "a2netcompat.h"
#include "util.h"
#include "TimeA2.h"
#include "fmt.h"
#include "File.h"
#include "LogFactory.h"
#include "BufferedFile.h"

namespace aria2 {

DHTRoutingTableSerializer::DHTRoutingTableSerializer(int family):
  family_(family) {}

DHTRoutingTableSerializer::~DHTRoutingTableSerializer() {}

void DHTRoutingTableSerializer::setLocalNode
(const SharedHandle<DHTNode>& localNode)
{
  localNode_ = localNode;
}

void DHTRoutingTableSerializer::setNodes
(const std::vector<SharedHandle<DHTNode> >& nodes)
{
  nodes_ = nodes;
}

#define WRITE_CHECK(fp, ptr, count)                                     \
  if(fp.write((ptr), (count)) != (count)) {                             \
    throw DL_ABORT_EX(fmt("Failed to save DHT routing table to %s.",    \
                          filename.c_str()));                           \
  }

void DHTRoutingTableSerializer::serialize(const std::string& filename)
{
  A2_LOG_INFO(fmt("Saving DHT routing table to %s.", filename.c_str()));
  std::string filenameTemp = filename;
  filenameTemp += "__temp";
  BufferedFile fp(filenameTemp, BufferedFile::WRITE);
  if(!fp) {
    throw DL_ABORT_EX(fmt("Failed to save DHT routing table to %s.",
                          filename.c_str()));
  }
  char header[8];
  memset(header, 0, sizeof(header));
  // magic
  header[0] = 0xa1u;
  header[1] = 0xa2u;
  // format ID
  header[2] = 0x02u;
  // version
  header[6] = 0;
  header[7] = 0x03u;
  
  char zero[18];
  memset(zero, 0, sizeof(zero));

  WRITE_CHECK(fp, header, 8);
  // write save date
  uint64_t ntime = hton64(Time().getTime());
  WRITE_CHECK(fp, &ntime, sizeof(ntime));

  // localnode
  // 8bytes reserved
  WRITE_CHECK(fp, zero, 8);
  // 20bytes localnode ID
  WRITE_CHECK(fp, localNode_->getID(), DHT_ID_LENGTH);
  // 4bytes reserved
  WRITE_CHECK(fp, zero, 4);

  // number of nodes
  uint32_t numNodes = htonl(nodes_.size());
  WRITE_CHECK(fp, &numNodes, sizeof(uint32_t));
  // 4bytes reserved
  WRITE_CHECK(fp, zero, 4);

  const int clen = bittorrent::getCompactLength(family_);
  // nodes
  for(std::vector<SharedHandle<DHTNode> >::const_iterator i = nodes_.begin(),
        eoi = nodes_.end(); i != eoi; ++i) {
    const SharedHandle<DHTNode>& node = *i;
    // Write IP address + port in Compact IP-address/port info form.
    unsigned char compactPeer[COMPACT_LEN_IPV6];
    int compactlen = bittorrent::packcompact
      (compactPeer, node->getIPAddress(), node->getPort());
    if(compactlen != clen) {
      memset(compactPeer, 0, clen);
    }
    uint8_t clen1 = clen;
    // 1byte compact peer format length
    WRITE_CHECK(fp, &clen1, sizeof(clen1));
    // 7bytes reserved
    WRITE_CHECK(fp, zero, 7);
    // clen bytes compact peer
    WRITE_CHECK(fp, compactPeer, static_cast<size_t>(clen));
    // 24-clen bytes reserved
    WRITE_CHECK(fp, zero, static_cast<size_t>(24-clen));
    // 20bytes: node ID
    WRITE_CHECK(fp, node->getID(), DHT_ID_LENGTH);
    // 4bytes reserved
    WRITE_CHECK(fp, zero, 4);
  }
  if(fp.close() == EOF) {
    throw DL_ABORT_EX(fmt("Failed to save DHT routing table to %s.",
                          filename.c_str()));
  }
  if(!File(filenameTemp).renameTo(filename)) {
    throw DL_ABORT_EX(fmt("Failed to save DHT routing table to %s.",
                          filename.c_str()));
  }
  A2_LOG_INFO("DHT routing table was saved successfully");
}

} // namespace aria2
