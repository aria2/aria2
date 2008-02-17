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
#include "DownloadEngine.h"
#include "PeerInitiateConnectionCommand.h"
#include "DefaultBtInteractive.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "Socket.h"
#include "Option.h"
#include "BtContext.h"
#include "BtRegistry.h"
#include "PeerObject.h"
#include "Peer.h"
#include "BtMessage.h"
#include "BtRuntime.h"
#include "PeerStorage.h"
#include "DefaultBtMessageDispatcher.h"
#include "DefaultBtMessageReceiver.h"
#include "DefaultBtRequestFactory.h"
#include "DefaultBtMessageFactory.h"
#include "DefaultBtInteractive.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"
#include "CUIDCounter.h"
#include "DHTRoutingTable.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTNode.h"
#include "DHTSetup.h"
#include "DHTRegistry.h"
#include <algorithm>

namespace aria2 {

PeerInteractionCommand::PeerInteractionCommand(int32_t cuid,
					       RequestGroup* requestGroup,
					       const PeerHandle& p,
					       DownloadEngine* e,
					       const BtContextHandle& btContext,
					       const SocketHandle& s,
					       Seq sequence,
					       const PeerConnectionHandle& passedPeerConnection)
  :PeerAbstractCommand(cuid, p, e, s),
   BtContextAwareCommand(btContext),
   RequestGroupAware(requestGroup),
   sequence(sequence),
   btInteractive(0),
   maxDownloadSpeedLimit(0)
{
  // TODO move following bunch of processing to separate method, like init()
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    disableReadCheckSocket();
    setWriteCheckSocket(socket);
    setTimeout(e->option->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));
  }
  DefaultBtMessageFactoryHandle factory = new DefaultBtMessageFactory();
  factory->setCuid(cuid);
  factory->setBtContext(btContext);
  factory->setPeer(peer);
  factory->setLocalNode(DHTRegistry::_localNode);
  factory->setRoutingTable(DHTRegistry::_routingTable);
  factory->setTaskQueue(DHTRegistry::_taskQueue);
  factory->setTaskFactory(DHTRegistry::_taskFactory);

  PeerConnectionHandle peerConnection = passedPeerConnection.isNull() ?
    new PeerConnection(cuid, socket, e->option) : passedPeerConnection;

  DefaultBtMessageDispatcherHandle dispatcher = new DefaultBtMessageDispatcher();
  dispatcher->setCuid(cuid);
  dispatcher->setPeer(peer);
  dispatcher->setBtContext(btContext);
  dispatcher->setMaxUploadSpeedLimit(e->option->getAsInt(PREF_MAX_UPLOAD_LIMIT));
  dispatcher->setRequestTimeout(e->option->getAsInt(PREF_BT_REQUEST_TIMEOUT));
  dispatcher->setBtMessageFactory(factory);

  DefaultBtMessageReceiverHandle receiver = new DefaultBtMessageReceiver();
  receiver->setCuid(cuid);
  receiver->setPeer(peer);
  receiver->setBtContext(btContext);
  receiver->setPeerConnection(peerConnection);
  receiver->setDispatcher(dispatcher);
  receiver->setBtMessageFactory(factory);

  DefaultBtRequestFactoryHandle reqFactory = new DefaultBtRequestFactory();
  reqFactory->setCuid(cuid);
  reqFactory->setPeer(peer);
  reqFactory->setBtContext(btContext);
  reqFactory->setBtMessageDispatcher(dispatcher);
  reqFactory->setBtMessageFactory(factory);

  DefaultBtInteractiveHandle btInteractive = new DefaultBtInteractive(btContext, peer);
  btInteractive->setCuid(cuid);
  btInteractive->setBtMessageReceiver(receiver);
  btInteractive->setDispatcher(dispatcher);
  btInteractive->setBtRequestFactory(reqFactory);
  btInteractive->setPeerConnection(peerConnection);
  btInteractive->setKeepAliveInterval(e->option->getAsInt(PREF_BT_KEEP_ALIVE_INTERVAL));
  btInteractive->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  btInteractive->setBtMessageFactory(factory);
  if(!btContext->isPrivate()) {
    if(e->option->getAsBool(PREF_ENABLE_PEER_EXCHANGE)) {
      btInteractive->setUTPexEnabled(true);
    }
    if(DHTSetup::initialized()) {
      btInteractive->setDHTEnabled(true);
      btInteractive->setLocalNode(DHTRegistry::_localNode);
      factory->setDHTEnabled(true);
    }
  }
  this->btInteractive = btInteractive;

  // reverse depends
  factory->setBtMessageDispatcher(dispatcher);
  factory->setBtRequestFactory(reqFactory);
  factory->setPeerConnection(peerConnection);

  PeerObjectHandle peerObject = new PeerObject();
  peerObject->btMessageDispatcher = dispatcher;
  peerObject->btMessageReceiver = receiver;
  peerObject->btMessageFactory = factory;
  peerObject->btRequestFactory = reqFactory;
  peerObject->peerConnection = peerConnection;
  
  PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getID(), peerObject);

  setUploadLimit(e->option->getAsInt(PREF_MAX_UPLOAD_LIMIT));
  peer->allocateSessionResource(btContext->getPieceLength(), btContext->getTotalLength());

  maxDownloadSpeedLimit = e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT);

  btRuntime->increaseConnections();
}

PeerInteractionCommand::~PeerInteractionCommand() {
  peer->releaseSessionResource();
  PEER_OBJECT_CLUSTER(btContext)->unregisterHandle(peer->getID());
					
  btRuntime->decreaseConnections();
  //logger->debug("CUID#%d - unregistered message factory using ID:%s",
  //cuid, peer->getId().c_str());
}

bool PeerInteractionCommand::executeInternal() {
  setUploadLimitCheck(false);
  setNoCheck(false);
  switch(sequence) {
  case INITIATOR_SEND_HANDSHAKE:
    if(!socket->isWritable(0)) {
      break;
    }
    disableWriteCheckSocket();
    setReadCheckSocket(socket);
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
    if(btInteractive->countReceivedMessageInIteration() > 0) {
      updateKeepAlive();
    }
    if(peer->amInterested() && !peer->peerChoking() ||
       peer->peerInterested() && !peer->amChoking()) {
      if(maxDownloadSpeedLimit > 0) {
	TransferStat stat = peerStorage->calculateStat();
	if(maxDownloadSpeedLimit < stat.downloadSpeed) {
	  disableReadCheckSocket();
	  setNoCheck(true);
	} else {
	  setReadCheckSocket(socket);
	}
      } else {
	setReadCheckSocket(socket);
      }
    } else {
      disableReadCheckSocket();
    }
    break;
  }
  if(btInteractive->countPendingMessage() > 0) {
    setNoCheck(true);
  }
  e->commands.push_back(this);
  return false;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(int32_t wait) {
  if(peerStorage->isPeerAvailable() && btRuntime->lessThanEqMinPeer()) {
    PeerHandle peer = peerStorage->getUnusedPeer();
    peer->usedBy(CUIDCounterSingletonHolder::instance()->newID());
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand(peer->usedBy(),
					_requestGroup,
					peer,
					e,
					btContext);
    e->commands.push_back(command);
  }
  return true;
}

void PeerInteractionCommand::onAbort(Exception* ex) {
  btInteractive->cancelAllPiece();
  peerStorage->returnPeer(peer);
  //PeerAbstractCommand::onAbort(ex);
}

bool PeerInteractionCommand::exitBeforeExecute()
{
  return btRuntime->isHalt();
}

} // namespace aria2
