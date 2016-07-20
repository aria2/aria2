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
#include <array>

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

DHTGetPeersReplyMessage::DHTGetPeersReplyMessage(
    int family, const std::shared_ptr<DHTNode>& localNode,
    const std::shared_ptr<DHTNode>& remoteNode, const std::string& token,
    const std::string& transactionID)
    : DHTResponseMessage{localNode, remoteNode, transactionID},
      family_{family},
      token_{token}
{
}

void DHTGetPeersReplyMessage::doReceivedAction()
{
  // Returned peers and nodes are handled in DHTPeerLookupTask.
}

std::unique_ptr<Dict> DHTGetPeersReplyMessage::getResponse()
{
  auto rDict = Dict::g();
  rDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  rDict->put(TOKEN, token_);
  // TODO want parameter
  if (!closestKNodes_.empty()) {
    std::array<unsigned char, DHTBucket::K * 38> buffer;
    const auto clen = bittorrent::getCompactLength(family_);
    auto last = std::begin(buffer);
    size_t k = 0;
    for (auto i = std::begin(closestKNodes_);
         i != std::end(closestKNodes_) && k < DHTBucket::K; ++i) {
      std::array<unsigned char, COMPACT_LEN_IPV6> compact;
      auto compactlen = bittorrent::packcompact(
          compact.data(), (*i)->getIPAddress(), (*i)->getPort());
      auto cclen =
          static_cast<std::make_unsigned<decltype(clen)>::type>((clen));
      if (clen >= 0 && compactlen == cclen) {
        last = std::copy_n((*i)->getID(), DHT_ID_LENGTH, last);
        last = std::copy_n(std::begin(compact), compactlen, last);
        ++k;
      }
    }
    rDict->put(family_ == AF_INET ? NODES : NODES6,
               String::g(std::begin(buffer), last));
  }
  if (!values_.empty()) {
    // Limit the size of values list.  The maximum size of UDP datagram
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
    // Discovery by default and returning ICMP message might be
    // filtered, we should avoid fragmentation.  MTU of pppoe is 1492
    // max according to RFC2516.  We use maximum packet size to be
    // 1024. Since it contains 20 bytes IP header and 8 bytes UDP
    // header and 395 bytes reply message template described above, We
    // can carry (1024-28-395)/(18+3) = 28 peer info. Since DHT spec
    // doesn't specify the maximum size of token, reply message
    // template may get bigger than 395 bytes. So we use 25 as maximum
    // number of peer info that a message can carry.
    constexpr size_t MAX_VALUES_SIZE = 25;
    auto valuesList = List::g();
    for (auto i = std::begin(values_);
         i != std::end(values_) && valuesList->size() < MAX_VALUES_SIZE; ++i) {
      std::array<unsigned char, COMPACT_LEN_IPV6> compact;
      const auto clen = bittorrent::getCompactLength(family_);
      auto compactlen = bittorrent::packcompact(
          compact.data(), (*i)->getIPAddress(), (*i)->getPort());
      auto cclen =
          static_cast<std::make_unsigned<decltype(clen)>::type>((clen));
      if (clen > 0 && compactlen == cclen) {
        valuesList->append(String::g(compact.data(), compactlen));
      }
    }
    rDict->put(VALUES, std::move(valuesList));
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
  return fmt("token=%s, values=%lu, nodes=%lu", util::toHex(token_).c_str(),
             static_cast<unsigned long>(values_.size()),
             static_cast<unsigned long>(closestKNodes_.size()));
}

void DHTGetPeersReplyMessage::setClosestKNodes(
    std::vector<std::shared_ptr<DHTNode>> closestKNodes)
{
  closestKNodes_ = std::move(closestKNodes);
}

void DHTGetPeersReplyMessage::setValues(
    std::vector<std::shared_ptr<Peer>> peers)
{
  values_ = std::move(peers);
}

} // namespace aria2
