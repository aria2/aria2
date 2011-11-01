/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "LpdReceiveMessageCommand.h"
#include "DownloadEngine.h"
#include "SocketCore.h"
#include "LpdMessageReceiver.h"
#include "RequestGroupMan.h"
#include "DownloadContext.h"
#include "PeerStorage.h"
#include "Peer.h"
#include "RequestGroup.h"
#include "BtRegistry.h"
#include "Logger.h"
#include "LogFactory.h"
#include "LpdMessage.h"
#include "bittorrent_helper.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

LpdReceiveMessageCommand::LpdReceiveMessageCommand
(cuid_t cuid,
 const SharedHandle<LpdMessageReceiver>& receiver,
 DownloadEngine* e)
  : Command(cuid),
    receiver_(receiver),
    e_(e)
{
  e_->addSocketForReadCheck(receiver_->getSocket(), this);
}

LpdReceiveMessageCommand::~LpdReceiveMessageCommand()
{
  e_->deleteSocketForReadCheck(receiver_->getSocket(), this);
}

bool LpdReceiveMessageCommand::execute()
{
  if(e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  for(size_t i = 0; i < 20; ++i) {
    SharedHandle<LpdMessage> m = receiver_->receiveMessage();
    if(!m) {
      break;
    }
    if(!m->peer) {
      // bad message
      continue;
    }
    SharedHandle<BtRegistry> reg = e_->getBtRegistry();
    SharedHandle<DownloadContext> dctx = reg->getDownloadContext(m->infoHash);
    if(!dctx) {
      A2_LOG_DEBUG(fmt("Download Context is null for infohash=%s.",
                       util::toHex(m->infoHash).c_str()));
      continue;
    }
    if(bittorrent::getTorrentAttrs(dctx)->privateTorrent) {
      A2_LOG_DEBUG("Ignore LPD message because the torrent is private.");
      continue;
    }
    RequestGroup* group = dctx->getOwnerRequestGroup();
    assert(group);
    const SharedHandle<BtObject>& btobj = reg->get(group->getGID());
    assert(btobj);
    const SharedHandle<PeerStorage>& peerStorage = btobj->peerStorage;
    assert(peerStorage);
    SharedHandle<Peer> peer = m->peer;
    if(peerStorage->addPeer(peer)) {
      A2_LOG_DEBUG(fmt("LPD peer %s:%u local=%d added.",
                       peer->getIPAddress().c_str(), peer->getPort(),
                       peer->isLocalPeer()?1:0));
    } else {
      A2_LOG_DEBUG(fmt("LPD peer %s:%u local=%d not added.",
                       peer->getIPAddress().c_str(), peer->getPort(),
                       peer->isLocalPeer()?1:0));
    }
  }
  e_->addCommand(this);
  return false;
}

} // namespace aria2
