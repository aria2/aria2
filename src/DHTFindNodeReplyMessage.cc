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
#include "DHTFindNodeReplyMessage.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTBucket.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "bittorrent_helper.h"
#include "util.h"

namespace aria2 {

const std::string DHTFindNodeReplyMessage::FIND_NODE("find_node");

const std::string DHTFindNodeReplyMessage::NODES("nodes");

const std::string DHTFindNodeReplyMessage::NODES6("nodes6");

DHTFindNodeReplyMessage::DHTFindNodeReplyMessage(
    int family, const std::shared_ptr<DHTNode>& localNode,
    const std::shared_ptr<DHTNode>& remoteNode,
    const std::string& transactionID)
    : DHTResponseMessage{localNode, remoteNode, transactionID}, family_{family}
{
}

void DHTFindNodeReplyMessage::doReceivedAction()
{
  for (auto& node : closestKNodes_) {
    if (memcmp(node->getID(), getLocalNode()->getID(), DHT_ID_LENGTH) != 0) {
      getRoutingTable()->addNode(node);
    }
  }
}

std::unique_ptr<Dict> DHTFindNodeReplyMessage::getResponse()
{
  auto aDict = Dict::g();
  aDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  unsigned char buffer[DHTBucket::K * 38];
  const int clen = bittorrent::getCompactLength(family_);
  const int unit = clen + 20;
  assert(unit <= 38);
  size_t offset = 0;
  size_t k = 0;
  for (auto i = std::begin(closestKNodes_), eoi = std::end(closestKNodes_);
       i != eoi && k < DHTBucket::K; ++i) {
    memcpy(buffer + offset, (*i)->getID(), DHT_ID_LENGTH);
    unsigned char compact[COMPACT_LEN_IPV6];
    int compactlen =
        bittorrent::packcompact(compact, (*i)->getIPAddress(), (*i)->getPort());
    if (compactlen == clen) {
      memcpy(buffer + 20 + offset, compact, compactlen);
      offset += unit;
      ++k;
    }
  }
  aDict->put(family_ == AF_INET ? NODES : NODES6, String::g(buffer, offset));
  return aDict;
}

const std::string& DHTFindNodeReplyMessage::getMessageType() const
{
  return FIND_NODE;
}

void DHTFindNodeReplyMessage::accept(DHTMessageCallback* callback)
{
  callback->visit(this);
}

void DHTFindNodeReplyMessage::setClosestKNodes(
    std::vector<std::shared_ptr<DHTNode>> closestKNodes)
{
  closestKNodes_ = std::move(closestKNodes);
}

std::string DHTFindNodeReplyMessage::toStringOptional() const
{
  return fmt("nodes=%lu", static_cast<unsigned long>(closestKNodes_.size()));
}

} // namespace aria2
