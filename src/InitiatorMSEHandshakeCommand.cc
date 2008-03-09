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
#include "InitiatorMSEHandshakeCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "PeerInteractionCommand.h"
#include "DownloadEngine.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "CUIDCounter.h"
#include "Socket.h"
#include "Logger.h"
#include "Peer.h"
#include "PeerConnection.h"
#include "BtContext.h"
#include "BtRuntime.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "BtAnnounce.h"
#include "BtProgressInfoFile.h"
#include "Option.h"
#include "MSEHandshake.h"
#include "ARC4Encryptor.h"
#include "ARC4Decryptor.h"

namespace aria2 {

InitiatorMSEHandshakeCommand::InitiatorMSEHandshakeCommand
(int32_t cuid,
 RequestGroup* requestGroup,
 const SharedHandle<Peer>& p,
 DownloadEngine* e,
 const SharedHandle<BtContext>& btContext,
 const SharedHandle<SocketCore>& s):

  PeerAbstractCommand(cuid, p, e, s),
  BtContextAwareCommand(btContext),
  RequestGroupAware(requestGroup),
  _sequence(INITIATOR_SEND_KEY),
  _mseHandshake(new MSEHandshake(cuid, socket, e->option))
{
  disableReadCheckSocket();
  setWriteCheckSocket(socket);
  setTimeout(e->option->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));

  btRuntime->increaseConnections();
}

InitiatorMSEHandshakeCommand::~InitiatorMSEHandshakeCommand()
{
  btRuntime->decreaseConnections();
  
  delete _mseHandshake;
}

bool InitiatorMSEHandshakeCommand::executeInternal() {
  switch(_sequence) {
  case INITIATOR_SEND_KEY: {
    if(!socket->isWritable(0)) {
      break;
    }
    disableWriteCheckSocket();
    setReadCheckSocket(socket);
    socket->setBlockingMode();
    setTimeout(e->option->getAsInt(PREF_BT_TIMEOUT));
    _mseHandshake->initEncryptionFacility(true);
    _mseHandshake->sendPublicKey();
    _sequence = INITIATOR_WAIT_KEY;    
    break;
  }
  case INITIATOR_WAIT_KEY: {
    if(_mseHandshake->receivePublicKey()) {
      _mseHandshake->initCipher(btContext->getInfoHash());
      _mseHandshake->sendInitiatorStep2();
      _sequence = INITIATOR_FIND_VC_MARKER;
    }
    break;
  }
  case INITIATOR_FIND_VC_MARKER: {
    if(_mseHandshake->findInitiatorVCMarker()) {
      _sequence = INITIATOR_RECEIVE_PAD_D_LENGTH;
    }
    break;
  }
  case INITIATOR_RECEIVE_PAD_D_LENGTH: {
    if(_mseHandshake->receiveInitiatorCryptoSelectAndPadDLength()) {
      _sequence = INITIATOR_RECEIVE_PAD_D;
    }
    break;
  }
  case INITIATOR_RECEIVE_PAD_D: {
    if(_mseHandshake->receivePad()) {
      SharedHandle<PeerConnection> peerConnection =
	new PeerConnection(cuid, socket, e->option);
      if(_mseHandshake->getNegotiatedCryptoType() == MSEHandshake::CRYPTO_ARC4) {
	peerConnection->enableEncryption(_mseHandshake->getEncryptor(),
					 _mseHandshake->getDecryptor());
      }
      Command* c =
	  new PeerInteractionCommand(cuid, _requestGroup, peer, e, btContext,
				     socket,
				     PeerInteractionCommand::INITIATOR_SEND_HANDSHAKE,
				     peerConnection);
      e->commands.push_back(c);
      return true;
    }
    break;
  }
  }
  e->commands.push_back(this);
  return false;
}

bool InitiatorMSEHandshakeCommand::prepareForNextPeer(time_t wait)
{
  if(e->option->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    logger->info("CUID#%d - Establishing connection using legacy BitTorrent handshake is disabled by preference.", cuid);
    if(peerStorage->isPeerAvailable() && btRuntime->lessThanEqMinPeer()) {
      SharedHandle<Peer> peer = peerStorage->getUnusedPeer();
      peer->usedBy(CUIDCounterSingletonHolder::instance()->newID());
      Command* command =
	new PeerInitiateConnectionCommand(peer->usedBy(), _requestGroup, peer, e,
					  btContext);
      e->commands.push_back(command);
    }
    return true;
  } else {
    // try legacy BitTorrent handshake
    logger->info("CUID#%d - Retry using legacy BitTorrent handshake.", cuid);
    Command* command =
      new PeerInitiateConnectionCommand(cuid, _requestGroup, peer, e, btContext,
					false);
    e->commands.push_back(command);
    return true;
  }
}

void InitiatorMSEHandshakeCommand::onAbort(Exception* ex)
{
  if(e->option->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    peerStorage->returnPeer(peer);
  }
}

bool InitiatorMSEHandshakeCommand::exitBeforeExecute()
{
  return btRuntime->isHalt();
}

} // namespace aria2
