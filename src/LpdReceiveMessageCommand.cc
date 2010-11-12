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
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "BtProgressInfoFile.h"
#include "BtAnnounce.h"
#include "LpdMessage.h"
#include "bittorrent_helper.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "util.h"

namespace aria2 {

unsigned int LpdReceiveMessageCommand::numInstance_ = 0;

LpdReceiveMessageCommand* LpdReceiveMessageCommand::instance_ = 0;

LpdReceiveMessageCommand::LpdReceiveMessageCommand
(cuid_t cuid, const SharedHandle<LpdMessageReceiver>& receiver,
 DownloadEngine* e):Command(cuid), receiver_(receiver), e_(e)
{
  e_->addSocketForReadCheck(receiver_->getSocket(), this);
  ++numInstance_;
}

LpdReceiveMessageCommand::~LpdReceiveMessageCommand()
{
  e_->deleteSocketForReadCheck(receiver_->getSocket(), this);
  --numInstance_;
  if(numInstance_ == 0) {
    instance_ = 0;
  }
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
    if(!m->getPeer()) {
      // bad message
      continue;
    }
    SharedHandle<BtRegistry> reg = e_->getBtRegistry();
    SharedHandle<DownloadContext> dctx =
      reg->getDownloadContext(m->getInfoHash());
    if(!dctx) {
      if(getLogger()->debug()) {
        getLogger()->debug("Download Context is null for infohash=%s.",
                           util::toHex(m->getInfoHash()).c_str());
      }
      continue;
    }
    if(bittorrent::getTorrentAttrs(dctx)->privateTorrent) {
      if(getLogger()->debug()) {
        getLogger()->debug
          ("Ignore LPD message because the torrent is private.");
      }
      continue;
    }
    RequestGroup* group = dctx->getOwnerRequestGroup();
    assert(group);
    BtObject btobj = reg->get(group->getGID());
    assert(!btobj.isNull());
    SharedHandle<PeerStorage> peerStorage = btobj.peerStorage_;
    assert(peerStorage);
    SharedHandle<Peer> peer = m->getPeer();
    if(peerStorage->addPeer(peer)) {
      if(getLogger()->debug()) {
        getLogger()->debug("LPD peer %s:%u local=%d added.",
                           peer->getIPAddress().c_str(), peer->getPort(),
                           peer->isLocalPeer()?1:0);
      }
    } else {
      if(getLogger()->debug()) {
        getLogger()->debug("LPD peer %s:%u local=%d not added.",
                           peer->getIPAddress().c_str(), peer->getPort(),
                           peer->isLocalPeer()?1:0);
      }
    }
  }
  e_->addCommand(this);
  return false;
}

LpdReceiveMessageCommand*
LpdReceiveMessageCommand::getInstance
(DownloadEngine* e, const SharedHandle<LpdMessageReceiver>& receiver)
{
  if(numInstance_ == 0) {
    instance_ = new LpdReceiveMessageCommand(e->newCUID(), receiver, e);
  }
  return instance_;
}

LpdReceiveMessageCommand* LpdReceiveMessageCommand::getInstance()
{
  if(numInstance_ == 0) {
    return 0;
  } else {
    return instance_;
  }
}

} // namespace aria2
