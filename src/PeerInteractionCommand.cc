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
#include "PeerInteractionCommand.h"

#include <algorithm>

#include "DownloadEngine.h"
#include "PeerInitiateConnectionCommand.h"
#include "DefaultBtInteractive.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "Socket.h"
#include "Option.h"
#include "DownloadContext.h"
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
#include "DHTRoutingTable.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTNode.h"
#include "DHTSetup.h"
#include "DHTRegistry.h"
#include "PieceStorage.h"
#include "RequestGroup.h"
#include "BtAnnounce.h"
#include "BtProgressInfoFile.h"
#include "DefaultExtensionMessageFactory.h"
#include "RequestGroupMan.h"
#include "ExtensionMessageRegistry.h"
#include "bittorrent_helper.h"
#include "UTMetadataRequestFactory.h"
#include "UTMetadataRequestTracker.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"

namespace aria2 {

PeerInteractionCommand::PeerInteractionCommand
(cuid_t cuid,
 RequestGroup* requestGroup,
 const SharedHandle<Peer>& p,
 DownloadEngine* e,
 const SharedHandle<BtRuntime>& btRuntime,
 const SharedHandle<PieceStorage>& pieceStorage,
 const SharedHandle<PeerStorage>& peerStorage,
 const SocketHandle& s,
 Seq sequence,
 const PeerConnectionHandle& passedPeerConnection)
  :PeerAbstractCommand(cuid, p, e, s),
   _requestGroup(requestGroup),
   _btRuntime(btRuntime),
   _pieceStorage(pieceStorage),
   _peerStorage(peerStorage),
   sequence(sequence)
{
  // TODO move following bunch of processing to separate method, like init()
  if(sequence == INITIATOR_SEND_HANDSHAKE) {
    disableReadCheckSocket();
    setWriteCheckSocket(socket);
    setTimeout(getOption()->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));
  }

  const BDE& torrentAttrs =
    _requestGroup->getDownloadContext()->getAttribute(bittorrent::BITTORRENT);

  bool metadataGetMode = !torrentAttrs.containsKey(bittorrent::METADATA);

  SharedHandle<ExtensionMessageRegistry> exMsgRegistry
    (new ExtensionMessageRegistry());

  SharedHandle<UTMetadataRequestFactory> utMetadataRequestFactory;
  SharedHandle<UTMetadataRequestTracker> utMetadataRequestTracker;
  if(metadataGetMode) {
    utMetadataRequestFactory.reset(new UTMetadataRequestFactory());
    utMetadataRequestTracker.reset(new UTMetadataRequestTracker());
  }

  SharedHandle<DefaultExtensionMessageFactory> extensionMessageFactory
    (new DefaultExtensionMessageFactory(peer, exMsgRegistry));
  extensionMessageFactory->setPeerStorage(peerStorage);
  extensionMessageFactory->setDownloadContext
    (_requestGroup->getDownloadContext());
  extensionMessageFactory->setUTMetadataRequestTracker
    (utMetadataRequestTracker);
  // PieceStorage will be set later.

  SharedHandle<DefaultBtMessageFactory> factory(new DefaultBtMessageFactory());
  factory->setCuid(cuid);
  factory->setDownloadContext(_requestGroup->getDownloadContext());
  factory->setPieceStorage(pieceStorage);
  factory->setPeerStorage(peerStorage);
  factory->setExtensionMessageFactory(extensionMessageFactory);
  factory->setPeer(peer);
  factory->setLocalNode(DHTRegistry::_localNode);
  factory->setRoutingTable(DHTRegistry::_routingTable);
  factory->setTaskQueue(DHTRegistry::_taskQueue);
  factory->setTaskFactory(DHTRegistry::_taskFactory);
  if(metadataGetMode) {
    factory->enableMetadataGetMode();
  }

  PeerConnectionHandle peerConnection;
  if(passedPeerConnection.isNull()) {
    peerConnection.reset(new PeerConnection(cuid, socket));
  } else {
    peerConnection = passedPeerConnection;
  }

  SharedHandle<DefaultBtMessageDispatcher> dispatcher
    (new DefaultBtMessageDispatcher());
  dispatcher->setCuid(cuid);
  dispatcher->setPeer(peer);
  dispatcher->setDownloadContext(_requestGroup->getDownloadContext());
  dispatcher->setPieceStorage(pieceStorage);
  dispatcher->setPeerStorage(peerStorage);
  dispatcher->setRequestTimeout(getOption()->getAsInt(PREF_BT_REQUEST_TIMEOUT));
  dispatcher->setBtMessageFactory(factory);
  dispatcher->setRequestGroupMan(e->getRequestGroupMan());

  DefaultBtMessageReceiverHandle receiver(new DefaultBtMessageReceiver());
  receiver->setCuid(cuid);
  receiver->setPeer(peer);
  receiver->setDownloadContext(_requestGroup->getDownloadContext());
  receiver->setPeerConnection(peerConnection);
  receiver->setDispatcher(dispatcher);
  receiver->setBtMessageFactory(factory);

  SharedHandle<DefaultBtRequestFactory> reqFactory
    (new DefaultBtRequestFactory());
  reqFactory->setCuid(cuid);
  reqFactory->setPeer(peer);
  reqFactory->setPieceStorage(pieceStorage);
  reqFactory->setBtMessageDispatcher(dispatcher);
  reqFactory->setBtMessageFactory(factory);

  DefaultBtInteractiveHandle btInteractive
    (new DefaultBtInteractive(_requestGroup->getDownloadContext(), peer));
  btInteractive->setBtRuntime(_btRuntime);
  btInteractive->setPieceStorage(_pieceStorage);
  btInteractive->setPeerStorage(peerStorage); // Note: Not a member variable.
  btInteractive->setCuid(cuid);
  btInteractive->setBtMessageReceiver(receiver);
  btInteractive->setDispatcher(dispatcher);
  btInteractive->setBtRequestFactory(reqFactory);
  btInteractive->setPeerConnection(peerConnection);
  btInteractive->setExtensionMessageFactory(extensionMessageFactory);
  btInteractive->setExtensionMessageRegistry(exMsgRegistry);
  btInteractive->setKeepAliveInterval
    (getOption()->getAsInt(PREF_BT_KEEP_ALIVE_INTERVAL));
  btInteractive->setRequestGroupMan(e->getRequestGroupMan());
  btInteractive->setBtMessageFactory(factory);
  if((metadataGetMode || torrentAttrs[bittorrent::PRIVATE].i() == 0) &&
     !peer->isLocalPeer()) {
    if(getOption()->getAsBool(PREF_ENABLE_PEER_EXCHANGE)) {
      btInteractive->setUTPexEnabled(true);
    }
    if(DHTSetup::initialized()) {
      btInteractive->setDHTEnabled(true);
      btInteractive->setLocalNode(DHTRegistry::_localNode);
      factory->setDHTEnabled(true);
    }
  }
  btInteractive->setUTMetadataRequestFactory(utMetadataRequestFactory);
  btInteractive->setUTMetadataRequestTracker(utMetadataRequestTracker);
  if(metadataGetMode) {
    btInteractive->enableMetadataGetMode();
  }

  this->btInteractive = btInteractive;

  // reverse depends
  factory->setBtMessageDispatcher(dispatcher);
  factory->setBtRequestFactory(reqFactory);
  factory->setPeerConnection(peerConnection);

  extensionMessageFactory->setBtMessageDispatcher(dispatcher);
  extensionMessageFactory->setBtMessageFactory(factory);

  if(metadataGetMode) {
    utMetadataRequestFactory->setDownloadContext
      (_requestGroup->getDownloadContext());
    utMetadataRequestFactory->setBtMessageDispatcher(dispatcher);
    utMetadataRequestFactory->setBtMessageFactory(factory);
    utMetadataRequestFactory->setPeer(peer);
    utMetadataRequestFactory->setUTMetadataRequestTracker
      (utMetadataRequestTracker);
  }

  peer->allocateSessionResource
    (_requestGroup->getDownloadContext()->getPieceLength(),
     _requestGroup->getDownloadContext()->getTotalLength());
  peer->setBtMessageDispatcher(dispatcher);

  _btRuntime->increaseConnections();
  _requestGroup->increaseNumCommand();
}

PeerInteractionCommand::~PeerInteractionCommand() {
  if(peer->getCompletedLength() > 0) {
    _pieceStorage->subtractPieceStats(peer->getBitfield(),
                                      peer->getBitfieldLength());
  }
  peer->releaseSessionResource();

  _requestGroup->decreaseNumCommand();
  _btRuntime->decreaseConnections();
}

bool PeerInteractionCommand::executeInternal() {
  setNoCheck(false);
  switch(sequence) {
  case INITIATOR_SEND_HANDSHAKE:
    if(!socket->isWritable(0)) {
      break;
    }
    disableWriteCheckSocket();
    setReadCheckSocket(socket);
    //socket->setBlockingMode();
    setTimeout(getOption()->getAsInt(PREF_BT_TIMEOUT));
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
    // See the comment for writable check below.
    disableWriteCheckSocket();

    btInteractive->doInteractionProcessing();
    if(btInteractive->countReceivedMessageInIteration() > 0) {
      updateKeepAlive();
    }
    if((peer->amInterested() && !peer->peerChoking()) ||
       btInteractive->countOutstandingRequest() ||
       (peer->peerInterested() && !peer->amChoking())) {

      // Writable check to avoid slow seeding
      if(btInteractive->isSendingMessageInProgress()) {
        setWriteCheckSocket(socket);
      }

      if(e->getRequestGroupMan()->doesOverallDownloadSpeedExceed() ||
         _requestGroup->doesDownloadSpeedExceed()) {
        disableReadCheckSocket();
        setNoCheck(true);
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
  e->addCommand(this);
  return false;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(time_t wait) {
  if(_peerStorage->isPeerAvailable() && _btRuntime->lessThanEqMinPeers()) {
    SharedHandle<Peer> peer = _peerStorage->getUnusedPeer();
    peer->usedBy(e->newCUID());
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand
      (peer->usedBy(), _requestGroup, peer, e, _btRuntime);
    command->setPeerStorage(_peerStorage);
    command->setPieceStorage(_pieceStorage);
    e->addCommand(command);
  }
  return true;
}

void PeerInteractionCommand::onAbort() {
  btInteractive->cancelAllPiece();
  _peerStorage->returnPeer(peer);
}

void PeerInteractionCommand::onFailure()
{
  _requestGroup->setHaltRequested(true);
}

bool PeerInteractionCommand::exitBeforeExecute()
{
  return _btRuntime->isHalt();
}

const SharedHandle<Option>& PeerInteractionCommand::getOption() const
{
  return _requestGroup->getOption();
}

} // namespace aria2
