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
#include "PeerInitiateConnectionCommand.h"
#include "InitiatorMSEHandshakeCommand.h"
#include "PeerInteractionCommand.h"
#include "DownloadEngine.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "SocketCore.h"
#include "Logger.h"
#include "LogFactory.h"
#include "Peer.h"
#include "BtRuntime.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "PeerConnection.h"
#include "RequestGroup.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

PeerInitiateConnectionCommand::PeerInitiateConnectionCommand(
    cuid_t cuid, RequestGroup* requestGroup, const std::shared_ptr<Peer>& peer,
    DownloadEngine* e, const std::shared_ptr<BtRuntime>& btRuntime,
    bool mseHandshakeEnabled)
    : PeerAbstractCommand(cuid, peer, e),
      requestGroup_(requestGroup),
      btRuntime_(btRuntime),
      mseHandshakeEnabled_(mseHandshakeEnabled)
{
  btRuntime_->increaseConnections();
  requestGroup_->increaseNumCommand();
}

PeerInitiateConnectionCommand::~PeerInitiateConnectionCommand()
{
  requestGroup_->decreaseNumCommand();
  btRuntime_->decreaseConnections();
}

bool PeerInitiateConnectionCommand::executeInternal()
{
  A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER, getCuid(),
                  getPeer()->getIPAddress().c_str(), getPeer()->getPort()));
  createSocket();
  getSocket()->establishConnection(getPeer()->getIPAddress(),
                                   getPeer()->getPort(), false);
  getSocket()->applyIpDscp();
  if (mseHandshakeEnabled_) {
    auto c = make_unique<InitiatorMSEHandshakeCommand>(
        getCuid(), requestGroup_, getPeer(), getDownloadEngine(), btRuntime_,
        getSocket());
    c->setPeerStorage(peerStorage_);
    c->setPieceStorage(pieceStorage_);
    getDownloadEngine()->addCommand(std::move(c));
  }
  else {
    getDownloadEngine()->addCommand(make_unique<PeerInteractionCommand>(
        getCuid(), requestGroup_, getPeer(), getDownloadEngine(), btRuntime_,
        pieceStorage_, peerStorage_, getSocket(),
        PeerInteractionCommand::INITIATOR_SEND_HANDSHAKE));
  }
  return true;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInitiateConnectionCommand::prepareForNextPeer(time_t wait)
{
  if (peerStorage_->isPeerAvailable() && btRuntime_->lessThanEqMinPeers()) {
    cuid_t ncuid = getDownloadEngine()->newCUID();
    std::shared_ptr<Peer> peer = peerStorage_->checkoutPeer(ncuid);
    // sanity check
    if (peer) {
      auto command = make_unique<PeerInitiateConnectionCommand>(
          ncuid, requestGroup_, peer, getDownloadEngine(), btRuntime_);
      command->setPeerStorage(peerStorage_);
      command->setPieceStorage(pieceStorage_);
      getDownloadEngine()->addCommand(std::move(command));
    }
  }
  return true;
}

void PeerInitiateConnectionCommand::onAbort()
{
  peerStorage_->returnPeer(getPeer());
}

bool PeerInitiateConnectionCommand::exitBeforeExecute()
{
  return btRuntime_->isHalt();
}

void PeerInitiateConnectionCommand::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void PeerInitiateConnectionCommand::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

} // namespace aria2
