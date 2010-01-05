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
#include "bencode.h"
#include "a2functional.h"

namespace aria2 {

const std::string DHTGetPeersReplyMessage::GET_PEERS("get_peers");

const std::string DHTGetPeersReplyMessage::TOKEN("token");

const std::string DHTGetPeersReplyMessage::VALUES("values");

const std::string DHTGetPeersReplyMessage::NODES("nodes");

DHTGetPeersReplyMessage::DHTGetPeersReplyMessage(const SharedHandle<DHTNode>& localNode,
                                                 const SharedHandle<DHTNode>& remoteNode,
                                                 const std::string& token,
                                                 const std::string& transactionID):
  DHTResponseMessage(localNode, remoteNode, transactionID),
  _token(token) {}

DHTGetPeersReplyMessage::~DHTGetPeersReplyMessage() {}

void DHTGetPeersReplyMessage::doReceivedAction()
{
  // Returned peers and nodes are handled in DHTPeerLookupTask.
}

BDE DHTGetPeersReplyMessage::getResponse()
{
  BDE rDict = BDE::dict();
  rDict[DHTMessage::ID] = BDE(_localNode->getID(), DHT_ID_LENGTH);
  rDict[TOKEN] = _token;
  if(_values.empty()) {
    size_t offset = 0;
    unsigned char buffer[DHTBucket::K*26];
    for(std::deque<SharedHandle<DHTNode> >::const_iterator i =
          _closestKNodes.begin();
        i != _closestKNodes.end() && offset < DHTBucket::K*26; ++i) {
      SharedHandle<DHTNode> node = *i;
      memcpy(buffer+offset, node->getID(), DHT_ID_LENGTH);
      if(bittorrent::createcompact
         (buffer+20+offset, node->getIPAddress(), node->getPort())) {
        offset += 26;
      }
    }
    rDict[NODES] = BDE(buffer, offset);
  } else {
    // Limit the size of values list.  The maxmum size of UDP datagram
    // is limited to 65535 bytes. aria2 uses 20bytes token and 2byte
    // transaction ID. The size of get_peers reply message without
    // values list is 87bytes:
    //
    // d1:rd2:id20:aaaaaaaaaaaaaaaaaaaa5:token20:aaaaaaaaaaaaaaaaaaaa
    // 6:valueslee1:t2:bb1:y1:re
    //
    // Because of Path MTU Discovery, UDP packet size which need not
    // to be fragmented is much smaller. Since Linux uses Path MTU
    // Dicoverry by default and returning ICMP message might be
    // filtered, we should avoid fragmentation.  MTU of pppoe is 1492
    // max according to RFC2516.  We use maximum packet size to be
    // 1000. Since it contains 20 bytes IP header and 8 bytes UDP
    // header and 87 bytes reply message template described above, We
    // can carry (1000-28-87)/8 = 110 peer info. Since DHT spec
    // doesn't specify the maximum size of token, reply message
    // template may get bigger than 87 bytes. So we use 100 as maximum
    // number of peer info that a message can carry.
    static const size_t MAX_VALUES_SIZE = 100;
    BDE valuesList = BDE::list();
    for(std::deque<SharedHandle<Peer> >::const_iterator i = _values.begin();
        i != _values.end() && valuesList.size() < MAX_VALUES_SIZE; ++i) {
      const SharedHandle<Peer>& peer = *i;
      unsigned char buffer[6];
      if(bittorrent::createcompact(buffer, peer->ipaddr, peer->port)) {
        valuesList << BDE(buffer, sizeof(buffer));
      }
    }
    rDict[VALUES] = valuesList;
  }
  return rDict;  
}

std::string DHTGetPeersReplyMessage::getMessageType() const
{
  return GET_PEERS;
}

void DHTGetPeersReplyMessage::validate() const {}

void DHTGetPeersReplyMessage::setClosestKNodes(const std::deque<SharedHandle<DHTNode> >& closestKNodes)
{
  _closestKNodes = closestKNodes;
}

void DHTGetPeersReplyMessage::setValues(const std::deque<SharedHandle<Peer> >& peers)
{
  _values = peers;
}

std::string DHTGetPeersReplyMessage::toStringOptional() const
{
  return strconcat("token=", util::toHex(_token),
                   ", values=", util::uitos(_values.size()),
                   ", nodes=", util::uitos(_closestKNodes.size()));
}

} // namespace aria2
