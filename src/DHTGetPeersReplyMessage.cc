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
#include "DHTGetPeersReplyMessage.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTBucket.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "bittorrent_helper.h"
#include "Peer.h"
#include "util.h"
#include "a2functional.h"

namespace aria2 {

const std::string DHTGetPeersReplyMessage::GET_PEERS("get_peers");

const std::string DHTGetPeersReplyMessage::TOKEN("token");

const std::string DHTGetPeersReplyMessage::VALUES("values");

const std::string DHTGetPeersReplyMessage::NODES("nodes");

const std::string DHTGetPeersReplyMessage::NODES6("nodes6");

DHTGetPeersReplyMessage::DHTGetPeersReplyMessage
(int family,
 const SharedHandle<DHTNode>& localNode,
 const SharedHandle<DHTNode>& remoteNode,
 const std::string& token,
 const std::string& transactionID):
  DHTResponseMessage(localNode, remoteNode, transactionID),
  family_(family),
  token_(token) {}

DHTGetPeersReplyMessage::~DHTGetPeersReplyMessage() {}

void DHTGetPeersReplyMessage::doReceivedAction()
{
  // Returned peers and nodes are handled in DHTPeerLookupTask.
}

SharedHandle<Dict> DHTGetPeersReplyMessage::getResponse()
{
  SharedHandle<Dict> rDict = Dict::g();
  rDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  rDict->put(TOKEN, token_);
  // TODO want parameter
  if(!closestKNodes_.empty()) {
    unsigned char buffer[DHTBucket::K*38];
    const int clen = bittorrent::getCompactLength(family_);
    const int unit = clen+20;
    size_t offset = 0;
    size_t k = 0;
    for(std::vector<SharedHandle<DHTNode> >::const_iterator i =
          closestKNodes_.begin(), eoi = closestKNodes_.end();
        i != eoi && k < DHTBucket::K; ++i) {
      SharedHandle<DHTNode> node = *i;
      memcpy(buffer+offset, node->getID(), DHT_ID_LENGTH);
      unsigned char compact[COMPACT_LEN_IPV6];
      int compactlen = bittorrent::packcompact
        (compact, node->getIPAddress(), node->getPort());
      if(compactlen == clen) {
        memcpy(buffer+20+offset, compact, compactlen);
        offset += unit;
        ++k;
      }
    }
    rDict->put(family_ == AF_INET?NODES:NODES6, String::g(buffer, offset));
  }
  if(!values_.empty()) {
    // Limit the size of values list.  The maxmum size of UDP datagram
    // is limited to 65535 bytes. aria2 uses 20bytes token and 2byte
    // transaction ID. The size of get_peers reply message without
    // values list and nodes is 87bytes:
    //
    // d1:rd2:id20:aaaaaaaaaaaaaaaaaaaa5:token20:aaaaaaaaaaaaaaaaaaaa
    // 6:valueslee1:t2:bb1:y1:re
    //
    // nodes are 38 bytes per host for IPv6 and the number of hosts is
    // K(=8) max. So without values list, we already 87+38*8+4 = 395.
    //
    // Because of Path MTU Discovery, UDP packet size which need not
    // to be fragmented is much smaller. Since Linux uses Path MTU
    // Dicoverry by default and returning ICMP message might be
    // filtered, we should avoid fragmentation.  MTU of pppoe is 1492
    // max according to RFC2516.  We use maximum packet size to be
    // 1024. Since it contains 20 bytes IP header and 8 bytes UDP
    // header and 395 bytes reply message template described above, We
    // can carry (1024-28-395)/(18+3) = 28 peer info. Since DHT spec
    // doesn't specify the maximum size of token, reply message
    // template may get bigger than 395 bytes. So we use 25 as maximum
    // number of peer info that a message can carry.
    static const size_t MAX_VALUES_SIZE = 25;
    SharedHandle<List> valuesList = List::g();
    for(std::vector<SharedHandle<Peer> >::const_iterator i = values_.begin(),
          eoi = values_.end(); i != eoi && valuesList->size() < MAX_VALUES_SIZE;
        ++i) {
      const SharedHandle<Peer>& peer = *i;
      unsigned char compact[COMPACT_LEN_IPV6];
      const int clen = bittorrent::getCompactLength(family_);
      int compactlen = bittorrent::packcompact
        (compact, peer->getIPAddress(), peer->getPort());
      if(compactlen == clen) {
        valuesList->append(String::g(compact, compactlen));
      }
    }
    rDict->put(VALUES, valuesList);
  }
  return rDict;
}

const std::string& DHTGetPeersReplyMessage::getMessageType() const
{
  return GET_PEERS;
}

void DHTGetPeersReplyMessage::accept(DHTMessageCallback* callback)
{
  callback->visit(this);
}

std::string DHTGetPeersReplyMessage::toStringOptional() const
{
  return fmt("token=%s, values=%lu, nodes=%lu",
             util::toHex(token_).c_str(),
             static_cast<unsigned long>(values_.size()),
             static_cast<unsigned long>(closestKNodes_.size()));
}

} // namespace aria2
