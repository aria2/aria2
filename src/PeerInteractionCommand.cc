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
#include "PeerInteractionCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "PeerMessageUtil.h"
#include "DefaultBtInteractive.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "message.h"
#include "prefs.h"
#include "DefaultBtMessageDispatcher.h"
#include "DefaultBtMessageReceiver.h"
#include "DefaultBtRequestFactory.h"
#include "DefaultBtMessageFactory.h"
#include "DefaultBtInteractive.h"
#include "PeerConnection.h"
#include <algorithm>

PeerInteractionCommand::PeerInteractionCommand(int cuid,
					       const PeerHandle& p,
					       TorrentDownloadEngine* e,
					       const BtContextHandle& btContext,
					       const SocketHandle& s,
					       int sequence)
  :PeerAbstractCommand(cuid, p, e, btContext, s),
   sequence(sequence),
   btInteractive(0)
{
  // TODO move following bunch of processing to separate method, like init()
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    disableReadCheckSocket();
    setWriteCheckSocket(socket);
    setTimeout(e->option->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));
  }
  PeerConnectionHandle peerConnection =
    new PeerConnection(cuid, socket, e->option);

  DefaultBtMessageDispatcherHandle dispatcher = new DefaultBtMessageDispatcher();
  dispatcher->setCuid(cuid);
  dispatcher->setPeer(peer);
  dispatcher->setBtContext(btContext);
  dispatcher->setOption(e->option);

  DefaultBtMessageReceiverHandle receiver = new DefaultBtMessageReceiver();
  receiver->setCuid(cuid);
  receiver->setPeer(peer);
  receiver->setBtContext(btContext);
  receiver->setPeerConnection(peerConnection);
  receiver->setDispatcher(dispatcher);

  DefaultBtRequestFactoryHandle reqFactory = new DefaultBtRequestFactory();
  reqFactory->setCuid(cuid);
  reqFactory->setPeer(peer);
  reqFactory->setBtContext(btContext);
  reqFactory->setBtMessageDispatcher(dispatcher);

  DefaultBtInteractiveHandle btInteractive = new DefaultBtInteractive();
  btInteractive->setCuid(cuid);
  btInteractive->setPeer(peer);
  btInteractive->setBtContext(btContext);
  btInteractive->setBtMessageReceiver(receiver);
  btInteractive->setDispatcher(dispatcher);
  btInteractive->setBtRequestFactory(reqFactory);
  btInteractive->setPeerConnection(peerConnection);
  btInteractive->setOption(e->option);
  this->btInteractive = btInteractive;

  DefaultBtMessageFactoryHandle factory = new DefaultBtMessageFactory();
  factory->setCuid(cuid);
  factory->setBtContext(btContext);
  factory->setPeer(peer);

  PeerObjectHandle peerObject = new PeerObject();
  peerObject->btMessageDispatcher = dispatcher;
  peerObject->btMessageFactory = factory;
  peerObject->btRequestFactory = reqFactory;
  peerObject->peerConnection = peerConnection;

  PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), peerObject);

  setUploadLimit(e->option->getAsInt(PREF_MAX_UPLOAD_LIMIT));
  peer->activate();
}

PeerInteractionCommand::~PeerInteractionCommand() {
  peer->deactivate();
  PEER_OBJECT_CLUSTER(btContext)->unregisterHandle(peer->getId());
							  
  logger->debug("CUID#%d - unregistered message factory using ID:%s",
		cuid, peer->getId().c_str());
}

bool PeerInteractionCommand::executeInternal() {
  setReadCheckSocket(socket);
  disableWriteCheckSocket();
  setUploadLimitCheck(false);
  setNoCheck(false);
  switch(sequence) {
  case INITIATOR_SEND_HANDSHAKE:
    if(!socket->isWritable(0)) {
      disableReadCheckSocket();
      setWriteCheckSocket(socket);
      break;
    }
    socket->setBlockingMode();
    setTimeout(e->option->getAsInt(PREF_BT_TIMEOUT));
    btInteractive->initiateHandshake();
    sequence = INITIATOR_WAIT_HANDSHAKE;
    break;
  case INITIATOR_WAIT_HANDSHAKE: {
    if(btInteractive->countPendingMessage() > 0) {
      btInteractive->sendPendingMessage();
      if(btInteractive->countPendingMessage() > 0) {
	break;
      }
    }
    BtMessageHandle handshakeMessage = btInteractive->receiveHandshake();
    if(handshakeMessage.isNull()) {
      break;
    }
    btInteractive->doPostHandshakeProcessing();
    sequence = WIRED;
    break;
  }
  case RECEIVER_WAIT_HANDSHAKE: {
    BtMessageHandle handshakeMessage = btInteractive->receiveAndSendHandshake();
    if(handshakeMessage.isNull()) {
      break;
    }
    btInteractive->doPostHandshakeProcessing();
    sequence = WIRED;    
    break;
  }
  case WIRED:
    btInteractive->doInteractionProcessing();

    break;
  }
  if(btInteractive->countPendingMessage() > 0) {
    setNoCheck(true);
  }
  int maxSpeedLimit = e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT);
  if(maxSpeedLimit > 0) {
    TransferStat stat = peerStorage->calculateStat();
    if(maxSpeedLimit < stat.downloadSpeed) {
      disableReadCheckSocket();
      setNoCheck(true);
    }
  }
  e->commands.push_back(this);
  return false;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(int wait) {
  if(peerStorage->isPeerAvailable() && btRuntime->lessThanEqMinPeer()) {
    PeerHandle peer = peerStorage->getUnusedPeer();
    int newCuid = btRuntime->getNewCuid();
    peer->cuid = newCuid;
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand(newCuid,
					peer,
					e,
					btContext);
    e->commands.push_back(command);
  }
  return true;
}

bool PeerInteractionCommand::prepareForRetry(int wait) {
  e->commands.push_back(this);
  return false;
}

void PeerInteractionCommand::onAbort(RecoverableException* ex) {
  btInteractive->cancelAllPiece();
  PeerAbstractCommand::onAbort(ex);
}
