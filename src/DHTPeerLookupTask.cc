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
#include "DHTPeerLookupTask.h"
#include "Peer.h"
#include "DHTGetPeersReplyMessage.h"
#include "Logger.h"
#include "LogFactory.h"
#include "DHTMessageFactory.h"
#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "PeerStorage.h"
#include "util.h"
#include "DHTBucket.h"
#include "bittorrent_helper.h"
#include "DHTPeerLookupTaskCallback.h"
#include "DHTQueryMessage.h"
#include "DHTGetPeersMessage.h"
#include "DHTAnnouncePeerMessage.h"

#include "fmt.h"

namespace aria2 {

DHTPeerLookupTask::DHTPeerLookupTask(
    const std::shared_ptr<DownloadContext>& downloadContext, uint16_t tcpPort)
    : DHTAbstractNodeLookupTask<DHTGetPeersReplyMessage>(
          bittorrent::getInfoHash(downloadContext)),
      tcpPort_(tcpPort)
{
}

void DHTPeerLookupTask::getNodesFromMessage(
    std::vector<std::shared_ptr<DHTNode>>& nodes,
    const DHTGetPeersReplyMessage* message)
{
  auto& knodes = message->getClosestKNodes();
  nodes.insert(std::end(nodes), std::begin(knodes), std::end(knodes));
}

void DHTPeerLookupTask::onReceivedInternal(
    const DHTGetPeersReplyMessage* message)
{
  std::shared_ptr<DHTNode> remoteNode = message->getRemoteNode();
  tokenStorage_[util::toHex(remoteNode->getID(), DHT_ID_LENGTH)] =
      message->getToken();
  peerStorage_->addPeer(message->getValues());
  A2_LOG_INFO(fmt("Received %lu peers.",
                  static_cast<unsigned long>(message->getValues().size())));
}

std::unique_ptr<DHTMessage>
DHTPeerLookupTask::createMessage(const std::shared_ptr<DHTNode>& remoteNode)
{
  return getMessageFactory()->createGetPeersMessage(remoteNode, getTargetID());
}

std::unique_ptr<DHTMessageCallback> DHTPeerLookupTask::createCallback()
{
  return make_unique<DHTPeerLookupTaskCallback>(this);
}

void DHTPeerLookupTask::onFinish()
{
  A2_LOG_DEBUG(fmt("Peer lookup for %s finished",
                   util::toHex(getTargetID(), DHT_ID_LENGTH).c_str()));
  // send announce_peer message to K closest nodes
  size_t num = DHTBucket::K;
  for (auto i = std::begin(getEntries()), eoi = std::end(getEntries());
       i != eoi && num > 0; ++i) {
    if (!(*i)->used) {
      continue;
    }
    auto& node = (*i)->node;
    std::string idHex = util::toHex(node->getID(), DHT_ID_LENGTH);
    std::string token = tokenStorage_[idHex];
    if (token.empty()) {
      A2_LOG_DEBUG(fmt("Token is empty for ID:%s", idHex.c_str()));
      continue;
    }
    getMessageDispatcher()->addMessageToQueue(
        getMessageFactory()->createAnnouncePeerMessage(
            node,
            getTargetID(), // this is infoHash
            tcpPort_, token));
    --num;
  }
}

void DHTPeerLookupTask::setPeerStorage(const std::shared_ptr<PeerStorage>& ps)
{
  peerStorage_ = ps;
}

} // namespace aria2
