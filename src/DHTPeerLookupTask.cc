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
#include "DHTPeerLookupTask.h"
#include "Peer.h"
#include "DHTGetPeersReplyMessage.h"
#include "Logger.h"
#include "DHTMessageFactory.h"
#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "BtRegistry.h"
#include "PeerStorage.h"
#include "BtRuntime.h"
#include "BtContext.h"
#include "Util.h"
#include "DHTBucket.h"

namespace aria2 {

DHTPeerLookupTask::DHTPeerLookupTask(const SharedHandle<BtContext>& btContext):
  DHTAbstractNodeLookupTask(btContext->getInfoHash()),
  _ctx(btContext),
  _peerStorage(PEER_STORAGE(btContext)),
  _btRuntime(BT_RUNTIME(btContext)) {}

std::deque<SharedHandle<DHTNode> > DHTPeerLookupTask::getNodesFromMessage(const SharedHandle<DHTMessage>& message)
{
  SharedHandle<DHTGetPeersReplyMessage> m = message;
  if(m.isNull()) {
    return std::deque<SharedHandle<DHTNode> >();
  } else {
    return m->getClosestKNodes();
  }
}
  
void DHTPeerLookupTask::onReceivedInternal(const SharedHandle<DHTMessage>& message)
{
  SharedHandle<DHTGetPeersReplyMessage> m = message;
  if(m.isNull()) {
    return;
  }
  SharedHandle<DHTNode> remoteNode = m->getRemoteNode();
  _tokenStorage[Util::toHex(remoteNode->getID(), DHT_ID_LENGTH)] = m->getToken();

  _peerStorage->addPeer(m->getValues());
  _peers.insert(_peers.end(), m->getValues().begin(), m->getValues().end());
  _logger->info("Received %u peers.", m->getValues().size());
}
  
SharedHandle<DHTMessage> DHTPeerLookupTask::createMessage(const SharedHandle<DHTNode>& remoteNode)
{
  return _factory->createGetPeersMessage(remoteNode, _targetID);
}

void DHTPeerLookupTask::onFinish()
{
  // send announce_peer message to K closest nodes
  size_t num = DHTBucket::K;
  for(std::deque<SharedHandle<DHTNodeLookupEntry> >::const_iterator i = _entries.begin();
      i != _entries.end() && num > 0; ++i, --num) {
    if((*i)->_used) {
      const SharedHandle<DHTNode>& node = (*i)->_node;
      SharedHandle<DHTMessage> m = 
	_factory->createAnnouncePeerMessage(node,
					    _ctx->getInfoHash(),
					    _btRuntime->getListenPort(),
					    _tokenStorage[Util::toHex(node->getID(),
								      DHT_ID_LENGTH)]);
      _dispatcher->addMessageToQueue(m);
    }
  }
}

const std::deque<SharedHandle<Peer> >& DHTPeerLookupTask::getPeers() const
{
  return _peers;
}

} // namespace aria2
