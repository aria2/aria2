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
   _sequence(sequence)
{
  // TODO move following bunch of processing to separate method, like init()
  if(_sequence == INITIATOR_SEND_HANDSHAKE) {
    disableReadCheckSocket();
    setWriteCheckSocket(getSocket());
    setTimeout(getOption()->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));
  }
  SharedHandle<TorrentAttribute> torrentAttrs =
    bittorrent::getTorrentAttrs(_requestGroup->getDownloadContext());
  bool metadataGetMode = torrentAttrs->metadata.empty();

  SharedHandle<ExtensionMessageRegistry> exMsgRegistry
    (new ExtensionMessageRegistry());

  SharedHandle<UTMetadataRequestFactory> utMetadataRequestFactory;
  SharedHandle<UTMetadataRequestTracker> utMetadataRequestTracker;
  if(metadataGetMode) {
    utMetadataRequestFactory.reset(new UTMetadataRequestFactory());
    utMetadataRequestTracker.reset(new UTMetadataRequestTracker());
  }

  SharedHandle<DefaultExtensionMessageFactory> extensionMessageFactory
    (new DefaultExtensionMessageFactory(getPeer(), exMsgRegistry));
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
  factory->setPeer(getPeer());
  factory->setLocalNode(DHTRegistry::getData().localNode);
  factory->setRoutingTable(DHTRegistry::getData().routingTable);
  factory->setTaskQueue(DHTRegistry::getData().taskQueue);
  factory->setTaskFactory(DHTRegistry::getData().taskFactory);
  if(metadataGetMode) {
    factory->enableMetadataGetMode();
  }

  PeerConnectionHandle peerConnection;
  if(passedPeerConnection.isNull()) {
    peerConnection.reset(new PeerConnection(cuid, getSocket()));
  } else {
    peerConnection = passedPeerConnection;
  }

  SharedHandle<DefaultBtMessageDispatcher> dispatcher
    (new DefaultBtMessageDispatcher());
  dispatcher->setCuid(cuid);
  dispatcher->setPeer(getPeer());
  dispatcher->setDownloadContext(_requestGroup->getDownloadContext());
  dispatcher->setPieceStorage(pieceStorage);
  dispatcher->setPeerStorage(peerStorage);
  dispatcher->setRequestTimeout(getOption()->getAsInt(PREF_BT_REQUEST_TIMEOUT));
  dispatcher->setBtMessageFactory(factory);
  dispatcher->setRequestGroupMan(getDownloadEngine()->getRequestGroupMan());

  DefaultBtMessageReceiverHandle receiver(new DefaultBtMessageReceiver());
  receiver->setDownloadContext(_requestGroup->getDownloadContext());
  receiver->setPeerConnection(peerConnection);
  receiver->setDispatcher(dispatcher);
  receiver->setBtMessageFactory(factory);

  SharedHandle<DefaultBtRequestFactory> reqFactory
    (new DefaultBtRequestFactory());
  reqFactory->setPeer(getPeer());
  reqFactory->setPieceStorage(pieceStorage);
  reqFactory->setBtMessageDispatcher(dispatcher);
  reqFactory->setBtMessageFactory(factory);

  DefaultBtInteractiveHandle btInteractive
    (new DefaultBtInteractive(_requestGroup->getDownloadContext(), getPeer()));
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
  btInteractive->setRequestGroupMan(getDownloadEngine()->getRequestGroupMan());
  btInteractive->setBtMessageFactory(factory);
  if((metadataGetMode || !torrentAttrs->privateTorrent) &&
     !getPeer()->isLocalPeer()) {
    if(getOption()->getAsBool(PREF_ENABLE_PEER_EXCHANGE)) {
      btInteractive->setUTPexEnabled(true);
    }
    if(DHTSetup::initialized()) {
      btInteractive->setDHTEnabled(true);
      btInteractive->setLocalNode(DHTRegistry::getData().localNode);
      factory->setDHTEnabled(true);
    }
  }
  btInteractive->setUTMetadataRequestFactory(utMetadataRequestFactory);
  btInteractive->setUTMetadataRequestTracker(utMetadataRequestTracker);
  if(metadataGetMode) {
    btInteractive->enableMetadataGetMode();
  }

  _btInteractive = btInteractive;

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
    utMetadataRequestFactory->setPeer(getPeer());
    utMetadataRequestFactory->setUTMetadataRequestTracker
      (utMetadataRequestTracker);
  }

  getPeer()->allocateSessionResource
    (_requestGroup->getDownloadContext()->getPieceLength(),
     _requestGroup->getDownloadContext()->getTotalLength());
  getPeer()->setBtMessageDispatcher(dispatcher);

  _btRuntime->increaseConnections();
  _requestGroup->increaseNumCommand();
}

PeerInteractionCommand::~PeerInteractionCommand() {
  if(getPeer()->getCompletedLength() > 0) {
    _pieceStorage->subtractPieceStats(getPeer()->getBitfield(),
                                      getPeer()->getBitfieldLength());
  }
  getPeer()->releaseSessionResource();

  _requestGroup->decreaseNumCommand();
  _btRuntime->decreaseConnections();
}

bool PeerInteractionCommand::executeInternal() {
  setNoCheck(false);
  switch(_sequence) {
  case INITIATOR_SEND_HANDSHAKE:
    if(!getSocket()->isWritable(0)) {
      break;
    }
    disableWriteCheckSocket();
    setReadCheckSocket(getSocket());
    //socket->setBlockingMode();
    setTimeout(getOption()->getAsInt(PREF_BT_TIMEOUT));
    _btInteractive->initiateHandshake();
    _sequence = INITIATOR_WAIT_HANDSHAKE;
    break;
  case INITIATOR_WAIT_HANDSHAKE: {
    if(_btInteractive->countPendingMessage() > 0) {
      _btInteractive->sendPendingMessage();
      if(_btInteractive->countPendingMessage() > 0) {
        break;
      }
    }
    BtMessageHandle handshakeMessage = _btInteractive->receiveHandshake();
    if(handshakeMessage.isNull()) {
      break;
    }
    _btInteractive->doPostHandshakeProcessing();
    _sequence = WIRED;
    break;
  }
  case RECEIVER_WAIT_HANDSHAKE: {
    BtMessageHandle handshakeMessage =_btInteractive->receiveAndSendHandshake();
    if(handshakeMessage.isNull()) {
      break;
    }
    _btInteractive->doPostHandshakeProcessing();
    _sequence = WIRED;    
    break;
  }
  case WIRED:
    // See the comment for writable check below.
    disableWriteCheckSocket();

    _btInteractive->doInteractionProcessing();
    if(_btInteractive->countReceivedMessageInIteration() > 0) {
      updateKeepAlive();
    }
    if((getPeer()->amInterested() && !getPeer()->peerChoking()) ||
       _btInteractive->countOutstandingRequest() ||
       (getPeer()->peerInterested() && !getPeer()->amChoking())) {

      // Writable check to avoid slow seeding
      if(_btInteractive->isSendingMessageInProgress()) {
        setWriteCheckSocket(getSocket());
      }

      if(getDownloadEngine()->getRequestGroupMan()->
         doesOverallDownloadSpeedExceed() ||
         _requestGroup->doesDownloadSpeedExceed()) {
        disableReadCheckSocket();
        setNoCheck(true);
      } else {
        setReadCheckSocket(getSocket());
      }
    } else {
      disableReadCheckSocket();
    }
    break;
  }
  if(_btInteractive->countPendingMessage() > 0) {
    setNoCheck(true);
  }
  getDownloadEngine()->addCommand(this);
  return false;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(time_t wait) {
  if(_peerStorage->isPeerAvailable() && _btRuntime->lessThanEqMinPeers()) {
    SharedHandle<Peer> peer = _peerStorage->getUnusedPeer();
    peer->usedBy(getDownloadEngine()->newCUID());
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand
      (peer->usedBy(), _requestGroup, peer, getDownloadEngine(), _btRuntime);
    command->setPeerStorage(_peerStorage);
    command->setPieceStorage(_pieceStorage);
    getDownloadEngine()->addCommand(command);
  }
  return true;
}

void PeerInteractionCommand::onAbort() {
  _btInteractive->cancelAllPiece();
  _peerStorage->returnPeer(getPeer());
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
