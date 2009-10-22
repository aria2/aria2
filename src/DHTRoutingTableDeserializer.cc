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

#include <cerrno>
#include <cstring>
#include <cassert>
#include <istream>
#include <utility>

#include "DHTNode.h"
#include "DHTConstants.h"
#include "bittorrent_helper.h"
#include "DlAbortEx.h"
#include "Logger.h"
#include "a2netcompat.h"
#include "StringFormat.h"
#include "util.h"
#include "array_fun.h"

namespace aria2 {

DHTRoutingTableDeserializer::DHTRoutingTableDeserializer() {}

DHTRoutingTableDeserializer::~DHTRoutingTableDeserializer() {}

static void readBytes(unsigned char* buf, size_t buflen,
		      std::istream& in, size_t readlen)
{
  assert(readlen <= buflen);
  in.read(reinterpret_cast<char*>(buf), readlen);
}

#define CHECK_STREAM(in, length)				\
  if(in.gcount() != length) {					\
  throw DL_ABORT_EX						\
  (StringFormat("Failed to load DHT routing table. cause:%s",	\
		"Unexpected EOF").str());			\
  }								\
  if(!in) {							\
  throw DL_ABORT_EX						\
  (StringFormat("Failed to load DHT routing table. cause:%s",	\
		strerror(errno)).str());			\
  }

void DHTRoutingTableDeserializer::deserialize(std::istream& in)
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

  char headerCompat[8];
  memset(headerCompat, 0, sizeof(headerCompat));
  // magic
  headerCompat[0] = 0xa1;
  headerCompat[1] = 0xa2;
  // format ID
  headerCompat[2] = 0x02;
  // version
  headerCompat[6] = 0;
  headerCompat[7] = 0x02;

  char zero[8];
  memset(zero, 0, sizeof(zero));

  int version;

  // If you change the code to read more than the size of buf, then
  // expand the buf size here.
  array_wrapper<unsigned char, 255> buf;

  // header
  readBytes(buf, buf.size(), in, 8);
  CHECK_STREAM(in, 8);
  if(memcmp(header, buf, 8) == 0) {
    version = 3;
  } else if(memcmp(headerCompat, buf, 8) == 0) {
    version = 2;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Failed to load DHT routing table. cause:%s",
		    "bad header").str());
  }

  uint32_t temp32;
  uint64_t temp64;
  // time
  if(version == 2) {
    in.read(reinterpret_cast<char*>(&temp32), sizeof(temp32));
    CHECK_STREAM(in, sizeof(temp32));
    _serializedTime.setTimeInSec(ntohl(temp32));
    // 4bytes reserved
    readBytes(buf, buf.size(), in, 4);
    CHECK_STREAM(in, 4);
  } else {
    in.read(reinterpret_cast<char*>(&temp64), sizeof(temp64));
    CHECK_STREAM(in, sizeof(temp64));
    _serializedTime.setTimeInSec(ntoh64(temp64));
  }
  
  // localnode
  // 8bytes reserved
  readBytes(buf, buf.size(), in, 8);
  CHECK_STREAM(in, 8);
  // localnode ID
  readBytes(buf, buf.size(), in, DHT_ID_LENGTH);
  CHECK_STREAM(in, DHT_ID_LENGTH);
  SharedHandle<DHTNode> localNode(new DHTNode(buf));
  // 4bytes reserved
  readBytes(buf, buf.size(), in, 4);
  CHECK_STREAM(in, 4);

  // number of nodes
  in.read(reinterpret_cast<char*>(&temp32), sizeof(temp32));
  CHECK_STREAM(in, sizeof(temp32));
  uint32_t numNodes = ntohl(temp32);
  // 4bytes reserved
  readBytes(buf, buf.size(), in, 4);
  CHECK_STREAM(in, 4);

  std::deque<SharedHandle<DHTNode> > nodes;
  // nodes
  for(size_t i = 0; i < numNodes; ++i) {
    // Currently, only IPv4 addresses are supported.
    // 1byte compact peer info length
    uint8_t peerInfoLen;
    in >> peerInfoLen;
    if(peerInfoLen != 6) {
      // skip this entry
      readBytes(buf, buf.size(), in, 42+7+6);
      CHECK_STREAM(in, 42+7+6);
      continue;
    }
    // 7bytes reserved
    readBytes(buf, buf.size(), in, 7);
    CHECK_STREAM(in, 7);
    // 6bytes compact peer info
    readBytes(buf, buf.size(), in, 6);
    CHECK_STREAM(in, 6);
    if(memcmp(zero, buf, 6) == 0) {
      // skip this entry
      readBytes(buf, buf.size(), in, 42);
      CHECK_STREAM(in, 42);
      continue;
    }
    std::pair<std::string, uint16_t> peer = bittorrent::unpackcompact(buf);
    if(peer.first.empty()) {
      // skip this entry
      readBytes(buf, buf.size(), in, 42);
      CHECK_STREAM(in, 42);
      continue;
    }
    // 2bytes reserved
    readBytes(buf, buf.size(), in, 2);
    CHECK_STREAM(in, 2);
    // 16byte reserved
    readBytes(buf, buf.size(), in, 16);
    CHECK_STREAM(in, 16);
    // localnode ID
    readBytes(buf, buf.size(), in, DHT_ID_LENGTH);
    CHECK_STREAM(in, DHT_ID_LENGTH);

    SharedHandle<DHTNode> node(new DHTNode(buf));
    node->setIPAddress(peer.first);
    node->setPort(peer.second);
    // 4bytes reserved
    readBytes(buf, buf.size(), in, 4);
    CHECK_STREAM(in, 4);

    nodes.push_back(node);
  }
  _localNode = localNode;
  _nodes = nodes;
}

} // namespace aria2
