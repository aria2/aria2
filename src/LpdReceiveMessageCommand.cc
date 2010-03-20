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

namespace aria2 {

unsigned int LpdReceiveMessageCommand::__numInstance = 0;

LpdReceiveMessageCommand* LpdReceiveMessageCommand::__instance = 0;

LpdReceiveMessageCommand::LpdReceiveMessageCommand
(cuid_t cuid, const SharedHandle<LpdMessageReceiver>& receiver,
 DownloadEngine* e):Command(cuid), _receiver(receiver), _e(e)
{
  _e->addSocketForReadCheck(_receiver->getSocket(), this);
  ++__numInstance;
}

LpdReceiveMessageCommand::~LpdReceiveMessageCommand()
{
  _e->deleteSocketForReadCheck(_receiver->getSocket(), this);
  --__numInstance;
  if(__numInstance == 0) {
    __instance = 0;
  }
}

bool LpdReceiveMessageCommand::execute()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    return true;
  }
  for(size_t i = 0; i < 20; ++i) {
    SharedHandle<LpdMessage> m = _receiver->receiveMessage();
    if(m.isNull()) {
      break;
    }
    if(m->getPeer().isNull()) {
      // bad message
      continue;
    }
    SharedHandle<BtRegistry> reg = _e->getBtRegistry();
    SharedHandle<DownloadContext> dctx =
      reg->getDownloadContext(m->getInfoHash());
    if(dctx.isNull()) {
      if(logger->debug()) {
        logger->debug("Download Context is null for infohash=%s.",
                      util::toHex(m->getInfoHash()).c_str());
      }
      continue;
    }
    const BDE& torrentAttrs = dctx->getAttribute(bittorrent::BITTORRENT);
    if(torrentAttrs.containsKey(bittorrent::PRIVATE)) {
      if(torrentAttrs[bittorrent::PRIVATE].i() == 1) {
        if(logger->debug()) {
          logger->debug("Ignore LPD message because the torrent is private.");
        }
        continue;
      }
    }
    RequestGroup* group = dctx->getOwnerRequestGroup();
    assert(group);
    BtObject btobj = reg->get(group->getGID());
    assert(!btobj.isNull());
    SharedHandle<PeerStorage> peerStorage = btobj._peerStorage;
    assert(!peerStorage.isNull());
    SharedHandle<Peer> peer = m->getPeer();
    if(peerStorage->addPeer(peer)) {
      if(logger->debug()) {
        logger->debug("LPD peer %s:%u local=%d added.",
                      peer->ipaddr.c_str(), peer->port,
                      peer->isLocalPeer()?1:0);
      }
    } else {
      if(logger->debug()) {
        logger->debug("LPD peer %s:%u local=%d not added.",
                      peer->ipaddr.c_str(), peer->port,
                      peer->isLocalPeer()?1:0);
      }
    }
  }
  _e->commands.push_back(this);
  return false;
}

LpdReceiveMessageCommand*
LpdReceiveMessageCommand::getInstance
(DownloadEngine* e, const SharedHandle<LpdMessageReceiver>& receiver)
{
  if(__numInstance == 0) {
    __instance = new LpdReceiveMessageCommand(e->newCUID(), receiver, e);
  }
  return __instance;
}

LpdReceiveMessageCommand* LpdReceiveMessageCommand::getInstance()
{
  if(__numInstance == 0) {
    return 0;
  } else {
    return __instance;
  }
}

} // namespace aria2
